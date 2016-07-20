
    bits    16
    org     100h

    nop
    cbw
    cwd
    cli
    sti
    pushf
    popf
    ret
    ret     4
    retf
    retf    4
    iret
    int     1
    int     3
    int3
    int     0x10
    int     0x21
    int     0xAB
    in      al,0x44
    in      al,dx
    in      ax,0x44
    in      ax,dx
    out     0x44,al
    out     dx,al
    out     0x44,ax
    out     dx,ax

    inc     ax
    inc     cx
    inc     dx
    inc     bx
    inc     sp
    inc     bp
    inc     si
    inc     di

    dec     ax
    dec     cx
    dec     dx
    dec     bx
    dec     sp
    dec     bp
    dec     si
    dec     di

    push    ax
    push    cx
    push    dx
    push    bx
    push    sp
    push    bp
    push    si
    push    di

    pop     ax
    pop     cx
    pop     dx
    pop     bx
    pop     sp
    pop     bp
    pop     si
    pop     di

    cs
    nop
    ds
    nop
    es
    nop
    ss
    nop

    xlatb

    movsb
    movsw
    cmpsb
    cmpsw
    stosb
    stosw
    lodsb
    lodsw
    scasb
    scasw

    hlt
    cmc
    nop
    clc
    stc
    cli
    sti
    cld
    std

    daa
    aaa
    das
    aas

    aam
    aad
    aam     0x17
    aad     0x17

    repz
    movsb
    repz
    movsw
    repz
    cmpsb
    repz
    cmpsw
    repz
    stosb
    repz
    stosw
    repz
    lodsb
    repz
    lodsw
    repz
    scasb
    repz
    scasw

    repnz
    movsb
    repnz
    movsw
    repnz
    cmpsb
    repnz
    cmpsw
    repnz
    stosb
    repnz
    stosw
    repnz
    lodsb
    repnz
    lodsw
    repnz
    scasb
    repnz
    scasw

    lock    xchg    ax,ax

    xchg    cx,ax
    xchg    dx,ax
    xchg    bx,ax
    xchg    sp,ax
    xchg    bp,ax
    xchg    si,ax
    xchg    di,ax

    wait
    sahf
    lahf

    push    es
    pop     es
    push    cs
    db      0x0F    ; pop cs
    nop
    nop
    push    ss
    pop     ss
    push    ds
    pop     ds

jmp1:
    jo      short jmp1
    jno     short jmp1
    jb      short jmp1
    jnb     short jmp1
    jz      short jmp1
    jnz     short jmp1
    jbe     short jmp1
    ja      short jmp1
    js      short jmp1
    jns     short jmp1
    jpe     short jmp1
    jpo     short jmp1
    jl      short jmp1
    jge     short jmp1
    jle     short jmp1
    jg      short jmp1
    jmp     short jmp1

;--------------byte
    add     byte [bx+si],bl ; 00 m/r/m mod=0
    add     byte [bx+di],bl ; 00 m/r/m mod=0
    add     byte [bp+si],bl ; 00 m/r/m mod=0
    add     byte [bp+di],bl ; 00 m/r/m mod=0
    add     byte [si],bl    ; 00 m/r/m mod=0
    add     byte [di],bl    ; 00 m/r/m mod=0
    add     byte [0x1234],bl; 00 m/r/m mod=0
    add     byte [bx],bl    ; 00 m/r/m mod=0

    add     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bx+si-0x5F],bl
    add     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bx+di-0x5F],bl
    add     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bp+si-0x5F],bl
    add     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bp+di-0x5F],bl
    add     byte [si+0x5F],bl ; 40 m/r/m mod=1
    add     byte [si-0x5F],bl
    add     byte [di+0x5F],bl ; 40 m/r/m mod=1
    add     byte [di-0x5F],bl
    add     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bp-0x5F],bl
    add     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bx-0x5F],bl

    add     byte [bx+si+0x1234],bl
    add     byte [bx+di+0x1234],bl
    add     byte [bp+si+0x1234],bl
    add     byte [bp+di+0x1234],bl
    add     byte [si+0x1234],bl
    add     byte [di+0x1234],bl
    add     byte [bp+0x1234],bl
    add     byte [bx+0x1234],bl
    add     bl,al           ; 00 m/r/m mod=3
    add     bl,cl           ; 00 m/r/m mod=3
    add     bl,dl           ; 00 m/r/m mod=3
    add     bl,bl           ; 00 m/r/m mod=3
    add     bl,ah
    add     bl,ch
    add     bl,dh
    add     bl,bh

