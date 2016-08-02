
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <sys/mman.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <algorithm>

#include "dosboxxr/lib/hostcpudetect/caps.h"
#include "dosboxxr/lib/util/nr_wfpack.h"
#include "dosboxxr/lib/util/bitscan.h"
#include "dosboxxr/lib/util/rgbinfo.h"

#if HAVE_CPU_MMX
# include <mmintrin.h>
#endif

#if HAVE_CPU_SSE2
# include <xmmintrin.h>
#endif

#if HAVE_CPU_AVX2
# include <immintrin.h>
#endif

#if HAVE_CPU_ARM_NEON
# include <arm_neon.h>
#endif

struct rgb_bitmap_info {
    // describe bitmap by base, width, height, alignment, length (of buffer), and stride.
    // this does NOT describe where it's allocated from, how to free it, etc. BY DESIGN.
    unsigned char*              base;           // base of the memory block
    unsigned char*              canvas;         // where to draw from
    size_t                      width,height;   // dimensions of frame
    size_t                      length;         // length of the buffer (last byte = base+length-1)
    size_t                      stride;         // bytes per scanline
    size_t                      bytes_per_pixel;// bytes per pixel
    size_t                      stride_align;   // required alignment per scanline
    size_t                      base_align;     // required alignment for base
    size_t                      stride_padding; // additional padding in pixels per scanline
    size_t                      height_padding; // additional padding in pixels, vertically
    rgbinfo_t                   rgbinfo;
public:
    rgb_bitmap_info() : base(NULL), canvas(NULL), width(0), height(0), length(0), stride(0), bytes_per_pixel(0), stride_align(0), base_align(0),
        stride_padding(0), height_padding(0), rgbinfo() { }
    bool inline is_valid() const /* does not change class members */ {
        if (base == NULL) return false;
        if (canvas == NULL) return false;
        if (length == 0) return false;
        if (stride == 0) return false;
        if (width == 0 || height == 0) return false;
        if (bytes_per_pixel == 0 || bytes_per_pixel > 8) return false;
        if ((width*bytes_per_pixel) > stride) return false;
        if ((height*stride) > length) return false;
        return true;
    }
    template <class T> inline T* get_scanline(const size_t y,const size_t x=0) const { /* WARNING: does not range-check 'y', assumes canvas != NULL */
        return (T*)(canvas + (y * stride) + (x * bytes_per_pixel));
    }
    void update_stride_from_width() {
        stride = ((width + stride_padding) * bytes_per_pixel);
        if (stride_align > 0) {
            size_t a = stride % stride_align;
            if (a != 0) stride += stride_align - a;
        }
    }
    void update_length_from_stride_and_height() {
        length = stride * (height + height_padding);
    }
    void update_canvas_from_base() {
        canvas = base;
    }
};

int                             method = 0;

Display*                        x_display = NULL;
Visual*                         x_visual = NULL;
Screen*                         x_screen = NULL;
int                             x_depth = 0;
Atom                            x_wmDelete;
XEvent                          x_event;
Colormap                        x_cmap;
XSetWindowAttributes            x_swa;
XWindowAttributes               x_gwa;
GC                              x_gc = 0;
XImage*                         x_image = NULL;
XShmSegmentInfo                 x_shminfo;
size_t                          x_image_length = 0;
int                             x_using_shm = 0;
Window                          x_root_window;
Window                          x_window = (Window)0;
int                             x_screen_index = 0;
int                             x_quit = 0;
int                             bitmap_width = 0;
int                             bitmap_height = 0;
rgbinfo_t                       x_rgbinfo;
rgb_bitmap_info                 src_bitmap;

void close_bitmap();

int init_shm() {
	close_bitmap();

	if ((x_gc=XCreateGC(x_display, (Drawable)x_window, 0, NULL)) == NULL) {
		fprintf(stderr,"Cannot create drawable\n");
		close_bitmap();
		return 0;
	}

	x_using_shm = 1;
	memset(&x_shminfo,0,sizeof(x_shminfo));
	bitmap_width = (x_gwa.width+15)&(~15);
	bitmap_height = (x_gwa.height+15)&(~15);
	if ((x_image=XShmCreateImage(x_display, x_visual, x_depth, ZPixmap, NULL, &x_shminfo, bitmap_width, bitmap_height)) == NULL) {
		fprintf(stderr,"Cannot create SHM image\n");
		close_bitmap();
		return 0;
	}

	x_image_length = x_image->bytes_per_line * (x_image->height + 1);
	if ((x_shminfo.shmid=shmget(IPC_PRIVATE, x_image_length, IPC_CREAT | 0777)) < 0) {
		fprintf(stderr,"Cannot get SHM ID for image\n");
		x_shminfo.shmid = 0;
		close_bitmap();
		return 0;
	}

	if ((x_shminfo.shmaddr=x_image->data=(char*)shmat(x_shminfo.shmid, 0, 0)) == MAP_FAILED) {
		fprintf(stderr,"Cannot mmap for image\n");
		x_shminfo.shmaddr = NULL;
		x_image->data = NULL;
		close_bitmap();
		return 0;
	}
	x_shminfo.readOnly = 0;
	XShmAttach(x_display, &x_shminfo);
	XSync(x_display, 0);

	memset(x_image->data,0x00,x_image_length);

	bitmap_width = x_gwa.width;
	bitmap_height = x_gwa.height;
	fprintf(stderr,"SHM-based XImage created %ux%u\n",bitmap_width,bitmap_height);
	return 1;
}

int init_norm() {
	close_bitmap();

	if ((x_gc=XCreateGC(x_display, (Drawable)x_window, 0, NULL)) == NULL) {
		fprintf(stderr,"Cannot create drawable\n");
		close_bitmap();
		return 0;
	}

	x_using_shm = 0;
	bitmap_width = (x_gwa.width+15)&(~15);
	bitmap_height = (x_gwa.height+15)&(~15);
	if ((x_image=XCreateImage(x_display, x_visual, x_depth, ZPixmap, 0, NULL, bitmap_width, bitmap_height, 32, 0)) == NULL) {
		fprintf(stderr,"Cannot create image\n");
		close_bitmap();
		return 0;
	}

	x_image_length = x_image->bytes_per_line * (x_image->height + 1);
	if ((x_image->data=(char*)mmap(NULL,x_image_length,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0)) == MAP_FAILED) {
		fprintf(stderr,"Cannot mmap for image\n");
		x_image->data = NULL;
		close_bitmap();
		return 0;
	}

	memset(x_image->data,0x00,x_image_length);

	bitmap_width = x_gwa.width;
	bitmap_height = x_gwa.height;
	fprintf(stderr,"Normal (non-SHM) XImage created %ux%u\n",bitmap_width,bitmap_height);
	return 1;
}

void close_bitmap() {
	if (x_shminfo.shmid != 0) {
		XShmDetach(x_display, &x_shminfo);
		shmctl(x_shminfo.shmid, IPC_RMID, 0); /* the buffer will persist until X closes it */
	}
	if (x_image != NULL) {
		if (x_image) {
			if (x_image->data != NULL) {
				shmdt(x_image->data);
				x_image->data = NULL;
				x_image_length = 0;
			}
		}
		else {
			if (x_image->data != NULL) {
				munmap(x_image->data, x_image_length);
				x_image->data = NULL;
				x_image_length = 0;
			}
		}
		XDestroyImage(x_image);
		x_image = NULL;
	}
	if (x_gc != NULL) {
		XFreeGC(x_display, x_gc);
		x_gc = NULL;
	}
	x_shminfo.shmid = 0;
	x_using_shm = 0;
}

void update_to_X11() {
	if (x_image == NULL || bitmap_width == 0 || bitmap_height == 0)
		return;

	if (x_using_shm)
		XShmPutImage(x_display, x_window, x_gc, x_image, 0, 0, 0, 0, bitmap_width, bitmap_height, 0);
	else
		XPutImage(x_display, x_window, x_gc, x_image, 0, 0, 0, 0, bitmap_width, bitmap_height);
}

