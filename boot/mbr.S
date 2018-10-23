;主引导程序
%include "boot.inc"

SECTION MBR vstart=MBR_BASE_ADDR
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7c00
    mov ax, 0xb800 ;显存文本模式地址在0xb8000，因为是实模式，所以需要除以16得到基址
    mov gs, ax     

    mov ax, 0x600
    mov bx, 0x799
    mov cx, 0
    mov dx, 0x184f

    int 0x10
    
    mov byte [gs:0x00], 'M'
    mov byte [gs:0x01], 0xA4

    mov byte [gs:0x02], 'B'
    mov byte [gs:0x03], 0xA4

    mov byte [gs:0x04], 'R'
    mov byte [gs:0x05], 0xA4

    mov byte [gs:0x06], 'l'
    mov byte [gs:0x07], 0xA4

    mov byte [gs:0x08], 'o'
    mov byte [gs:0x09], 0xA4

    mov byte [gs:0x0a], ','
    mov byte [gs:0x0b], 0xA4
    
    ;mov byte [gs:0x0c], 'w'
    ;mov byte [gs:0x0d], 0xA4    
    
    ;mov byte [gs:0x0e], 'o'
    ;mov byte [gs:0x0f], 0xA4

    ;mov byte [gs:0x10], 'r'
    ;mov byte [gs:0x11], 0xA4

    ;mov byte [gs:0x12], 'l'
    ;mov byte [gs:0x13], 0xA4

    ;mov byte [gs:0x14], 'd'
    ;mov byte [gs:0x15], 0xA4

    mov eax, LOADER_START_SECTOR ;起始扇区lbd地址
    mov bx, LOADER_BASE_ADDR     ;写入的内存地址
    mov cx, 1                    ;读入的扇区数
    call rd_disk_m_16

    jmp LOADER_BASE_ADDR         ;跳转到加载器


;硬盘的n个扇区
; eax=LBA
; bx=待写入的地址
; cx=扇区数
rd_disk_m_16:
        ;1.设置要读取的扇区数
        mov esi, eax
        mov di, cx

        mov dx, 0x1f2
        mov al, cl
        out dx, al

        mov eax, esi

        ;2.设置LBA地址
        mov dx, 0x1f3
        out dx, al 

        mov cl, 8
        shr eax, cl
        mov dx, 0x1f4
        out dx, al

        shr eax, cl
        mov dx, 0x1f5
        out dx, al

        shr eax, cl
        and al, 0x0f
        or al, 0xe0
        mov dx, 0x1f6
        out dx, al

        ;3.写入读命令0x20
        mov dx, 0x1f7
        mov al, 0x20
        out dx, al

        ;4.检查硬盘状态
    .not_ready:
        nop
        in al, dx
        and al, 0x88
        cmp al, 0x08
        jnz .not_ready ;如果硬盘没有准备好，就一直等待

    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax

    mov dx, 0x1f0

    .go_on_read:
        in ax, dx
        mov [bx], ax ;写入到内存
        add bx, 2
        loop .go_on_read
        ret

    times 510-($-$$) db 0 ;设置魔数
    db 0x55,0xaa