;--------------word
    add     word [bx+si],bx ; 00 m/r/m mod=0
    add     word [bx+di],bx ; 00 m/r/m mod=0
    add     word [bp+si],bx ; 00 m/r/m mod=0
    add     word [bp+di],bx ; 00 m/r/m mod=0
    add     word [si],bx    ; 00 m/r/m mod=0
    add     word [di],bx    ; 00 m/r/m mod=0
    add     word [0x1234],bx; 00 m/r/m mod=0
    add     word [bx],bx    ; 00 m/r/m mod=0

    add     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    add     word [bx+si-0x5F],bx
    add     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    add     word [bx+di-0x5F],bx
    add     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    add     word [bp+si-0x5F],bx
    add     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    add     word [bp+di-0x5F],bx
    add     word [si+0x5F],bx ; 40 m/r/m mod=1
    add     word [si-0x5F],bx
    add     word [di+0x5F],bx ; 40 m/r/m mod=1
    add     word [di-0x5F],bx
    add     word [bp+0x5F],bx ; 40 m/r/m mod=1
    add     word [bp-0x5F],bx
    add     word [bx+0x5F],bx ; 40 m/r/m mod=1
    add     word [bx-0x5F],bx

    add     word [bx+si+0x1234],bx
    add     word [bx+di+0x1234],bx
    add     word [bp+si+0x1234],bx
    add     word [bp+di+0x1234],bx
    add     word [si+0x1234],bx
    add     word [di+0x1234],bx
    add     word [bp+0x1234],bx
    add     word [bx+0x1234],bx
    add     bx,ax           ; 00 m/r/m mod=3
    add     bx,cx           ; 00 m/r/m mod=3
    add     bx,dx           ; 00 m/r/m mod=3
    add     bx,bx           ; 00 m/r/m mod=3
    add     bx,si
    add     bx,di
    add     bx,bp
    add     bx,sp

;--------------byte
    add     bl,byte [bx+si] ; 00 m/r/m mod=0
    add     bl,byte [bx+di] ; 00 m/r/m mod=0
    add     bl,byte [bp+si] ; 00 m/r/m mod=0
    add     bl,byte [bp+di] ; 00 m/r/m mod=0
    add     bl,byte [si]    ; 00 m/r/m mod=0
    add     bl,byte [di]    ; 00 m/r/m mod=0
    add     bl,byte [0x1234]; 00 m/r/m mod=0
    add     bl,byte [bx]    ; 00 m/r/m mod=0

    add     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bx+si-0x5F]
    add     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bx+di-0x5F]
    add     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bp+si-0x5F]
    add     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bp+di-0x5F]
    add     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [si-0x5F]
    add     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [di-0x5F]
    add     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bp-0x5F]
    add     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bx-0x5F]

    add     bl,byte [bx+si+0x1234]
    add     bl,byte [bx+di+0x1234]
    add     bl,byte [bp+si+0x1234]
    add     bl,byte [bp+di+0x1234]
    add     bl,byte [si+0x1234]
    add     bl,byte [di+0x1234]
    add     bl,byte [bp+0x1234]
    add     bl,byte [bx+0x1234]

;--------------word
    add     bx,word [bx+si] ; 00 m/r/m mod=0
    add     bx,word [bx+di] ; 00 m/r/m mod=0
    add     bx,word [bp+si] ; 00 m/r/m mod=0
    add     bx,word [bp+di] ; 00 m/r/m mod=0
    add     bx,word [si]    ; 00 m/r/m mod=0
    add     bx,word [di]    ; 00 m/r/m mod=0
    add     bx,word [0x1234]; 00 m/r/m mod=0
    add     bx,word [bx]    ; 00 m/r/m mod=0

    add     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bx+si-0x5F]
    add     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bx+di-0x5F]
    add     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bp+si-0x5F]
    add     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bp+di-0x5F]
    add     bx,word [si+0x5F] ; 40 m/r/m mod=1
    add     bx,word [si-0x5F]
    add     bx,word [di+0x5F] ; 40 m/r/m mod=1
    add     bx,word [di-0x5F]
    add     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bp-0x5F]
    add     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bx-0x5F]

    add     bx,word [bx+si+0x1234]
    add     bx,word [bx+di+0x1234]
    add     bx,word [bp+si+0x1234]
    add     bx,word [bp+di+0x1234]
    add     bx,word [si+0x1234]
    add     bx,word [di+0x1234]
    add     bx,word [bp+0x1234]
    add     bx,word [bx+0x1234]

;---------------add imm
    add     al,0x12
    add     al,0xEF
    add     ax,0x1234
    add     ax,0xFEDC

;--------------byte
    or      byte [bx+si],bl ; 00 m/r/m mod=0
    or      byte [bx+di],bl ; 00 m/r/m mod=0
    or      byte [bp+si],bl ; 00 m/r/m mod=0
    or      byte [bp+di],bl ; 00 m/r/m mod=0
    or      byte [si],bl    ; 00 m/r/m mod=0
    or      byte [di],bl    ; 00 m/r/m mod=0
    or      byte [0x1234],bl; 00 m/r/m mod=0
    or      byte [bx],bl    ; 00 m/r/m mod=0

    or      byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bx+si-0x5F],bl
    or      byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bx+di-0x5F],bl
    or      byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bp+si-0x5F],bl
    or      byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bp+di-0x5F],bl
    or      byte [si+0x5F],bl ; 40 m/r/m mod=1
    or      byte [si-0x5F],bl
    or      byte [di+0x5F],bl ; 40 m/r/m mod=1
    or      byte [di-0x5F],bl
    or      byte [bp+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bp-0x5F],bl
    or      byte [bx+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bx-0x5F],bl

    or      byte [bx+si+0x1234],bl
    or      byte [bx+di+0x1234],bl
    or      byte [bp+si+0x1234],bl
    or      byte [bp+di+0x1234],bl
    or      byte [si+0x1234],bl
    or      byte [di+0x1234],bl
    or      byte [bp+0x1234],bl
    or      byte [bx+0x1234],bl
    or      bl,al           ; 00 m/r/m mod=3
    or      bl,cl           ; 00 m/r/m mod=3
    or      bl,dl           ; 00 m/r/m mod=3
    or      bl,bl           ; 00 m/r/m mod=3
    or      bl,ah
    or      bl,ch
    or      bl,dh
    or      bl,bh