template <class T> void src_bitmap_render(rgb_bitmap_info &bitmap) {
    T r,g,b,*drow;
    size_t ox,oy;

    if (!bitmap.is_valid()) return;

    for (oy=0;oy < (bitmap.height/2);oy++) {
        drow = bitmap.get_scanline<T>(oy);
        for (ox=0;ox < (bitmap.width/2);ox++) {
            /* color */
            r = ((T)ox * (T)x_rgbinfo.r.bmask) / (T)(bitmap.width/2);
            g = ((T)oy * (T)x_rgbinfo.g.bmask) / (T)(bitmap.height/2);
            b = (T)x_rgbinfo.b.bmask -
                (((T)ox * (T)x_rgbinfo.b.bmask) / (T)(bitmap.width/2));

            *drow++ = (r << x_rgbinfo.r.shift) + (g << x_rgbinfo.g.shift) + (b << x_rgbinfo.b.shift) + x_rgbinfo.a.mask;
        }
    }

    for (oy=0;oy < (bitmap.height/2);oy++) {
        drow = bitmap.get_scanline<T>(oy,bitmap.width/2U);
        for (ox=(bitmap.width/2);ox < bitmap.width;ox++) {
            /* color */
            r = ((ox ^ oy) & 1) ? x_rgbinfo.r.bmask : 0;
            g = ((ox ^ oy) & 1) ? x_rgbinfo.g.bmask : 0;
            b = ((ox ^ oy) & 1) ? x_rgbinfo.b.bmask : 0;

            *drow++ = (r << x_rgbinfo.r.shift) + (g << x_rgbinfo.g.shift) + (b << x_rgbinfo.b.shift) + x_rgbinfo.a.mask;
        }
    }

    for (oy=(bitmap.height/2);oy < bitmap.height;oy++) {
        drow = bitmap.get_scanline<T>(oy);
        for (ox=0;ox < (bitmap.width/2);ox++) {
            /* color */
            r = ((ox ^ oy) & 2) ? x_rgbinfo.r.bmask : 0;
            g = ((ox ^ oy) & 2) ? x_rgbinfo.g.bmask : 0;
            b = ((ox ^ oy) & 2) ? x_rgbinfo.b.bmask : 0;

            *drow++ = (r << x_rgbinfo.r.shift) + (g << x_rgbinfo.g.shift) + (b << x_rgbinfo.b.shift) + x_rgbinfo.a.mask;
        }
    }

    for (oy=(bitmap.height/2);oy < bitmap.height;oy++) {
        drow = bitmap.get_scanline<T>(oy,bitmap.width/2U);
        for (ox=(bitmap.width/2);ox < bitmap.width;ox++) {
            /* color */
            r = ((ox ^ oy) & 4) ? x_rgbinfo.r.bmask : 0;
            g = ((ox ^ oy) & 4) ? x_rgbinfo.g.bmask : 0;
            b = ((ox ^ oy) & 4) ? x_rgbinfo.b.bmask : 0;

            *drow++ = (r << x_rgbinfo.r.shift) + (g << x_rgbinfo.g.shift) + (b << x_rgbinfo.b.shift) + x_rgbinfo.a.mask;
        }
    }
}

void src_bitmap_render(rgb_bitmap_info &bitmap) {
    if (x_image->bits_per_pixel == 32)
        src_bitmap_render<uint32_t>(bitmap);
    else if (x_image->bits_per_pixel == 16)
        src_bitmap_render<uint16_t>(bitmap);
    else {
        fprintf(stderr,"WARNING: src_bitmap_render() unsupported bit depth %u/bpp\n",
            x_image->bits_per_pixel);
    }
}

static inline void render_scale_from_sd_64towf(struct nr_wfpack &sy,const uint64_t t) {
	if (sizeof(nr_wftype) == 8) {
		// nr_wftype is 64.64
		sy.w = (nr_wftype)(t >> (uint64_t)32);
		sy.f = (nr_wftype)(t << (uint64_t)32);
	}
	else if (sizeof(nr_wftype) == 4) {
		// nr_wftype is 32.32
		sy.w = (nr_wftype)(t >> (uint64_t)32);
		sy.f = (nr_wftype)t;
	}
	else {
		abort();
	}
}

// This version uses sh / dh for even scaling with nearest neighbor interpolation.
// for bilinear interpolation, use render_scale_from_sd(). using this function for
// bilinear interpolation will expose blank/junk pixels on the bottom/right edge of the frame.
void render_scale_from_sd_nearest(struct nr_wfpack &sy,const uint32_t dh,const uint32_t sh) {
	uint64_t t;

    t  = (uint64_t)((uint64_t)sh << (uint64_t)32);
    t /= (uint64_t)dh;
    render_scale_from_sd_64towf(sy,t);
}

// This version uses (sh - 1) / (dh - 1) so that bilinear interpolation does not expose junk pixels on the
// bottom/right edge of the frame. If you are using nearest neighbor scaling, don't use this version, use
// the render_scale_from_sd_nearest() version of the function.
void render_scale_from_sd(struct nr_wfpack &sy,const uint32_t dh,const uint32_t sh) {
	uint64_t t;

    t  = (uint64_t)((uint64_t)(sh - uint32_t(1)) << (uint64_t)32);
    t /= (uint64_t)(dh - uint32_t(1));
    render_scale_from_sd_64towf(sy,t);
}

/* render image to XImage.
 * stretch fit using crude nearest neighbor scaling */

template <class T> void rerender_out_neighbor() {
    nr_wfpack sx,sxi={0,0},sy={0,0},stepx,stepy;
    unsigned char *drow;
    size_t ox,oy;

    render_scale_from_sd_nearest(/*&*/stepx,bitmap_width,src_bitmap.width);
    render_scale_from_sd_nearest(/*&*/stepy,bitmap_height,src_bitmap.height);

    drow = (unsigned char*)x_image->data;
    oy = bitmap_height;
    do {
        T *s = src_bitmap.get_scanline<T>(sy.w);
        T *d = (T*)drow;

        ox = bitmap_width;
        sx = sxi;
        do {
            *d++ = s[sx.w];
            if ((--ox) == 0) break;
            sx += stepx;
        } while (1);

        if ((--oy) == 0) break;
        drow += x_image->bytes_per_line;
        sy += stepy;
    } while (1);
}



#define VINTERP_MAX 4096

// WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
template <class T> inline T rerender_line_bilinear_pixel_blend(const T cur,const T nxt,const T mulmax,const T mul,const T rbmask,const T abmask,const T pshift) {
    T sum;

    sum  = (((uint64_t)(cur & rbmask) * (mulmax - mul)) >> pshift) & rbmask;
    sum += (((uint64_t)(cur & abmask) * (mulmax - mul)) >> pshift) & abmask;
    sum += (((uint64_t)(nxt & rbmask) *           mul ) >> pshift) & rbmask;
    sum += (((uint64_t)(nxt & abmask) *           mul ) >> pshift) & abmask;
    return sum;
}

#if HAVE_CPU_MMX
// 32bpp optimized for 8-bit ARGB/RGBA. rmask should be 0x00FF,0x00FF,... etc
static inline __m64 rerender_line_bilinear_pixel_blend_mmx_argb8(const __m64 cur,const __m64 nxt,const __m64 mul,const __m64 rmask) {
    __m64 d1,d2,d3,d4;

    d1 = _mm_and_si64(_mm_mulhi_pi16(_mm_sub_pi16(_mm_and_si64(nxt,rmask),_mm_and_si64(cur,rmask)),mul),rmask);
    d2 = _mm_slli_si64(_mm_and_si64(_mm_mulhi_pi16(_mm_sub_pi16(_mm_and_si64(_mm_srli_si64(nxt,8),rmask),_mm_and_si64(_mm_srli_si64(cur,8),rmask)),mul),rmask),8);
    d3 = _mm_add_pi8(d1,d2);
    d4 = _mm_add_pi8(d3,d3);
    return _mm_add_pi8(d4,cur);
}

