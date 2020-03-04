
enum {
    GDC_MASTER=0,
    GDC_SLAVE=1
};

struct PC98_GDC_state {
    PC98_GDC_state();
    void reset_fifo(void);
    void reset_rfifo(void);
    void flush_fifo_old(void);
    bool write_fifo(const uint16_t c);
    bool write_rfifo(const uint8_t c);
    bool write_fifo_command(const unsigned char c);
    bool write_fifo_param(const unsigned char c);
    bool rfifo_has_content(void);
    uint8_t read_status(void);
    uint8_t rfifo_read_data(void);
    void idle_proc(void);

    void force_fifo_complete(void);
    void take_cursor_char_setup(unsigned char bi);
    void take_cursor_pos(unsigned char bi);
    void take_reset_sync_parameters(void);
    void cursor_advance(void);

    void begin_frame(void);
    void next_line(void);

    void load_display_partition(void);
    void next_display_partition(void);

    size_t fifo_can_read(void);
    bool fifo_empty(void);
    Bit16u read_fifo(void);

    /* NTS:
     *
     * We're following the Neko Project II method of FIFO emulation BUT
     * I wonder if the GDC maintains two FIFOs and allows stacking params
     * in one and commands in another....? */

    uint8_t                 cmd_parm_tmp[8];            /* temp storage before accepting params */

    uint8_t                 param_ram[16];
    uint8_t                 param_ram_wptr;

    uint16_t                scan_address;
    uint8_t                 row_height;
    uint8_t                 row_line;

    uint8_t                 display_partition;
    uint16_t                display_partition_rem_lines;
    uint8_t                 display_partition_mask;

    uint16_t                active_display_lines;       /* AL (translated) */
    uint16_t                active_display_words_per_line;/* AW bits (translated) */
    uint16_t                display_pitch;
    uint8_t                 horizontal_sync_width;      /* HS (translated) */
    uint8_t                 vertical_sync_width;        /* VS (translated) */
    uint8_t                 horizontal_front_porch_width;/* HFP (translated) */
    uint8_t                 horizontal_back_porch_width;/* HBP (translated) */
    uint8_t                 vertical_front_porch_width; /* VFP (translated) */
    uint8_t                 vertical_back_porch_width;  /* VBP (translated) */
    uint8_t                 display_mode;               /* CG bits */
            /* CG = 00 = mixed graphics & character
             * CG = 01 = graphics mode
             * CG = 10 = character mode
             * CG = 11 = invalid */
    uint8_t                 video_framing;              /* IS bits */
            /* IS = 00 = non-interlaced
             * IS = 01 = invalid
             * IS = 10 = interlaced repeat field for character displays
             * IS = 11 = interlaced */
    uint8_t                 current_command;
    uint8_t                 proc_step;
    uint8_t                 cursor_blink_state;
    uint8_t                 cursor_blink_count;         /* count from 0 to BR - 1 */
    uint8_t                 cursor_blink_rate;          /* BR */
    bool                    draw_only_during_retrace;   /* F bits */
    bool                    dynamic_ram_refresh;        /* D bits */
    bool                    master_sync;                /* master source generation */
    bool                    display_enable;
    bool                    cursor_enable;
    bool                    cursor_blink;
    bool                    IM_bit;                     /* display partition, IM bit */
    bool                    idle;

    bool                    doublescan;                 /* 200-line as 400-line */
};