;--------------word
    or      word [bx+si],bx ; 00 m/r/m mod=0
    or      word [bx+di],bx ; 00 m/r/m mod=0
    or      word [bp+si],bx ; 00 m/r/m mod=0
    or      word [bp+di],bx ; 00 m/r/m mod=0
    or      word [si],bx    ; 00 m/r/m mod=0
    or      word [di],bx    ; 00 m/r/m mod=0
    or      word [0x1234],bx; 00 m/r/m mod=0
    or      word [bx],bx    ; 00 m/r/m mod=0

    or      word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    or      word [bx+si-0x5F],bx
    or      word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    or      word [bx+di-0x5F],bx
    or      word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    or      word [bp+si-0x5F],bx
    or      word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    or      word [bp+di-0x5F],bx
    or      word [si+0x5F],bx ; 40 m/r/m mod=1
    or      word [si-0x5F],bx
    or      word [di+0x5F],bx ; 40 m/r/m mod=1
    or      word [di-0x5F],bx
    or      word [bp+0x5F],bx ; 40 m/r/m mod=1
    or      word [bp-0x5F],bx
    or      word [bx+0x5F],bx ; 40 m/r/m mod=1
    or      word [bx-0x5F],bx

    or      word [bx+si+0x1234],bx
    or      word [bx+di+0x1234],bx
    or      word [bp+si+0x1234],bx
    or      word [bp+di+0x1234],bx
    or      word [si+0x1234],bx
    or      word [di+0x1234],bx
    or      word [bp+0x1234],bx
    or      word [bx+0x1234],bx
    or      bx,ax           ; 00 m/r/m mod=3
    or      bx,cx           ; 00 m/r/m mod=3
    or      bx,dx           ; 00 m/r/m mod=3
    or      bx,bx           ; 00 m/r/m mod=3
    or      bx,si
    or      bx,di
    or      bx,bp
    or      bx,sp

;--------------byte
    or      bl,byte [bx+si] ; 00 m/r/m mod=0
    or      bl,byte [bx+di] ; 00 m/r/m mod=0
    or      bl,byte [bp+si] ; 00 m/r/m mod=0
    or      bl,byte [bp+di] ; 00 m/r/m mod=0
    or      bl,byte [si]    ; 00 m/r/m mod=0
    or      bl,byte [di]    ; 00 m/r/m mod=0
    or      bl,byte [0x1234]; 00 m/r/m mod=0
    or      bl,byte [bx]    ; 00 m/r/m mod=0

    or      bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bx+si-0x5F]
    or      bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bx+di-0x5F]
    or      bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bp+si-0x5F]
    or      bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bp+di-0x5F]
    or      bl,byte [si+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [si-0x5F]
    or      bl,byte [di+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [di-0x5F]
    or      bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bp-0x5F]
    or      bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bx-0x5F]

    or      bl,byte [bx+si+0x1234]
    or      bl,byte [bx+di+0x1234]
    or      bl,byte [bp+si+0x1234]
    or      bl,byte [bp+di+0x1234]
    or      bl,byte [si+0x1234]
    or      bl,byte [di+0x1234]
    or      bl,byte [bp+0x1234]
    or      bl,byte [bx+0x1234]

;--------------word
    or      bx,word [bx+si] ; 00 m/r/m mod=0
    or      bx,word [bx+di] ; 00 m/r/m mod=0
    or      bx,word [bp+si] ; 00 m/r/m mod=0
    or      bx,word [bp+di] ; 00 m/r/m mod=0
    or      bx,word [si]    ; 00 m/r/m mod=0
    or      bx,word [di]    ; 00 m/r/m mod=0
    or      bx,word [0x1234]; 00 m/r/m mod=0
    or      bx,word [bx]    ; 00 m/r/m mod=0

    or      bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bx+si-0x5F]
    or      bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bx+di-0x5F]
    or      bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bp+si-0x5F]
    or      bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bp+di-0x5F]
    or      bx,word [si+0x5F] ; 40 m/r/m mod=1
    or      bx,word [si-0x5F]
    or      bx,word [di+0x5F] ; 40 m/r/m mod=1
    or      bx,word [di-0x5F]
    or      bx,word [bp+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bp-0x5F]
    or      bx,word [bx+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bx-0x5F]

    or      bx,word [bx+si+0x1234]
    or      bx,word [bx+di+0x1234]
    or      bx,word [bp+si+0x1234]
    or      bx,word [bp+di+0x1234]
    or      bx,word [si+0x1234]
    or      bx,word [di+0x1234]
    or      bx,word [bp+0x1234]
    or      bx,word [bx+0x1234]