// 16bpp general R/G/B, usually 5/6/5 or 5/5/5
static inline __m64 rerender_line_bilinear_pixel_blend_mmx_rgb16(const __m64 cur,const __m64 nxt,const __m64 mul,const __m64 rmask,const uint16_t rshift,const __m64 gmask,const uint16_t gshift,const __m64 bmask,const uint16_t bshift) {
    __m64 rc,gc,bc;
    __m64 rn,gn,bn;
    __m64 d,sum;

    rc = _mm_and_si64(_mm_srli_si64(cur,rshift),rmask);
    gc = _mm_and_si64(_mm_srli_si64(cur,gshift),gmask);
    bc = _mm_and_si64(_mm_srli_si64(cur,bshift),bmask);

    rn = _mm_and_si64(_mm_srli_si64(nxt,rshift),rmask);
    gn = _mm_and_si64(_mm_srli_si64(nxt,gshift),gmask);
    bn = _mm_and_si64(_mm_srli_si64(nxt,bshift),bmask);

    d = _mm_sub_pi16(rn,rc);
    sum = _mm_slli_si64(_mm_add_pi16(rc,_mm_and_si64(_mm_mulhi_pi16(_mm_add_pi16(d,d),mul),rmask)),rshift);

    d = _mm_sub_pi16(gn,gc);
    sum = _mm_add_pi16(_mm_slli_si64(_mm_add_pi16(gc,_mm_and_si64(_mm_mulhi_pi16(_mm_add_pi16(d,d),mul),gmask)),gshift),sum);

    d = _mm_sub_pi16(bn,bc);
    sum = _mm_add_pi16(_mm_slli_si64(_mm_add_pi16(bc,_mm_and_si64(_mm_mulhi_pi16(_mm_add_pi16(d,d),mul),bmask)),bshift),sum);

    return sum;
}
#endif

#if HAVE_CPU_SSE2
// 32bpp optimized for 8-bit ARGB/RGBA. rmask should be 0x00FF,0x00FF,... etc
static inline __m128i rerender_line_bilinear_pixel_blend_sse_argb8(const __m128i cur,const __m128i nxt,const __m128i mul,const __m128i rmask) {
    __m128i d1,d2,d3,d4;

    d1 = _mm_and_si128(_mm_mulhi_epi16(_mm_sub_epi16(_mm_and_si128(nxt,rmask),_mm_and_si128(cur,rmask)),mul),rmask);
    d2 = _mm_slli_si128(_mm_and_si128(_mm_mulhi_epi16(_mm_sub_epi16(_mm_and_si128(_mm_srli_si128(nxt,1/*bytes!*/),rmask),_mm_and_si128(_mm_srli_si128(cur,1/*bytes!*/),rmask)),mul),rmask),1/*bytes!*/);
    d3 = _mm_add_epi8(d1,d2);
    d4 = _mm_add_epi8(d3,d3);
    return _mm_add_epi8(d4,cur);
}

// 16bpp general R/G/B, usually 5/6/5 or 5/5/5
static inline __m128i rerender_line_bilinear_pixel_blend_sse_rgb16(const __m128i cur,const __m128i nxt,const __m128i mul,const __m128i rmask,const uint16_t rshift,const __m128i gmask,const uint16_t gshift,const __m128i bmask,const uint16_t bshift) {
    __m128i rc,gc,bc;
    __m128i rn,gn,bn;
    __m128i d,sum;

    rc = _mm_and_si128(_mm_srli_epi16(cur,rshift),rmask);
    gc = _mm_and_si128(_mm_srli_epi16(cur,gshift),gmask);
    bc = _mm_and_si128(_mm_srli_epi16(cur,bshift),bmask);

    rn = _mm_and_si128(_mm_srli_epi16(nxt,rshift),rmask);
    gn = _mm_and_si128(_mm_srli_epi16(nxt,gshift),gmask);
    bn = _mm_and_si128(_mm_srli_epi16(nxt,bshift),bmask);

    d = _mm_sub_epi16(rn,rc);
    sum = _mm_slli_epi16(_mm_add_epi16(rc,_mm_and_si128(_mm_mulhi_epi16(_mm_add_epi16(d,d),mul),rmask)),rshift);

    d = _mm_sub_epi16(gn,gc);
    sum = _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(gc,_mm_and_si128(_mm_mulhi_epi16(_mm_add_epi16(d,d),mul),gmask)),gshift),sum);

    d = _mm_sub_epi16(bn,bc);
    sum = _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(bc,_mm_and_si128(_mm_mulhi_epi16(_mm_add_epi16(d,d),mul),bmask)),bshift),sum);

    return sum;
}
#endif

#if HAVE_CPU_AVX2
// 32bpp optimized for 8-bit ARGB/RGBA. rmask should be 0x00FF,0x00FF,... etc
static inline __m256i rerender_line_bilinear_pixel_blend_avx_argb8(const __m256i cur,const __m256i nxt,const __m256i mul,const __m256i rmask) {
    __m256i d1,d2,d3,d4;

    d1 = _mm256_and_si256(_mm256_mulhi_epi16(_mm256_sub_epi16(_mm256_and_si256(nxt,rmask),_mm256_and_si256(cur,rmask)),mul),rmask);
    d2 = _mm256_slli_si256(_mm256_and_si256(_mm256_mulhi_epi16(_mm256_sub_epi16(_mm256_and_si256(_mm256_srli_si256(nxt,1/*bytes!*/),rmask),_mm256_and_si256(_mm256_srli_si256(cur,1/*bytes!*/),rmask)),mul),rmask),1/*bytes!*/);
    d3 = _mm256_add_epi8(d1,d2);
    d4 = _mm256_add_epi8(d3,d3);
    return _mm256_add_epi8(d4,cur);
}

// 16bpp general R/G/B, usually 5/6/5 or 5/5/5
static inline __m256i rerender_line_bilinear_pixel_blend_avx_rgb16(const __m256i cur,const __m256i nxt,const __m256i mul,const __m256i rmask,const uint16_t rshift,const __m256i gmask,const uint16_t gshift,const __m256i bmask,const uint16_t bshift) {
    __m256i rc,gc,bc;
    __m256i rn,gn,bn;
    __m256i d,sum;

    rc = _mm256_and_si256(_mm256_srli_epi16(cur,rshift),rmask);
    gc = _mm256_and_si256(_mm256_srli_epi16(cur,gshift),gmask);
    bc = _mm256_and_si256(_mm256_srli_epi16(cur,bshift),bmask);

    rn = _mm256_and_si256(_mm256_srli_epi16(nxt,rshift),rmask);
    gn = _mm256_and_si256(_mm256_srli_epi16(nxt,gshift),gmask);
    bn = _mm256_and_si256(_mm256_srli_epi16(nxt,bshift),bmask);

    d = _mm256_sub_epi16(rn,rc);
    sum = _mm256_slli_epi16(_mm256_add_epi16(rc,_mm256_and_si256(_mm256_mulhi_epi16(_mm256_add_epi16(d,d),mul),rmask)),rshift);

    d = _mm256_sub_epi16(gn,gc);
    sum = _mm256_add_epi16(_mm256_slli_epi16(_mm256_add_epi16(gc,_mm256_and_si256(_mm256_mulhi_epi16(_mm256_add_epi16(d,d),mul),gmask)),gshift),sum);

    d = _mm256_sub_epi16(bn,bc);
    sum = _mm256_add_epi16(_mm256_slli_epi16(_mm256_add_epi16(bc,_mm256_and_si256(_mm256_mulhi_epi16(_mm256_add_epi16(d,d),mul),bmask)),bshift),sum);

    return sum;
}
#endif

