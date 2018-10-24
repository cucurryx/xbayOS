;实现基本的打印函数

; 1.保护寄存器现场
; 2.获取光标坐标
; 3.获取打印字符
; 4.判断是否为控制字符（换行、回车、退格）
; 5.判断是否滚屏
; 6.更新光标
; 7.恢复现场

%include "lib.inc"

;标示32位保护模式
[bits 32]
section .text

global put_str
put_str:
  push ebx
  push ecx
  xor ecx, ecx
  mov ebx, [esp + 12]   ;读入待打印字符串
  
.continue:
  mov cl, [ebx]
  cmp cl, 0 
  jz .str_end
  push ecx              ;给put_char函数的参数
  call put_char
  add esp, 4
  inc ebx
  jmp .continue
.str_end:
  pop ecx
  pop ebx
  ret

global put_char
put_char:
  pushad

  mov ax, SELECTOR_VIDEO
  mov gs, ax

  ;获取当前光标位置，先高8位，再低8位
  mov dx, 0x03d4
  mov al, 0x0e
  out dx, al
  mov dx, 0x03d5
  in al, dx
  mov ah, al 

  mov dx, 0x03d4
  mov al, 0x0f
  out dx, al
  mov dx, 0x03d5
  in al, dx

  mov bx, ax
  mov ecx, [esp + 36] ;从栈中取出待打印的字符

  ;处理特殊字符
  ;CR 回车 0xd
  ;LF 换行 0xa
  ;BS 退格 0x8
  cmp cl, 0xd
  jz .is_cr
  cmp cl, 0xa
  jz .is_lf
  cmp cl, 0x8
  jz .is_bs
  jmp .is_others

.is_cr:
.is_lf:
  xor dx, dx
  mov ax, bx
  mov si, 80
  div si
  sub bx, dx
.is_cr_end:
  add bx, 80
  cmp bx, 2000
.is_lf_end:
  jl .set_cursor

.is_bs:
  dec bx
  shl bx, 1

  mov byte [gs:bx], 0x20
  inc bx
  mov byte [gs:bx], 0x07
  shr bx, 1
  jmp .set_cursor

.is_others:
  shl bx, 1
  mov [gs:bx], cl
  inc bx
  mov byte [gs:bx], 0x07
  shr bx, 1
  inc bx
  cmp bx, 2000
  jl .set_cursor 
  jmp .is_cr        ;如果超出限制，就换行

;滚屏操作，不更改屏幕开始寄存器，使用固定窗口
.roll_screen:
  cld
  mov ecx, 960

  mov esi, 0xc00b80a0
  mov edi, 0xc00b8000
  rep movsd

  ;最后一行填充空白
  mov ebx, 3840
  mov ecx, 80 

.cls:
  mov word [gs:ebx], 0x0720
  add ebx, 2 
  loop .cls
  mov bx, 1920

.set_cursor:
  ;分别设置高低8位 
  mov dx, 0x03d4
  mov al, 0x0e
  out dx, al
  mov dx, 0x03d5
  mov al, bh
  out dx, al

  mov dx, 0x03d4
  mov al, 0x0f
  out dx, al
  mov dx, 0x03d5
  mov al, bl
  out dx, al

.put_char_done:
  popad
  ret