;---------------or  imm
    or      al,0x12
    or      al,0xEF
    or      ax,0x1234
    or      ax,0xFEDC

;--------------byte
    adc     byte [bx+si],bl ; 00 m/r/m mod=0
    adc     byte [bx+di],bl ; 00 m/r/m mod=0
    adc     byte [bp+si],bl ; 00 m/r/m mod=0
    adc     byte [bp+di],bl ; 00 m/r/m mod=0
    adc     byte [si],bl    ; 00 m/r/m mod=0
    adc     byte [di],bl    ; 00 m/r/m mod=0
    adc     byte [0x1234],bl; 00 m/r/m mod=0
    adc     byte [bx],bl    ; 00 m/r/m mod=0

    adc     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bx+si-0x5F],bl
    adc     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bx+di-0x5F],bl
    adc     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bp+si-0x5F],bl
    adc     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bp+di-0x5F],bl
    adc     byte [si+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [si-0x5F],bl
    adc     byte [di+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [di-0x5F],bl
    adc     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bp-0x5F],bl
    adc     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bx-0x5F],bl

    adc     byte [bx+si+0x1234],bl
    adc     byte [bx+di+0x1234],bl
    adc     byte [bp+si+0x1234],bl
    adc     byte [bp+di+0x1234],bl
    adc     byte [si+0x1234],bl
    adc     byte [di+0x1234],bl
    adc     byte [bp+0x1234],bl
    adc     byte [bx+0x1234],bl
    adc     bl,al           ; 00 m/r/m mod=3
    adc     bl,cl           ; 00 m/r/m mod=3
    adc     bl,dl           ; 00 m/r/m mod=3
    adc     bl,bl           ; 00 m/r/m mod=3
    adc     bl,ah
    adc     bl,ch
    adc     bl,dh
    adc     bl,bh

;--------------word
    adc     word [bx+si],bx ; 00 m/r/m mod=0
    adc     word [bx+di],bx ; 00 m/r/m mod=0
    adc     word [bp+si],bx ; 00 m/r/m mod=0
    adc     word [bp+di],bx ; 00 m/r/m mod=0
    adc     word [si],bx    ; 00 m/r/m mod=0
    adc     word [di],bx    ; 00 m/r/m mod=0
    adc     word [0x1234],bx; 00 m/r/m mod=0
    adc     word [bx],bx    ; 00 m/r/m mod=0

    adc     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bx+si-0x5F],bx
    adc     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bx+di-0x5F],bx
    adc     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bp+si-0x5F],bx
    adc     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bp+di-0x5F],bx
    adc     word [si+0x5F],bx ; 40 m/r/m mod=1
    adc     word [si-0x5F],bx
    adc     word [di+0x5F],bx ; 40 m/r/m mod=1
    adc     word [di-0x5F],bx
    adc     word [bp+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bp-0x5F],bx
    adc     word [bx+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bx-0x5F],bx

    adc     word [bx+si+0x1234],bx
    adc     word [bx+di+0x1234],bx
    adc     word [bp+si+0x1234],bx
    adc     word [bp+di+0x1234],bx
    adc     word [si+0x1234],bx
    adc     word [di+0x1234],bx
    adc     word [bp+0x1234],bx
    adc     word [bx+0x1234],bx
    adc     bx,ax           ; 00 m/r/m mod=3
    adc     bx,cx           ; 00 m/r/m mod=3
    adc     bx,dx           ; 00 m/r/m mod=3
    adc     bx,bx           ; 00 m/r/m mod=3
    adc     bx,si
    adc     bx,di
    adc     bx,bp
    adc     bx,sp

;--------------byte
    adc     bl,byte [bx+si] ; 00 m/r/m mod=0
    adc     bl,byte [bx+di] ; 00 m/r/m mod=0
    adc     bl,byte [bp+si] ; 00 m/r/m mod=0
    adc     bl,byte [bp+di] ; 00 m/r/m mod=0
    adc     bl,byte [si]    ; 00 m/r/m mod=0
    adc     bl,byte [di]    ; 00 m/r/m mod=0
    adc     bl,byte [0x1234]; 00 m/r/m mod=0
    adc     bl,byte [bx]    ; 00 m/r/m mod=0

    adc     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bx+si-0x5F]
    adc     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bx+di-0x5F]
    adc     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bp+si-0x5F]
    adc     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bp+di-0x5F]
    adc     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [si-0x5F]
    adc     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [di-0x5F]
    adc     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bp-0x5F]
    adc     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bx-0x5F]

    adc     bl,byte [bx+si+0x1234]
    adc     bl,byte [bx+di+0x1234]
    adc     bl,byte [bp+si+0x1234]
    adc     bl,byte [bp+di+0x1234]
    adc     bl,byte [si+0x1234]
    adc     bl,byte [di+0x1234]
    adc     bl,byte [bp+0x1234]
    adc     bl,byte [bx+0x1234]