#if HAVE_CPU_ARM_NEON
// 32bpp optimized for 8-bit ARGB/RGBA. rmask should be 0x00FF,0x00FF,... etc
static inline int16x8_t rerender_line_bilinear_pixel_blend_arm_neon_argb8(const int16x8_t cur,const int16x8_t nxt,const int16_t mul,const int16x8_t rmask) {
#if 0 // FIXME: I can't develop this code until I can figure out how to get X on my Raspberry Pi 2 to come up in 32-bit ARGB mode
    int16x8_t d1,d2,d3,d4;

    d1 = _mm256_and_si256(_mm256_mulhi_epi16(_mm256_sub_epi16(_mm256_and_si256(nxt,rmask),_mm256_and_si256(cur,rmask)),mul),rmask);
    d2 = _mm256_slli_si256(_mm256_and_si256(_mm256_mulhi_epi16(_mm256_sub_epi16(_mm256_and_si256(_mm256_srli_si256(nxt,1/*bytes!*/),rmask),_mm256_and_si256(_mm256_srli_si256(cur,1/*bytes!*/),rmask)),mul),rmask),1/*bytes!*/);
    d3 = _mm256_add_epi8(d1,d2);
    d4 = _mm256_add_epi8(d3,d3);
    return _mm256_add_epi8(d4,cur);
#endif
}

template <const uint8_t shf> static inline int16x8_t rerender_line_bilinear_pixel_blend_arm_neon_rgb16channel(const int16x8_t cur,const int16x8_t nxt,const int16_t mul,const int16x8_t cmask) {
    // WARNING: The reason for shf != 0 conditional shifting by template param 'shf' is that
    //          the ARMv7 assembler behind GCC 4.8 on the Raspberry Pi won't allow you to specify a
    //          constant shift of 0. It will complain "invalid constant", though in a way that
    //          is confusing that causes GCC to point to some unrelated closing bracket farther
    //          down this source code. The ternary expression is meant to bypass the shift
    //          entirely if shf == 0 to avoid that error. That is also why the template parameter
    //          is declared const.
    const int16x8_t cir = (shf != 0) ? vshrq_n_s16(cur,shf) : cur;
    const int16x8_t nir = (shf != 0) ? vshrq_n_s16(nxt,shf) : nxt;
    const int16x8_t rc = vandq_s16(cir,cmask);
    const int16x8_t rn = vandq_s16(nir,cmask);
    const int16x8_t d = vsubq_s16(rn,rc);
    const int16x8_t f = vaddq_s16(rc,vandq_s16(vqdmulhq_n_s16(d,mul),cmask));
    return (shf != 0) ? vshlq_n_s16(f,shf) : f;
}

// 16bpp 5/6/5 (FIXME: Add another variant for 5/5/5 when the need arises)
static inline int16x8_t rerender_line_bilinear_pixel_blend_arm_neon_rgb16(const int16x8_t cur,const int16x8_t nxt,const int16_t mul,const int16x8_t rmask,const uint16_t rshift,const int16x8_t gmask,const uint16_t gshift,const int16x8_t bmask,const uint16_t bshift) {
    int16x8_t sr,sg,sb;

    // WARNING: ARMv7 NEON shift intrinsics demand that the shift bit count is a constant. It cannot
    //          be a variable. So in the interest of getting this to work, we assume (for now)
    //          that it is the RGB 5/6/5 format that my Raspberry Pi 2 comes up in when starting X.
    //
    //          Note that it doesn't matter whether the 5/6/5 bit fields are R/G/B or B/G/R, what
    //          matters is that the bit fields are 5-bit/6-bit/5-bit.

    sr = rerender_line_bilinear_pixel_blend_arm_neon_rgb16channel<uint8_t(11)>(cur,nxt,mul,rmask);
    sg = rerender_line_bilinear_pixel_blend_arm_neon_rgb16channel<uint8_t(5)>(cur,nxt,mul,gmask);
    sb = rerender_line_bilinear_pixel_blend_arm_neon_rgb16channel<uint8_t(0)>(cur,nxt,mul,bmask);
    return vaddq_s16(vaddq_s16(sr,sg),sb);
}
#endif

template <class T> inline void rerender_line_bilinear_hinterp_stage(T *d,T *s,struct nr_wfpack sx,const struct nr_wfpack &stepx,size_t dwidth,const T rbmask,const T abmask,const T fmax,const T fshift,const T pshift) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend<T>(s[sx.w],s[sx.w+1],fmax,(T)(sx.f >> (T)fshift),rbmask,abmask,pshift);
        sx += stepx;
    } while ((--dwidth) != (size_t)0);
}

#if HAVE_CPU_MMX
// case 2: 32-bit ARGB 8-bits per pixel
static inline void rerender_line_bilinear_vinterp_stage_mmx_argb8(__m64 *d,__m64 *s,__m64 *s2,const __m64 mul,size_t width,const __m64 rmask) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend_mmx_argb8(*s++,*s2++,mul,rmask);
    } while ((--width) != (size_t)0);
    _mm_empty();
}

// case 1: 16-bit arbitrary masks
static inline void rerender_line_bilinear_vinterp_stage_mmx_rgb16(__m64 *d,__m64 *s,__m64 *s2,const __m64 mul,size_t width,const __m64 rmask,const uint16_t rshift,const __m64 gmask,const uint16_t gshift,const __m64 bmask,const uint16_t bshift) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend_mmx_rgb16(*s++,*s2++,mul,rmask,rshift,gmask,gshift,bmask,bshift);
    } while ((--width) != (size_t)0);
}
#endif

#if HAVE_CPU_SSE2
// case 2: 32-bit ARGB 8-bits per pixel
static inline void rerender_line_bilinear_vinterp_stage_sse_argb8(__m128i *d,__m128i *s,__m128i *s2,const __m128i mul,size_t width,const __m128i rmask) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend_sse_argb8(*s++,*s2++,mul,rmask);
    } while ((--width) != (size_t)0);
}

// case 1: 16-bit arbitrary masks
static inline void rerender_line_bilinear_vinterp_stage_sse_rgb16(__m128i *d,__m128i *s,__m128i *s2,const __m128i mul,size_t width,const __m128i rmask,const uint16_t rshift,const __m128i gmask,const uint16_t gshift,const __m128i bmask,const uint16_t bshift) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend_sse_rgb16(*s++,*s2++,mul,rmask,rshift,gmask,gshift,bmask,bshift);
    } while ((--width) != (size_t)0);
}
#endif

#if HAVE_CPU_AVX2
// case 2: 32-bit ARGB 8-bits per pixel
static inline void rerender_line_bilinear_vinterp_stage_avx_argb8(__m256i *d,__m256i *s,__m256i *s2,const __m256i mul,size_t width,const __m256i rmask) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend_avx_argb8(*s++,*s2++,mul,rmask);
    } while ((--width) != (size_t)0);
}

// case 1: 16-bit arbitrary masks
static inline void rerender_line_bilinear_vinterp_stage_avx_rgb16(__m256i *d,__m256i *s,__m256i *s2,const __m256i mul,size_t width,const __m256i rmask,const uint16_t rshift,const __m256i gmask,const uint16_t gshift,const __m256i bmask,const uint16_t bshift) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend_avx_rgb16(*s++,*s2++,mul,rmask,rshift,gmask,gshift,bmask,bshift);
    } while ((--width) != (size_t)0);
}
#endif

#if HAVE_CPU_ARM_NEON
// case 2: 32-bit ARGB 8-bits per pixel
static inline void rerender_line_bilinear_vinterp_stage_arm_neon_argb8(int16x8_t *d,int16x8_t *s,int16x8_t *s2,const int16_t mul,size_t width,const int16x8_t rmask) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend_arm_neon_argb8(*s++,*s2++,mul,rmask);
    } while ((--width) != (size_t)0);
}

// case 1: 16-bit arbitrary masks
static inline void rerender_line_bilinear_vinterp_stage_arm_neon_rgb16(int16x8_t *d,int16x8_t *s,int16x8_t *s2,const int16_t mul,size_t width,const int16x8_t rmask,const uint16_t rshift,const int16x8_t gmask,const uint16_t gshift,const int16x8_t bmask,const uint16_t bshift) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend_arm_neon_rgb16(*s++,*s2++,mul,rmask,rshift,gmask,gshift,bmask,bshift);
    } while ((--width) != (size_t)0);
}
#endif

template <class T> inline void rerender_line_bilinear_vinterp_stage(T *d,T *s,T *s2,const T mul,size_t width,const T rbmask,const T abmask,const T fmax,const T pshift) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend<T>(*s++,*s2++,fmax,mul,rbmask,abmask,pshift);
    } while ((--width) != (size_t)0);
}

