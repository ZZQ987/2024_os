[org 0x7C00]  ; 告诉编译器加载到内存地址0x7C00处

section .text

    ; 一些寄存器初始化            
    xor ax,ax                 
    mov ss,ax                 
    mov es,ax                  
    mov ds,ax  

    ; 设置8253/4定时器芯片
    mov al, 00110110b   ; 必要的设置指令，详解见报告
    out 0x43, al        ; 将指令送入8253/4芯片的命令寄存器

    mov ax,0xE90B       ; 设置为约50ms触发一次
    out 0x40, al        ; 将低8位送入8253/4芯片的计数器2的数据寄存器
    mov al, ah          ; 将高8位送入8253/4芯片的计数器2的数据寄存器
    out 0x40, al

    mov cx,0x0
    mov [es:0x7DA0], cx ;根据MBR空白区域，将7DA0作为记录中断次数的区域

    ; 设置中断向量表
    cli                 ; 关中断
    mov ax, 0           
    mov es, ax          
    mov ax, int_handler 
    mov [es:0x1c*4], ax ; 0x1C处为向量表中定时器有关中断处理例程的位置
    mov ax,0x0          ; 分别将偏移地址(先)和段地址(后)写入对应位置 
    mov [es:0x1c*4+2], ax
    sti                 ; 开中断


    ; 无限循环等待
    jmp $

int_handler:

    mov cx,[es:0x7DA0]
    inc cx              ; 中断次数加一
    cmp cx,0x14         ; 20次中断，约为1s的时间
    je print

    continue:
    mov [es:0x7DA0], cx
    iret

print:

    ; 输出'A'
    mov ah, 0x0E        ; BIOS功能号
    mov al, 'A'         ; 要显示的字符'A'
    mov bh, 0x00        
    int 0x10            ; 调用BIOS中断
    call disable_timer_interrupt

    mov cx,0x0
    jmp continue

disable_timer_interrupt:
    ; 禁止计数器0中断
    mov al, 0b00110000    ; 将控制寄存器设置为停止计数，使用方式3
    out 0x43, al

    ; 将计数器0初值和计数器值设置为0，停止计数
    mov al, 0
    out 0x40, al    ; 写入低8位
    out 0x40, al    ; 写入高8位

    ; 现在需要确保中断控制器（如PIC或APIC）将时钟中断屏蔽
    ; 这可以通过编程相应的控制寄存器来实现
    ; 这取决于你正在使用的硬件和操作系统

    ret

; 填充MBR剩余部分，使其大小为512字节
times 510 - ($-$$) db 0
dw 0xAA55  ; MBR结束标志