;--------------word
    adc     bx,word [bx+si] ; 00 m/r/m mod=0
    adc     bx,word [bx+di] ; 00 m/r/m mod=0
    adc     bx,word [bp+si] ; 00 m/r/m mod=0
    adc     bx,word [bp+di] ; 00 m/r/m mod=0
    adc     bx,word [si]    ; 00 m/r/m mod=0
    adc     bx,word [di]    ; 00 m/r/m mod=0
    adc     bx,word [0x1234]; 00 m/r/m mod=0
    adc     bx,word [bx]    ; 00 m/r/m mod=0

    adc     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bx+si-0x5F]
    adc     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bx+di-0x5F]
    adc     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bp+si-0x5F]
    adc     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bp+di-0x5F]
    adc     bx,word [si+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [si-0x5F]
    adc     bx,word [di+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [di-0x5F]
    adc     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bp-0x5F]
    adc     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bx-0x5F]

    adc     bx,word [bx+si+0x1234]
    adc     bx,word [bx+di+0x1234]
    adc     bx,word [bp+si+0x1234]
    adc     bx,word [bp+di+0x1234]
    adc     bx,word [si+0x1234]
    adc     bx,word [di+0x1234]
    adc     bx,word [bp+0x1234]
    adc     bx,word [bx+0x1234]

;---------------adc imm
    adc     al,0x12
    adc     al,0xEF
    adc     ax,0x1234
    adc     ax,0xFEDC

;--------------byte
    sbb     byte [bx+si],bl ; 00 m/r/m mod=0
    sbb     byte [bx+di],bl ; 00 m/r/m mod=0
    sbb     byte [bp+si],bl ; 00 m/r/m mod=0
    sbb     byte [bp+di],bl ; 00 m/r/m mod=0
    sbb     byte [si],bl    ; 00 m/r/m mod=0
    sbb     byte [di],bl    ; 00 m/r/m mod=0
    sbb     byte [0x1234],bl; 00 m/r/m mod=0
    sbb     byte [bx],bl    ; 00 m/r/m mod=0

    sbb     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bx+si-0x5F],bl
    sbb     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bx+di-0x5F],bl
    sbb     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bp+si-0x5F],bl
    sbb     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bp+di-0x5F],bl
    sbb     byte [si+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [si-0x5F],bl
    sbb     byte [di+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [di-0x5F],bl
    sbb     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bp-0x5F],bl
    sbb     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bx-0x5F],bl

    sbb     byte [bx+si+0x1234],bl
    sbb     byte [bx+di+0x1234],bl
    sbb     byte [bp+si+0x1234],bl
    sbb     byte [bp+di+0x1234],bl
    sbb     byte [si+0x1234],bl
    sbb     byte [di+0x1234],bl
    sbb     byte [bp+0x1234],bl
    sbb     byte [bx+0x1234],bl
    sbb     bl,al           ; 00 m/r/m mod=3
    sbb     bl,cl           ; 00 m/r/m mod=3
    sbb     bl,dl           ; 00 m/r/m mod=3
    sbb     bl,bl           ; 00 m/r/m mod=3
    sbb     bl,ah
    sbb     bl,ch
    sbb     bl,dh
    sbb     bl,bh

;--------------word
    sbb     word [bx+si],bx ; 00 m/r/m mod=0
    sbb     word [bx+di],bx ; 00 m/r/m mod=0
    sbb     word [bp+si],bx ; 00 m/r/m mod=0
    sbb     word [bp+di],bx ; 00 m/r/m mod=0
    sbb     word [si],bx    ; 00 m/r/m mod=0
    sbb     word [di],bx    ; 00 m/r/m mod=0
    sbb     word [0x1234],bx; 00 m/r/m mod=0
    sbb     word [bx],bx    ; 00 m/r/m mod=0

    sbb     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bx+si-0x5F],bx
    sbb     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bx+di-0x5F],bx
    sbb     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bp+si-0x5F],bx
    sbb     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bp+di-0x5F],bx
    sbb     word [si+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [si-0x5F],bx
    sbb     word [di+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [di-0x5F],bx
    sbb     word [bp+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bp-0x5F],bx
    sbb     word [bx+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bx-0x5F],bx

    sbb     word [bx+si+0x1234],bx
    sbb     word [bx+di+0x1234],bx
    sbb     word [bp+si+0x1234],bx
    sbb     word [bp+di+0x1234],bx
    sbb     word [si+0x1234],bx
    sbb     word [di+0x1234],bx
    sbb     word [bp+0x1234],bx
    sbb     word [bx+0x1234],bx
    sbb     bx,ax           ; 00 m/r/m mod=3
    sbb     bx,cx           ; 00 m/r/m mod=3
    sbb     bx,dx           ; 00 m/r/m mod=3
    sbb     bx,bx           ; 00 m/r/m mod=3
    sbb     bx,si
    sbb     bx,di
    sbb     bx,bp
    sbb     bx,sp

;--------------byte
    sbb     bl,byte [bx+si] ; 00 m/r/m mod=0
    sbb     bl,byte [bx+di] ; 00 m/r/m mod=0
    sbb     bl,byte [bp+si] ; 00 m/r/m mod=0
    sbb     bl,byte [bp+di] ; 00 m/r/m mod=0
    sbb     bl,byte [si]    ; 00 m/r/m mod=0
    sbb     bl,byte [di]    ; 00 m/r/m mod=0
    sbb     bl,byte [0x1234]; 00 m/r/m mod=0
    sbb     bl,byte [bx]    ; 00 m/r/m mod=0

    sbb     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bx+si-0x5F]
    sbb     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bx+di-0x5F]
    sbb     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bp+si-0x5F]
    sbb     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bp+di-0x5F]
    sbb     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [si-0x5F]
    sbb     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [di-0x5F]
    sbb     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bp-0x5F]
    sbb     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bx-0x5F]

    sbb     bl,byte [bx+si+0x1234]
    sbb     bl,byte [bx+di+0x1234]
    sbb     bl,byte [bp+si+0x1234]
    sbb     bl,byte [bp+di+0x1234]
    sbb     bl,byte [si+0x1234]
    sbb     bl,byte [di+0x1234]
    sbb     bl,byte [bp+0x1234]
    sbb     bl,byte [bx+0x1234]