template <class T> class vinterp_tmp {
public:
    T               tmp[VINTERP_MAX];
};

template <class T> void rerender_out_bilinear() {
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<T> vinterp_tmp;
    const T alpha = 
        (T)(~(x_image->red_mask+x_image->green_mask+x_image->blue_mask));
    const T rbmask = (T)(x_image->red_mask+x_image->blue_mask);
    const T abmask = (T)x_image->green_mask + alpha;
    unsigned char *drow;
    uint32_t rm,gm,bm;
    uint8_t rs,gs,bs;
    size_t ox,oy;
    T fshift;
    T pshift;
    T fmax;
    T mul;

    rs = bitscan_forward(x_image->red_mask,0);
    rm = bitscan_count(x_image->red_mask,rs) - rs;

    gs = bitscan_forward(x_image->green_mask,0);
    gm = bitscan_count(x_image->green_mask,gs) - gs;

    bs = bitscan_forward(x_image->blue_mask,0);
    bm = bitscan_count(x_image->blue_mask,bs) - bs;

    fshift = std::min(rm,std::min(gm,bm));
    pshift = fshift;
    fshift = (sizeof(nr_wftype) * 8) - fshift;

    fmax = 1U << pshift;

    render_scale_from_sd(/*&*/stepx,bitmap_width,src_bitmap.width);
    render_scale_from_sd(/*&*/stepy,bitmap_height,src_bitmap.height);

    if (bitmap_width == 0 || src_bitmap.width == 0) return;

    drow = (unsigned char*)x_image->data;
    oy = bitmap_height;
    do {
        T *s2 = src_bitmap.get_scanline<T>(sy.w+1);
        T *s = src_bitmap.get_scanline<T>(sy.w);
        T *d = (T*)drow;

        mul = (T)(sy.f >> fshift);

        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                rerender_line_bilinear_vinterp_stage<T>(vinterp_tmp.tmp,s,s2,mul,src_bitmap.width,rbmask,abmask,fmax,pshift);
                rerender_line_bilinear_hinterp_stage<T>(d,vinterp_tmp.tmp,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                rerender_line_bilinear_vinterp_stage<T>(d,s,s2,mul,src_bitmap.width,rbmask,abmask,fmax,pshift);
            }
        }
        else {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, no vertical interpolation
                rerender_line_bilinear_hinterp_stage<T>(d,s,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // copy the scanline 1:1 no interpolation
                memcpy(d,s,bitmap_width*sizeof(T));
            }
        }

        if ((--oy) == 0) break;
        drow += x_image->bytes_per_line;
        sy += stepy;
    } while (1);
}

