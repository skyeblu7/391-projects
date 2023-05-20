
.text

.globl irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7, irq8
.globl irq9, irq10, irq11, irq12, irq13, irq14, irq15

.globl sys_call, halt_wrapper, execute_wrapper, read_wrapper 
.globl write_wrapper, open_wrapper, close_wrapper

.globl execute_context_switch


.align 4



irq0:
    pushl $0x00000000
    jmp common_interrupt

irq1:
    pushl $0xFFFFFFFF
    jmp common_interrupt

irq2:
    pushl $0xFFFFFFFE
    jmp common_interrupt

irq3:
    pushl $0xFFFFFFFD
    jmp common_interrupt

irq4:
    pushl $0xFFFFFFFC
    jmp common_interrupt

irq5:
    pushl $0xFFFFFFFB
    jmp common_interrupt

irq6:
    pushl $0xFFFFFFFA
    jmp common_interrupt

irq7:
    pushl $0xFFFFFFF9
    jmp common_interrupt

irq8:
    pushl $0xFFFFFFF8
    jmp common_interrupt

irq9:
    pushl $0xFFFFFFF7
    jmp common_interrupt

irq10:
    pushl $0xFFFFFFF6
    jmp common_interrupt

irq11:
    pushl $0xFFFFFFF5
    jmp common_interrupt

irq12:
    pushl $0xFFFFFFF4
    jmp common_interrupt

irq13:
    pushl $0xFFFFFFF3
    jmp common_interrupt

irq14:
    pushl $0xFFFFFFF2
    jmp common_interrupt

irq15:
    pushl $0xFFFFFFF1
    jmp common_interrupt

common_interrupt:
    popl %eax

    push %fs
    push %es
    push %ds
    pushal
    pushfl

    # pushl %eax
    call do_irq
    # popl %eax

    popfl
    popal
    pop %ds
    pop %es
    pop %fs
    
    iret


sys_call:

     # check for syscall > 10 or syscall < 0 (if you treat it as unsigned its > 10)
    cmpl $10, %eax
    ja invalid_syscall

    # check for syscall == 0
    cmpl $0, %eax
    je invalid_syscall

    pushfl
    push %fs
    push %es
    push %ds
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx


    call *sys_call_table(,%eax,4)
    
    
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    pop %ds
    pop %es
    pop %fs
    popfl

    sti
    iret

    invalid_syscall:
    movl $-1, %eax

    sti
    iret



sys_call_table:
	.long 0x0, sysHalt, sysExecute, sysRead, sysWrite, sysOpen, sysClose, sysGetArgs, vidmap
    