;--------------word
    sbb     bx,word [bx+si] ; 00 m/r/m mod=0
    sbb     bx,word [bx+di] ; 00 m/r/m mod=0
    sbb     bx,word [bp+si] ; 00 m/r/m mod=0
    sbb     bx,word [bp+di] ; 00 m/r/m mod=0
    sbb     bx,word [si]    ; 00 m/r/m mod=0
    sbb     bx,word [di]    ; 00 m/r/m mod=0
    sbb     bx,word [0x1234]; 00 m/r/m mod=0
    sbb     bx,word [bx]    ; 00 m/r/m mod=0

    sbb     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bx+si-0x5F]
    sbb     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bx+di-0x5F]
    sbb     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bp+si-0x5F]
    sbb     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bp+di-0x5F]
    sbb     bx,word [si+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [si-0x5F]
    sbb     bx,word [di+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [di-0x5F]
    sbb     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bp-0x5F]
    sbb     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bx-0x5F]

    sbb     bx,word [bx+si+0x1234]
    sbb     bx,word [bx+di+0x1234]
    sbb     bx,word [bp+si+0x1234]
    sbb     bx,word [bp+di+0x1234]
    sbb     bx,word [si+0x1234]
    sbb     bx,word [di+0x1234]
    sbb     bx,word [bp+0x1234]
    sbb     bx,word [bx+0x1234]

;---------------sbb imm
    sbb     al,0x12
    sbb     al,0xEF
    sbb     ax,0x1234
    sbb     ax,0xFEDC

;--------------byte
    and     byte [bx+si],bl ; 00 m/r/m mod=0
    and     byte [bx+di],bl ; 00 m/r/m mod=0
    and     byte [bp+si],bl ; 00 m/r/m mod=0
    and     byte [bp+di],bl ; 00 m/r/m mod=0
    and     byte [si],bl    ; 00 m/r/m mod=0
    and     byte [di],bl    ; 00 m/r/m mod=0
    and     byte [0x1234],bl; 00 m/r/m mod=0
    and     byte [bx],bl    ; 00 m/r/m mod=0

    and     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bx+si-0x5F],bl
    and     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bx+di-0x5F],bl
    and     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bp+si-0x5F],bl
    and     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bp+di-0x5F],bl
    and     byte [si+0x5F],bl ; 40 m/r/m mod=1
    and     byte [si-0x5F],bl
    and     byte [di+0x5F],bl ; 40 m/r/m mod=1
    and     byte [di-0x5F],bl
    and     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bp-0x5F],bl
    and     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bx-0x5F],bl

    and     byte [bx+si+0x1234],bl
    and     byte [bx+di+0x1234],bl
    and     byte [bp+si+0x1234],bl
    and     byte [bp+di+0x1234],bl
    and     byte [si+0x1234],bl
    and     byte [di+0x1234],bl
    and     byte [bp+0x1234],bl
    and     byte [bx+0x1234],bl
    and     bl,al           ; 00 m/r/m mod=3
    and     bl,cl           ; 00 m/r/m mod=3
    and     bl,dl           ; 00 m/r/m mod=3
    and     bl,bl           ; 00 m/r/m mod=3
    and     bl,ah
    and     bl,ch
    and     bl,dh
    and     bl,bh