// MMX alignment warning:
// It is strongly recommended that the bitmap base and scan line stride are MMX aligned, meaning
// that the base of the bitmap is aligned to 8 bytes and the scan line stride is a multiple of 8.
// Failure to do so may cause a performance loss.
template <class T> void rerender_out_bilinear_mmx() {
#if HAVE_CPU_MMX
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<__m64> vinterp_tmp;
    const T alpha = 
        (T)(~(x_image->red_mask+x_image->green_mask+x_image->blue_mask));
    const T rbmask = (T)(x_image->red_mask+x_image->blue_mask);
    const T abmask = (T)x_image->green_mask + alpha;
    __m64 rmask64,gmask64,bmask64,mul64;
    const size_t pixels_per_mmx =
        sizeof(__m64) / sizeof(T);
    unsigned char *drow;
    uint32_t rm,gm,bm;
    uint8_t rs,gs,bs;
    size_t ox,oy;
    T fshift;
    T pshift;
    T fmax;
    T mul;

    // do not run this function if MMX extensions are not present
    if (!hostCPUcaps.mmx) {
        fprintf(stderr,"CPU does not support MMX\n");
        return;
    }

    rs = bitscan_forward(x_image->red_mask,0);
    rm = bitscan_count(x_image->red_mask,rs) - rs;

    gs = bitscan_forward(x_image->green_mask,0);
    gm = bitscan_count(x_image->green_mask,gs) - gs;

    bs = bitscan_forward(x_image->blue_mask,0);
    bm = bitscan_count(x_image->blue_mask,bs) - bs;

    fshift = std::min(rm,std::min(gm,bm));
    pshift = fshift;
    fshift = (sizeof(nr_wftype) * 8) - fshift;

    if (sizeof(T) == 4) {
        // 32bpp this code can only handle the 8-bit RGBA/ARGB case, else R/G/B fields cross 16-bit boundaries
        if (pshift != 8) return;
        if (bm != 8 || gm != 8 || rm != 8) return; // each field, 8 bits
        if ((rs&7) != 0 || (gs&7) != 0 || (bs&7) != 0) return; // each field, starts on 8-bit boundaries

        rmask64 = _mm_set_pi16(0x00FF,0x00FF,0x00FF,0x00FF);
    }
    else {
        // 16bpp this code can handle any case
        if (pshift > 15) return;

        rmask64 = _mm_set_pi16((1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1);
        gmask64 = _mm_set_pi16((1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1);
        bmask64 = _mm_set_pi16((1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1);
    }

    fmax = 1U << pshift;

    render_scale_from_sd(/*&*/stepx,bitmap_width,src_bitmap.width);
    render_scale_from_sd(/*&*/stepy,bitmap_height,src_bitmap.height);

    unsigned int src_bitmap_width_m64 = (src_bitmap.width + pixels_per_mmx - 1) / pixels_per_mmx;

    if (bitmap_width == 0 || src_bitmap_width_m64 == 0) return;

    drow = (unsigned char*)x_image->data;
    oy = bitmap_height;
    do {
        T *s2 = src_bitmap.get_scanline<T>(sy.w+1);
        T *s = src_bitmap.get_scanline<T>(sy.w);
        T *d = (T*)drow;

        mul = (T)(sy.f >> fshift);

        {
            unsigned int m = (mul & (~1U)) << (15 - pshift); // 16-bit MMX multiply (signed bit), remove one bit to match precision
            mul64 = _mm_set_pi16(m,m,m,m);
        }

        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_mmx_argb8(vinterp_tmp.tmp,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_m64,rmask64);
                else
                    rerender_line_bilinear_vinterp_stage_mmx_rgb16(vinterp_tmp.tmp,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_m64,
                        rmask64,rs,gmask64,gs,bmask64,bs);

                rerender_line_bilinear_hinterp_stage<T>(d,(T*)vinterp_tmp.tmp,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_mmx_argb8((__m64*)d,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_m64,rmask64);
                else
                    rerender_line_bilinear_vinterp_stage_mmx_rgb16((__m64*)d,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_m64,
                        rmask64,rs,gmask64,gs,bmask64,bs);
            }
        }
        else {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, no vertical interpolation
                rerender_line_bilinear_hinterp_stage<T>(d,s,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // copy the scanline 1:1 no interpolation
                memcpy(d,s,bitmap_width*sizeof(T));
            }
        }

        if ((--oy) == 0) break;
        drow += x_image->bytes_per_line;
        sy += stepy;
    } while (1);
#else
    fprintf(stderr,"Compiler does not support MMX\n");
#endif
}

// SSE alignment warning:
// Do not use this routine unless the scan lines in the bitmap are all aligned to SSE boundaries,
// meaning that the bitmap base is aligned to a 16 byte boundary and the scan line "stride" is a
// multiple of 16 bytes. Modern processors (like Intel Core 2 and such) tolerate misaligned SSE
// but older processors (like the Pentium 4) will throw an exception if SSE instructions are
// executed on misaligned memory. Besides that, there may be performance loss if SSE operations
// are misaligned.
template <class T> void rerender_out_bilinear_sse() {
#if HAVE_CPU_SSE2
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<__m128i> vinterp_tmp;
    const T alpha = 
        (T)(~(x_image->red_mask+x_image->green_mask+x_image->blue_mask));
    const T rbmask = (T)(x_image->red_mask+x_image->blue_mask);
    const T abmask = (T)x_image->green_mask + alpha;
    __m128i rmask128,gmask128,bmask128,mul128;
    const size_t pixels_per_sse =
        sizeof(__m128i) / sizeof(T);
    unsigned char *drow;
    uint32_t rm,gm,bm;
    uint8_t rs,gs,bs;
    size_t ox,oy;
    T fshift;
    T pshift;
    T fmax;
    T mul;

    // do not run this function if SSE2 extensions are not present
    if (!hostCPUcaps.sse2) {
        fprintf(stderr,"CPU does not support SSE2\n");
        return;
    }

    rs = bitscan_forward(x_image->red_mask,0);
    rm = bitscan_count(x_image->red_mask,rs) - rs;

    gs = bitscan_forward(x_image->green_mask,0);
    gm = bitscan_count(x_image->green_mask,gs) - gs;

    bs = bitscan_forward(x_image->blue_mask,0);
    bm = bitscan_count(x_image->blue_mask,bs) - bs;

    fshift = std::min(rm,std::min(gm,bm));
    pshift = fshift;
    fshift = (sizeof(nr_wftype) * 8) - fshift;

    if (sizeof(T) == 4) {
        // 32bpp this code can only handle the 8-bit RGBA/ARGB case, else R/G/B fields cross 16-bit boundaries
        if (pshift != 8) return;
        if (bm != 8 || gm != 8 || rm != 8) return; // each field, 8 bits
        if ((rs&7) != 0 || (gs&7) != 0 || (bs&7) != 0) return; // each field, starts on 8-bit boundaries

        rmask128 = _mm_set_epi16(0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF);
    }
    else {
        // 16bpp this code can handle any case
        if (pshift > 15) return;

        rmask128 = _mm_set_epi16((1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1);
        gmask128 = _mm_set_epi16((1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1);
        bmask128 = _mm_set_epi16((1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1);
    }

    fmax = 1U << pshift;

    render_scale_from_sd(/*&*/stepx,bitmap_width,src_bitmap.width);
    render_scale_from_sd(/*&*/stepy,bitmap_height,src_bitmap.height);

    unsigned int src_bitmap_width_m128 = (src_bitmap.width + pixels_per_sse - 1) / pixels_per_sse;

    if (bitmap_width == 0 || src_bitmap_width_m128 == 0) return;

    drow = (unsigned char*)x_image->data;
    oy = bitmap_height;
    do {
        T *s2 = src_bitmap.get_scanline<T>(sy.w+1);
        T *s = src_bitmap.get_scanline<T>(sy.w);
        T *d = (T*)drow;

        mul = (T)(sy.f >> fshift);

        {
            unsigned int m = (mul & (~1U)) << (15 - pshift); // 16-bit MMX multiply (signed bit), remove one bit to match precision
            mul128 = _mm_set_epi16(m,m,m,m,m,m,m,m);
        }

        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_sse_argb8(vinterp_tmp.tmp,(__m128i*)s,(__m128i*)s2,mul128,src_bitmap_width_m128,rmask128);
                else
                    rerender_line_bilinear_vinterp_stage_sse_rgb16(vinterp_tmp.tmp,(__m128i*)s,(__m128i*)s2,mul128,src_bitmap_width_m128,
                        rmask128,rs,gmask128,gs,bmask128,bs);

                rerender_line_bilinear_hinterp_stage<T>(d,(T*)vinterp_tmp.tmp,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_sse_argb8((__m128i*)d,(__m128i*)s,(__m128i*)s2,mul128,src_bitmap_width_m128,rmask128);
                else
                    rerender_line_bilinear_vinterp_stage_sse_rgb16((__m128i*)d,(__m128i*)s,(__m128i*)s2,mul128,src_bitmap_width_m128,
                        rmask128,rs,gmask128,gs,bmask128,bs);
            }
        }
        else {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, no vertical interpolation
                rerender_line_bilinear_hinterp_stage<T>(d,s,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // copy the scanline 1:1 no interpolation
                memcpy(d,s,bitmap_width*sizeof(T));
            }
        }

        if ((--oy) == 0) break;
        drow += x_image->bytes_per_line;
        sy += stepy;
    } while (1);
#else
    fprintf(stderr,"Compiler does not support SSE2\n");
#endif
}

// AVX alignment warning:
// Do not use this routine unless the scan lines in the bitmap are all aligned to AVX boundaries,
// meaning that the bitmap base is aligned to a 32 byte boundary and the scan line "stride" is a
// multiple of 32 bytes. Besides the performance loss of unaligned access, program testing on
// current Intel hardware says that unaligned access triggers a fault from the processor.
//
// NOTE: Experience on my development laptop says this isn't much faster than SSE. However
//       I'm going to assume that's just my laptop, and that maybe in the future, AVX will
//       get faster.
template <class T> void rerender_out_bilinear_avx() {
#if HAVE_CPU_AVX2
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<__m256i> vinterp_tmp;
    const T alpha = 
        (T)(~(x_image->red_mask+x_image->green_mask+x_image->blue_mask));
    const T rbmask = (T)(x_image->red_mask+x_image->blue_mask);
    const T abmask = (T)x_image->green_mask + alpha;
    __m256i rmask256,gmask256,bmask256,mul256;
    const size_t pixels_per_avx =
        sizeof(__m256i) / sizeof(T);
    unsigned char *drow;
    uint32_t rm,gm,bm;
    uint8_t rs,gs,bs;
    size_t ox,oy;
    T fshift;
    T pshift;
    T fmax;
    T mul;

    // do not run this function if AVX2 extensions are not present
    if (!hostCPUcaps.avx2) {
        fprintf(stderr,"CPU does not support AVX2\n");
        return;
    }

    rs = bitscan_forward(x_image->red_mask,0);
    rm = bitscan_count(x_image->red_mask,rs) - rs;

    gs = bitscan_forward(x_image->green_mask,0);
    gm = bitscan_count(x_image->green_mask,gs) - gs;

    bs = bitscan_forward(x_image->blue_mask,0);
    bm = bitscan_count(x_image->blue_mask,bs) - bs;

    fshift = std::min(rm,std::min(gm,bm));
    pshift = fshift;
    fshift = (sizeof(nr_wftype) * 8) - fshift;

    // this code WILL fault if base or stride are not multiple of 32!
    if ((src_bitmap_stride & 31) != 0 || ((size_t)src_bitmap & 31) != 0) {
        fprintf(stderr,"Source bitmap not AVX usable (base=%p stride=0x%x\n",(void*)src_bitmap,(unsigned int)src_bitmap_stride);
        return;
    }
    if (((size_t)x_image->data & 31) != 0 || (x_image->bytes_per_line & 31) != 0) {
        fprintf(stderr,"Target bitmap not AVX usable (base=%p stride=0x%x\n",(void*)x_image->data,(unsigned int)x_image->bytes_per_line);
        return;
    }

    if (sizeof(T) == 4) {
        // 32bpp this code can only handle the 8-bit RGBA/ARGB case, else R/G/B fields cross 16-bit boundaries
        if (pshift != 8) return;
        if (bm != 8 || gm != 8 || rm != 8) return; // each field, 8 bits
        if ((rs&7) != 0 || (gs&7) != 0 || (bs&7) != 0) return; // each field, starts on 8-bit boundaries

        rmask256 = _mm256_set_epi16(
            0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,
            0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF);
    }
    else {
        // 16bpp this code can handle any case
        if (pshift > 15) return;

        rmask256 = _mm256_set_epi16(
            (1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,
            (1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1);
        gmask256 = _mm256_set_epi16(
            (1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,
            (1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1);
        bmask256 = _mm256_set_epi16(
            (1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,
            (1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1);
    }

    fmax = 1U << pshift;

    render_scale_from_sd(/*&*/stepx,bitmap_width,src_bitmap_width);
    render_scale_from_sd(/*&*/stepy,bitmap_height,src_bitmap_height);

    unsigned int src_bitmap_width_m256 = (src_bitmap_width + pixels_per_avx - 1) / pixels_per_avx;

    if (bitmap_width == 0 || src_bitmap_width_m256 == 0) return;

    drow = (unsigned char*)x_image->data;
    oy = bitmap_height;
    do {
        T *s2 = src_bitmap.get_scanline<T>(sy.w+1);
        T *s = src_bitmap.get_scanline<T>(sy.w);
        T *d = (T*)drow;

        mul = (T)(sy.f >> fshift);

        {
            unsigned int m = (mul & (~1U)) << (15 - pshift); // 16-bit MMX multiply (signed bit), remove one bit to match precision
            mul256 = _mm256_set_epi16(
                m,m,m,m,m,m,m,m,
                m,m,m,m,m,m,m,m);
        }

        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_avx_argb8(vinterp_tmp.tmp,(__m256i*)s,(__m256i*)s2,mul256,src_bitmap_width_m256,rmask256);
                else
                    rerender_line_bilinear_vinterp_stage_avx_rgb16(vinterp_tmp.tmp,(__m256i*)s,(__m256i*)s2,mul256,src_bitmap_width_m256,
                        rmask256,rs,gmask256,gs,bmask256,bs);

                rerender_line_bilinear_hinterp_stage<T>(d,(T*)vinterp_tmp.tmp,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_avx_argb8((__m256i*)d,(__m256i*)s,(__m256i*)s2,mul256,src_bitmap_width_m256,rmask256);
                else
                    rerender_line_bilinear_vinterp_stage_avx_rgb16((__m256i*)d,(__m256i*)s,(__m256i*)s2,mul256,src_bitmap_width_m256,
                        rmask256,rs,gmask256,gs,bmask256,bs);
            }
        }
        else {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, no vertical interpolation
                rerender_line_bilinear_hinterp_stage<T>(d,s,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // copy the scanline 1:1 no interpolation
                memcpy(d,s,bitmap_width*sizeof(T));
            }
        }

        if ((--oy) == 0) break;
        drow += x_image->bytes_per_line;
        sy += stepy;
    } while (1);
#else
    fprintf(stderr,"Compiler does not support AVX2\n");
#endif
}

template <class T> void rerender_out_bilinear_arm_neon() {
#if HAVE_CPU_ARM_NEON
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<int16x8_t> vinterp_tmp;
    const T alpha = 
        (T)(~(x_image->red_mask+x_image->green_mask+x_image->blue_mask));
    const T rbmask = (T)(x_image->red_mask+x_image->blue_mask);
    const T abmask = (T)x_image->green_mask + alpha;
    int16x8_t rmask128,gmask128,bmask128;
    const size_t pixels_per_group =
        sizeof(int16x8_t) / sizeof(T);
    unsigned char *drow;
    uint16_t rm,gm,bm;
    uint8_t rs,gs,bs;
    int16_t mul128;
    size_t ox,oy;
    T fshift;
    T pshift;
    T fmax;
    T mul;

    // do not run this function if NEON extensions are not present
    if (!hostCPUcaps.neon) {
        fprintf(stderr,"CPU does not support NEON\n");
        return;
    }

    rs = bitscan_forward(x_image->red_mask,0);
    rm = bitscan_count(x_image->red_mask,rs) - rs;

    gs = bitscan_forward(x_image->green_mask,0);
    gm = bitscan_count(x_image->green_mask,gs) - gs;

    bs = bitscan_forward(x_image->blue_mask,0);
    bm = bitscan_count(x_image->blue_mask,bs) - bs;

    fshift = std::min(rm,std::min(gm,bm));
    pshift = fshift;
    fshift = (sizeof(nr_wftype) * 8) - fshift;

    // this code WILL fault if base or stride are not multiple of 16!
    if ((src_bitmap_stride & 15) != 0 || ((size_t)src_bitmap & 15) != 0) {
        fprintf(stderr,"Source bitmap not NEON usable (base=%p stride=0x%x\n",(void*)src_bitmap,(unsigned int)src_bitmap_stride);
        return;
    }
    if (((size_t)x_image->data & 15) != 0 || (x_image->bytes_per_line & 15) != 0) {
        fprintf(stderr,"Target bitmap not NEON usable (base=%p stride=0x%x\n",(void*)x_image->data,(unsigned int)x_image->bytes_per_line);
        return;
    }

    if (sizeof(T) == 4) {
        // 32bpp this code can only handle the 8-bit RGBA/ARGB case, else R/G/B fields cross 16-bit boundaries
        if (pshift != 8) return;
        if (bm != 8 || gm != 8 || rm != 8) return; // each field, 8 bits
        if ((rs&7) != 0 || (gs&7) != 0 || (bs&7) != 0) return; // each field, starts on 8-bit boundaries

        rmask128 = (int16x8_t){0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    }
    else {
        // 16bpp this code can handle any case
        if (pshift > 15) return;

        rmask128 = (int16x8_t){
            (int16_t)((1 << rm) - 1),   (int16_t)((1 << rm) - 1),
            (int16_t)((1 << rm) - 1),   (int16_t)((1 << rm) - 1),
            (int16_t)((1 << rm) - 1),   (int16_t)((1 << rm) - 1),
            (int16_t)((1 << rm) - 1),   (int16_t)((1 << rm) - 1)
        };

        gmask128 = (int16x8_t){
            (int16_t)((1 << gm) - 1),   (int16_t)((1 << gm) - 1),
            (int16_t)((1 << gm) - 1),   (int16_t)((1 << gm) - 1),
            (int16_t)((1 << gm) - 1),   (int16_t)((1 << gm) - 1),
            (int16_t)((1 << gm) - 1),   (int16_t)((1 << gm) - 1)
        };

        bmask128 = (int16x8_t){
            (int16_t)((1 << bm) - 1),   (int16_t)((1 << bm) - 1),
            (int16_t)((1 << bm) - 1),   (int16_t)((1 << bm) - 1),
            (int16_t)((1 << bm) - 1),   (int16_t)((1 << bm) - 1),
            (int16_t)((1 << bm) - 1),   (int16_t)((1 << bm) - 1)
        };
    }

    fmax = 1U << pshift;

    render_scale_from_sd(/*&*/stepx,bitmap_width,src_bitmap_width);
    render_scale_from_sd(/*&*/stepy,bitmap_height,src_bitmap_height);

    unsigned int src_bitmap_width_groups = (src_bitmap_width + pixels_per_group - 1) / pixels_per_group;

    if (bitmap_width == 0 || src_bitmap_width_groups == 0) return;

    drow = (unsigned char*)x_image->data;
    oy = bitmap_height;
    do {
        T *s2 = src_bitmap.get_scanline<T>(sy.w+1);
        T *s = src_bitmap.get_scanline<T>(sy.w);
        T *d = (T*)drow;

        mul = (T)(sy.f >> fshift);
        mul128 = (mul & (~1U)) << (15 - pshift); // 16-bit MMX multiply (signed bit), remove one bit to match precision

        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_arm_neon_argb8(vinterp_tmp.tmp,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,rmask128);
                else
                    rerender_line_bilinear_vinterp_stage_arm_neon_rgb16(vinterp_tmp.tmp,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,
                        rmask128,rs,gmask128,gs,bmask128,bs);

                rerender_line_bilinear_hinterp_stage<T>(d,(T*)vinterp_tmp.tmp,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_arm_neon_argb8((int16x8_t*)d,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,rmask128);
                else
                    rerender_line_bilinear_vinterp_stage_arm_neon_rgb16((int16x8_t*)d,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,
                        rmask128,rs,gmask128,gs,bmask128,bs);
            }
        }
        else {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, no vertical interpolation
                rerender_line_bilinear_hinterp_stage<T>(d,s,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // copy the scanline 1:1 no interpolation
                memcpy(d,s,bitmap_width*sizeof(T));
            }
        }

        if ((--oy) == 0) break;
        drow += x_image->bytes_per_line;
        sy += stepy;
    } while (1);
#else
    fprintf(stderr,"Compiler does not support ARM NEON\n");
#endif
}

void rerender_out() {
    if (method == 0) {
        fprintf(stderr,"Neighbor\n");
        if (x_image->bits_per_pixel == 32)
            rerender_out_neighbor<uint32_t>();
        else if (x_image->bits_per_pixel == 16)
            rerender_out_neighbor<uint16_t>();
        else if (x_image->bits_per_pixel == 8)
            rerender_out_neighbor<uint8_t>();
    }
    else if (method == 1) {
        fprintf(stderr,"Basic bilinear\n");
        if (x_image->bits_per_pixel == 32)
            rerender_out_bilinear<uint32_t>();
        else if (x_image->bits_per_pixel == 16)
            rerender_out_bilinear<uint16_t>();
    }
    else if (method == 2) {
        fprintf(stderr,"Basic bilinear MMX\n");
        if (x_image->bits_per_pixel == 32)
            rerender_out_bilinear_mmx<uint32_t>();
        else if (x_image->bits_per_pixel == 16)
            rerender_out_bilinear_mmx<uint16_t>();
    }
    else if (method == 3) {
        fprintf(stderr,"Basic bilinear SSE\n"); // Well, SSE2. The base SSE set is geared for floating point and doesn't give us what we need.
        if (x_image->bits_per_pixel == 32)
            rerender_out_bilinear_sse<uint32_t>();
        else if (x_image->bits_per_pixel == 16)
            rerender_out_bilinear_sse<uint16_t>();
    }
    else if (method == 4) {
        fprintf(stderr,"Basic bilinear AVX\n"); // Well, AVX2, actually
        if (x_image->bits_per_pixel == 32)
            rerender_out_bilinear_avx<uint32_t>();
        else if (x_image->bits_per_pixel == 16)
            rerender_out_bilinear_avx<uint16_t>();
    }
    else if (method == 5) {
        fprintf(stderr,"Basic bilinear ARM NEON\n");
        if (x_image->bits_per_pixel == 32)
            rerender_out_bilinear_arm_neon<uint32_t>();
        else if (x_image->bits_per_pixel == 16)
            rerender_out_bilinear_arm_neon<uint16_t>();
    }
}

int main() {
	int redraw = 1;

    hostCPUcaps.detect();

	memset(&x_shminfo,0,sizeof(x_shminfo));

	/* WARNING: There are a LOT of steps involved at this level of X-windows Xlib interaction! */

	if ((x_display=XOpenDisplay(NULL)) == NULL) {
		if ((x_display=XOpenDisplay(":0")) == NULL) {
			fprintf(stderr,"Unable to open X display\n");
			return 1;
		}
	}

	x_root_window = DefaultRootWindow(x_display);
	x_screen_index = DefaultScreen(x_display);
	x_screen = XScreenOfDisplay(x_display,x_screen_index);
	x_depth = DefaultDepthOfScreen(x_screen);

	fprintf(stderr,"Root window: id=%lu depth=%lu screenindex=%d\n",
		(unsigned long)x_root_window,
		(unsigned long)x_depth,
		x_screen_index);

	/* we want to respond to WM_DELETE_WINDOW */
	x_wmDelete = XInternAtom(x_display,"WM_DELETE_WINDOW", True);

	if ((x_visual=DefaultVisualOfScreen(x_screen)) == NULL) {
		fprintf(stderr,"Cannot get default visual\n");
		return 1;
	}

	x_cmap = XCreateColormap(x_display, x_root_window, x_visual, AllocNone);

	x_swa.colormap = x_cmap;
	x_swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask; /* send us expose events, keypress/keyrelease events */

	x_window = XCreateWindow(x_display, x_root_window, 0, 0, 640, 480, 0, x_depth, InputOutput, x_visual, CWEventMask, &x_swa);

	XMapWindow(x_display, x_window);
	XMoveWindow(x_display, x_window, 0, 0);
	XStoreName(x_display, x_window, "Hello world");

	XSetWMProtocols(x_display, x_window, &x_wmDelete, 1); /* please send us WM_DELETE_WINDOW event */

	XGetWindowAttributes(x_display, x_window, &x_gwa);

	/* now we need an XImage. Try to use SHM (shared memory) mapping if possible, else, don't */
	if (!init_shm()) {
		if (!init_norm()) {
			fprintf(stderr,"Cannot alloc bitmap\n");
			return 1;
		}
	}

    assert(x_image != NULL);
    x_rgbinfo.r.setByMask(x_image->red_mask);
    x_rgbinfo.g.setByMask(x_image->green_mask);
    x_rgbinfo.b.setByMask(x_image->blue_mask);
    x_rgbinfo.a.setByMask(~(x_image->red_mask+x_image->green_mask+x_image->blue_mask)); // alpha = anything not covered by R,G,B

    fprintf(stderr,"R/G/B/A shift/width/mask %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X\n",
        x_rgbinfo.r.shift, x_rgbinfo.r.bwidth, x_rgbinfo.r.bmask,
        x_rgbinfo.g.shift, x_rgbinfo.g.bwidth, x_rgbinfo.g.bmask,
        x_rgbinfo.b.shift, x_rgbinfo.b.bwidth, x_rgbinfo.b.bmask,
        x_rgbinfo.a.shift, x_rgbinfo.a.bwidth, x_rgbinfo.a.bmask);

    /* src bitmap */
    src_bitmap.width = 640;
    src_bitmap.height = 480;
    src_bitmap.bytes_per_pixel = (x_image->bits_per_pixel + 7) / 8;
    src_bitmap.stride_align = 32;
    src_bitmap.rgbinfo = x_rgbinfo;
    src_bitmap.update_stride_from_width();
    src_bitmap.update_length_from_stride_and_height();
#if HAVE_POSIX_MEMALIGN
    if (posix_memalign((void**)(&src_bitmap.base),src_bitmap.stride_align,src_bitmap.length)) {
        fprintf(stderr,"Cannot alloc src bitmap\n");
        return 1;
    }
#else
    // WARNING: malloc() is not necessarily safe for SSE and AVX use because it cannot guarantee alignment.
    // On Linux x86_64 malloc() seems to generally auto align to 16-byte boundaries which is OK for SSE but
    // can break AVX.
    src_bitmap.base = (unsigned char*)malloc(src_bitmap_length);
    if (src_bitmap.base == NULL) {
        fprintf(stderr,"Cannot alloc src bitmap\n");
        return 1;
    }
#endif
    src_bitmap.update_canvas_from_base();
    assert(src_bitmap.canvas != NULL);
    assert(src_bitmap.base != NULL);
    assert(src_bitmap.is_valid());

    /* make up something */
    src_bitmap_render(/*&*/src_bitmap);

	rerender_out();

	/* wait for events */
	while (!x_quit) {
		/* you can skip XSync and go straight to XPending() if you are doing animation */
		if (XPending(x_display)) {
			XNextEvent(x_display, &x_event);

			if (x_event.type == Expose) {
				fprintf(stderr,"Expose event\n");
				redraw = 1;

				/* this also seems to be a good way to detect window resize */
				{
					int pw = x_gwa.width;
					int ph = x_gwa.height;

					XGetWindowAttributes(x_display, x_window, &x_gwa);

					if (pw != x_gwa.width || ph != x_gwa.height) {
						close_bitmap();
						if (!init_shm()) {
							if (!init_norm()) {
								fprintf(stderr,"Cannot alloc bitmap\n");
								return 1;
							}
						}

						rerender_out();
					}
				}
			}
			else if (x_event.type == KeyPress) {
				char buffer[80];
				KeySym sym=0;

				XLookupString(&x_event.xkey, buffer, sizeof(buffer), &sym, NULL);

				if (sym == XK_Escape) {
					fprintf(stderr,"Exit, by ESC\n");
					x_quit = 1;
				}
                else if (sym == XK_space) {
                    if ((++method) >= 6)
                        method = 0;

                    rerender_out();
                    redraw = 1;
                }
			}
			else if (x_event.type == ClientMessage) {
				if ((Atom)x_event.xclient.data.l[0] == x_wmDelete) {
					fprintf(stderr,"Exit, by window close\n");
					x_quit = 1;
				}
			}
		}

		if (redraw) {
			update_to_X11();
			redraw = 0;
		}

		XSync(x_display,False);
	}

	/* also a lot involved for cleanup */
	close_bitmap();
    free(src_bitmap.base);
	XDestroyWindow(x_display,x_window);
	XCloseDisplay(x_display);
	return 0;
}
