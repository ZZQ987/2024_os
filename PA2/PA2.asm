BITS 16


%define SIZE 512             ; MBR的大小
%define BASE 0x7C00          ; BIOS初始将MBR加载的位置
%define DEST 0x0600          ; MBR应该复制转移的位置

SECTION MBR vstart=0x7c00

cli               

;********************************************************************;
;*                        寄存器和栈指针初始化                        *;
;********************************************************************;

mov sp, BASE                
xor ax,ax                 
mov ss,ax                 
mov es,ax                  
mov ds,ax                                      

;********************************************************************;
;*                           复制MBR并跳转                           *;
;********************************************************************;

; 为了配合win的VBR(stage2)启动,需要将MBR先进行复制转移
mov si,BASE                  ; 源
mov di, DEST                 ; 目的地
mov cx, SIZE                 ; 大小(循环次数)
cld                          ; 设置方向(forward)
rep MOVSB                    ; 复制
jmp SKIP + DEST              ; 跳转

SKIP: EQU ($ - $$)           

;********************************************************************;
;*                             设置显示                              *;
;********************************************************************;

sti                          ; Enable interrupts

; 清屏 利用 0x06号功能，上卷全部行，则可清屏幕   
mov ah,0x06	
;清屏
mov al,0
;设置滚动(清屏)范围
mov cx,0
mov dx,0xffff  
int 0x10

; 设置字体颜色
; 打印系统stage2选项
; 设置颜色，0xE为黄色，0x2为绿色，0x4为红色
mov bl,0xE
call print0
; 设置为绿色
mov bl,0x2
call print1
call print2
; 进入接受键盘输入阶段
jmp input


;********************************************************************;
;*                             输入选择                              *;
;********************************************************************;

input:

    ; 初始阶段，还未选择
    ; 如果有键盘输入"1"或"2"则跳转到下一阶段
    ; 否则一直在此阶段
    mov ah,0
    int 16h
    cmp al,'1'
    je case1
    cmp al,'2'
    je case2
    jmp input

    case1:
        
        ; 根据系统选择，进行颜色变化
        mov bl,0x4
        call print1
        mov bl,0x2
        call print2

        ; 再次检测输入
        mov ah,0
        int 16h

        ; 如果输入回车，则跳转对于选择的stage2
        ; 如果输入"2"，则进入另一case2
        ; 否则一直在此阶段
        cmp al,0x0d
        je win_stage2
        cmp al,'2'
        je case2
        jne case1

    ret

    case2:
        
        ; 同上

        mov bl,0x2
        call print1
        mov bl,0x4
        call print2

        mov ah,0
        int 16h

        cmp al,0x0d
        je linux_stage2
        cmp al,'1'
        je case1
        jne case2

    ret


ret


;********************************************************************;
;*                             stage2引导                            *;
;********************************************************************;


win_stage2:

    ; 结合分区表信息,bp现在指向第一个分区开头
    mov bp,0x07BE
    ; 设置DAP(磁盘地址包)
    ; 通过压栈的方式设置
    push dword 0x0
    push dword [bp+0x8]
    push word 0x0
    push word 0x7c00
    push word 0x1
    push word 0x10
    ; 通过int 0x13读取磁盘
    mov ah,0x42
    mov dl,[bp]
    mov si,sp
    int 0x13

    ; 清栈
    add sp,+10

    ; 跳转到stage2
    jmp  0x0:0x7c00


ret

linux_stage2:

    ; 设置DAP位置
    mov ax,0x0000
    mov ds,ax
    mov si,0x07DA8
    
    ; 设置DAP详细信息
    xor ax,ax
    mov [si+0x4],ax
    inc ax
    mov [si-0x1],al
    mov [si+0x2],ax
    mov word [si],0x10
    mov ebx,0x00000001
    mov [si+0x8],ebx
    mov ebx,0x00000000
    mov [si+0xc],ebx
    mov word [si+0x6],0x7000

    ; 使用INT 0x13读取硬盘
    mov dl,0x80
    mov ah,0x42
    int 13h

    ; 根据Linux的MBR，还需要将这一段复制转移
    mov bx,0x7000
    pusha
    push ds
    mov cx,0x100
    mov ds,bx
    xor si,si
    mov di,0x8000
    mov es,si
    cld
    rep movsw

    pop ds
    popa

    ; 跳转到stage2
    jmp 0x00:0x8000



print1:
		
    ;光标位置初始化
    mov ah,0x02	
    ;设置页码和行列位置
    mov bh,0
    mov dh,5
    mov dl,8
    int 0x10


    ; ; 获取光标位置,INT 10h,ah=3h

    ; mov ah,3                ; 输入 3 子功能是获取光标位置，需要存入 ah 寄存器
    ; mov bh,0                ; bh 寄存器是待获取的光标的页号
    ; int 0x10 
               
    ; 打印字符串,INT 10h,ah=13h

    mov ax,sys1
    mov bp,ax  		        ; es：bp 为串首地址，es 此时同 cs 一致
    mov cx,10               ; cx表示字符串长度
    mov ah,0x13 		    ; 子功能号：13h 是显示字符及属性，要存入 ah 寄存器
    mov al,0x01             ; al 设置写字符方式 al=01 ： 显示字符串，光标随着移动
    mov bh,0x0 	            ; bh 存储要显示的页号，此处是第 0 页


    int 0x10
ret

print2:

    ; 详细注释同上
			
    ;光标位置初始化
    mov ah,0x02	
    ;设置页码和行列位置
    mov bh,0
    mov dh,6
    mov dl,8
    int 0x10
        
    ; 打印字符串,INT 10h ah=13h
    mov ax,sys2
    mov bp,ax  		
    mov cx,5         
    mov ah,0x13 	
    mov al,0x01         
    mov bh,0x0 	    

    int 0x10
ret

print0:

    ; 详细注释同上
			
    ;光标位置初始化
    mov ah,0x02	
    ;设置页码和行列位置
    mov bh,0
    mov dh,3
    mov dl,8
    int 0x10
         
    ; 打印字符串,INT 10h ah=13h
    mov ax,tip1
    mov bp,ax  		
    mov cx,67       
    mov ah,0x13 	
    mov al,0x01         
    mov bh,0x0 	    

    int 0x10

ret


tip1 db "Please use 1 and 2 to select the system and press enter to confirm."
sys1 db "Windows 10"
sys2 db "Linux"

; 用0填充到440字节
times 440 -($ -$$) db 0