;--------------word
    and     word [bx+si],bx ; 00 m/r/m mod=0
    and     word [bx+di],bx ; 00 m/r/m mod=0
    and     word [bp+si],bx ; 00 m/r/m mod=0
    and     word [bp+di],bx ; 00 m/r/m mod=0
    and     word [si],bx    ; 00 m/r/m mod=0
    and     word [di],bx    ; 00 m/r/m mod=0
    and     word [0x1234],bx; 00 m/r/m mod=0
    and     word [bx],bx    ; 00 m/r/m mod=0

    and     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    and     word [bx+si-0x5F],bx
    and     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    and     word [bx+di-0x5F],bx
    and     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    and     word [bp+si-0x5F],bx
    and     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    and     word [bp+di-0x5F],bx
    and     word [si+0x5F],bx ; 40 m/r/m mod=1
    and     word [si-0x5F],bx
    and     word [di+0x5F],bx ; 40 m/r/m mod=1
    and     word [di-0x5F],bx
    and     word [bp+0x5F],bx ; 40 m/r/m mod=1
    and     word [bp-0x5F],bx
    and     word [bx+0x5F],bx ; 40 m/r/m mod=1
    and     word [bx-0x5F],bx

    and     word [bx+si+0x1234],bx
    and     word [bx+di+0x1234],bx
    and     word [bp+si+0x1234],bx
    and     word [bp+di+0x1234],bx
    and     word [si+0x1234],bx
    and     word [di+0x1234],bx
    and     word [bp+0x1234],bx
    and     word [bx+0x1234],bx
    and     bx,ax           ; 00 m/r/m mod=3
    and     bx,cx           ; 00 m/r/m mod=3
    and     bx,dx           ; 00 m/r/m mod=3
    and     bx,bx           ; 00 m/r/m mod=3
    and     bx,si
    and     bx,di
    and     bx,bp
    and     bx,sp

;--------------byte
    and     bl,byte [bx+si] ; 00 m/r/m mod=0
    and     bl,byte [bx+di] ; 00 m/r/m mod=0
    and     bl,byte [bp+si] ; 00 m/r/m mod=0
    and     bl,byte [bp+di] ; 00 m/r/m mod=0
    and     bl,byte [si]    ; 00 m/r/m mod=0
    and     bl,byte [di]    ; 00 m/r/m mod=0
    and     bl,byte [0x1234]; 00 m/r/m mod=0
    and     bl,byte [bx]    ; 00 m/r/m mod=0

    and     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bx+si-0x5F]
    and     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bx+di-0x5F]
    and     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bp+si-0x5F]
    and     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bp+di-0x5F]
    and     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [si-0x5F]
    and     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [di-0x5F]
    and     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bp-0x5F]
    and     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bx-0x5F]

    and     bl,byte [bx+si+0x1234]
    and     bl,byte [bx+di+0x1234]
    and     bl,byte [bp+si+0x1234]
    and     bl,byte [bp+di+0x1234]
    and     bl,byte [si+0x1234]
    and     bl,byte [di+0x1234]
    and     bl,byte [bp+0x1234]
    and     bl,byte [bx+0x1234]

;--------------word
    and     bx,word [bx+si] ; 00 m/r/m mod=0
    and     bx,word [bx+di] ; 00 m/r/m mod=0
    and     bx,word [bp+si] ; 00 m/r/m mod=0
    and     bx,word [bp+di] ; 00 m/r/m mod=0
    and     bx,word [si]    ; 00 m/r/m mod=0
    and     bx,word [di]    ; 00 m/r/m mod=0
    and     bx,word [0x1234]; 00 m/r/m mod=0
    and     bx,word [bx]    ; 00 m/r/m mod=0

    and     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bx+si-0x5F]
    and     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bx+di-0x5F]
    and     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bp+si-0x5F]
    and     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bp+di-0x5F]
    and     bx,word [si+0x5F] ; 40 m/r/m mod=1
    and     bx,word [si-0x5F]
    and     bx,word [di+0x5F] ; 40 m/r/m mod=1
    and     bx,word [di-0x5F]
    and     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bp-0x5F]
    and     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bx-0x5F]

    and     bx,word [bx+si+0x1234]
    and     bx,word [bx+di+0x1234]
    and     bx,word [bp+si+0x1234]
    and     bx,word [bp+di+0x1234]
    and     bx,word [si+0x1234]
    and     bx,word [di+0x1234]
    and     bx,word [bp+0x1234]
    and     bx,word [bx+0x1234]

;---------------and imm
    and     al,0x12
    and     al,0xEF
    and     ax,0x1234
    and     ax,0xFEDC

;--------------byte
    sub     byte [bx+si],bl ; 00 m/r/m mod=0
    sub     byte [bx+di],bl ; 00 m/r/m mod=0
    sub     byte [bp+si],bl ; 00 m/r/m mod=0
    sub     byte [bp+di],bl ; 00 m/r/m mod=0
    sub     byte [si],bl    ; 00 m/r/m mod=0
    sub     byte [di],bl    ; 00 m/r/m mod=0
    sub     byte [0x1234],bl; 00 m/r/m mod=0
    sub     byte [bx],bl    ; 00 m/r/m mod=0

    sub     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bx+si-0x5F],bl
    sub     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bx+di-0x5F],bl
    sub     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bp+si-0x5F],bl
    sub     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bp+di-0x5F],bl
    sub     byte [si+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [si-0x5F],bl
    sub     byte [di+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [di-0x5F],bl
    sub     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bp-0x5F],bl
    sub     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bx-0x5F],bl

    sub     byte [bx+si+0x1234],bl
    sub     byte [bx+di+0x1234],bl
    sub     byte [bp+si+0x1234],bl
    sub     byte [bp+di+0x1234],bl
    sub     byte [si+0x1234],bl
    sub     byte [di+0x1234],bl
    sub     byte [bp+0x1234],bl
    sub     byte [bx+0x1234],bl
    sub     bl,al           ; 00 m/r/m mod=3
    sub     bl,cl           ; 00 m/r/m mod=3
    sub     bl,dl           ; 00 m/r/m mod=3
    sub     bl,bl           ; 00 m/r/m mod=3
    sub     bl,ah
    sub     bl,ch
    sub     bl,dh
    sub     bl,bh

;--------------word
    sub     word [bx+si],bx ; 00 m/r/m mod=0
    sub     word [bx+di],bx ; 00 m/r/m mod=0
    sub     word [bp+si],bx ; 00 m/r/m mod=0
    sub     word [bp+di],bx ; 00 m/r/m mod=0
    sub     word [si],bx    ; 00 m/r/m mod=0
    sub     word [di],bx    ; 00 m/r/m mod=0
    sub     word [0x1234],bx; 00 m/r/m mod=0
    sub     word [bx],bx    ; 00 m/r/m mod=0

    sub     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bx+si-0x5F],bx
    sub     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bx+di-0x5F],bx
    sub     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bp+si-0x5F],bx
    sub     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bp+di-0x5F],bx
    sub     word [si+0x5F],bx ; 40 m/r/m mod=1
    sub     word [si-0x5F],bx
    sub     word [di+0x5F],bx ; 40 m/r/m mod=1
    sub     word [di-0x5F],bx
    sub     word [bp+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bp-0x5F],bx
    sub     word [bx+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bx-0x5F],bx

    sub     word [bx+si+0x1234],bx
    sub     word [bx+di+0x1234],bx
    sub     word [bp+si+0x1234],bx
    sub     word [bp+di+0x1234],bx
    sub     word [si+0x1234],bx
    sub     word [di+0x1234],bx
    sub     word [bp+0x1234],bx
    sub     word [bx+0x1234],bx
    sub     bx,ax           ; 00 m/r/m mod=3
    sub     bx,cx           ; 00 m/r/m mod=3
    sub     bx,dx           ; 00 m/r/m mod=3
    sub     bx,bx           ; 00 m/r/m mod=3
    sub     bx,si
    sub     bx,di
    sub     bx,bp
    sub     bx,sp

;--------------byte
    sub     bl,byte [bx+si] ; 00 m/r/m mod=0
    sub     bl,byte [bx+di] ; 00 m/r/m mod=0
    sub     bl,byte [bp+si] ; 00 m/r/m mod=0
    sub     bl,byte [bp+di] ; 00 m/r/m mod=0
    sub     bl,byte [si]    ; 00 m/r/m mod=0
    sub     bl,byte [di]    ; 00 m/r/m mod=0
    sub     bl,byte [0x1234]; 00 m/r/m mod=0
    sub     bl,byte [bx]    ; 00 m/r/m mod=0

    sub     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bx+si-0x5F]
    sub     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bx+di-0x5F]
    sub     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bp+si-0x5F]
    sub     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bp+di-0x5F]
    sub     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [si-0x5F]
    sub     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [di-0x5F]
    sub     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bp-0x5F]
    sub     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bx-0x5F]

    sub     bl,byte [bx+si+0x1234]
    sub     bl,byte [bx+di+0x1234]
    sub     bl,byte [bp+si+0x1234]
    sub     bl,byte [bp+di+0x1234]
    sub     bl,byte [si+0x1234]
    sub     bl,byte [di+0x1234]
    sub     bl,byte [bp+0x1234]
    sub     bl,byte [bx+0x1234]

;--------------word
    sub     bx,word [bx+si] ; 00 m/r/m mod=0
    sub     bx,word [bx+di] ; 00 m/r/m mod=0
    sub     bx,word [bp+si] ; 00 m/r/m mod=0
    sub     bx,word [bp+di] ; 00 m/r/m mod=0
    sub     bx,word [si]    ; 00 m/r/m mod=0
    sub     bx,word [di]    ; 00 m/r/m mod=0
    sub     bx,word [0x1234]; 00 m/r/m mod=0
    sub     bx,word [bx]    ; 00 m/r/m mod=0

    sub     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bx+si-0x5F]
    sub     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bx+di-0x5F]
    sub     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bp+si-0x5F]
    sub     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bp+di-0x5F]
    sub     bx,word [si+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [si-0x5F]
    sub     bx,word [di+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [di-0x5F]
    sub     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bp-0x5F]
    sub     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bx-0x5F]

    sub     bx,word [bx+si+0x1234]
    sub     bx,word [bx+di+0x1234]
    sub     bx,word [bp+si+0x1234]
    sub     bx,word [bp+di+0x1234]
    sub     bx,word [si+0x1234]
    sub     bx,word [di+0x1234]
    sub     bx,word [bp+0x1234]
    sub     bx,word [bx+0x1234]

;---------------sub imm
    sub     al,0x12
    sub     al,0xEF
    sub     ax,0x1234
    sub     ax,0xFEDC

