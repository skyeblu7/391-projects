
#include "idt.h"
#include "pit.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "rtc.h"
#include "kb.h"
#include "systemcall.h"


/* idt_setup()
 * Description: Populates the IDT with idt_desc_t descriptors (or rather,
 *              sets the values of the descriptors in the idt to non-provisonal
 *              values)
 * INPUTS:      None
 * OUTPUTS:     None
 * EFFECTS:     Changes values in the idt[] array
 */
void idt_setup(){
    int i;
    /* iterate over the IDT and change values accordingly */
    for(i = 0; i < NUM_VEC; i++){
        /* set up the first 32 vectors as TRAPS (type = 0b0111)
         * and set their selector to KERNEL_CS
         */
        if(i < 20 && i != 15){
            idt[i].present = 1;
            idt[i].reserved4 = 0x00;

            idt[i].reserved0 = 0;
            idt[i].reserved1 = 1;
            idt[i].reserved2 = 1;
            idt[i].reserved3 = 1;
            
            idt[i].size = 1;
            idt[i].dpl = 0;

            idt[i].seg_selector = KERNEL_CS;
        }

        // for the hardware interrupts, set them up as such
        // specifically, 
        if(i >= NUM_TRAP && i < NUM_TRAP + 16){
            idt[i].present = 0;
            idt[i].reserved4 = 0x00;

            idt[i].reserved0 = 0;
            idt[i].reserved1 = 1;
            idt[i].reserved2 = 1;
            idt[i].reserved3 = 0;

            idt[i].size = 1;
            idt[i].dpl = 0;

            idt[i].seg_selector = KERNEL_CS;
        }
        // set up the INT vector for system calls 
        if(i == 128){
            idt[i].present = 1;
            idt[i].reserved4 = 0x00;

            idt[i].reserved0 = 0;
            idt[i].reserved1 = 1;
            idt[i].reserved2 = 1;
            idt[i].reserved3 = 0;
            
            idt[i].size = 1;
            idt[i].dpl = 3;

            idt[i].seg_selector = KERNEL_CS;
        }
    }

    idt[IRQ0].present = 1;
    idt[IRQ1].present = 1;
    idt[IRQ8].present = 1;

    /* now manually associate IDT vectors with their handlers */
    SET_IDT_ENTRY(idt[0], intr0);
    SET_IDT_ENTRY(idt[1], intr1);
    SET_IDT_ENTRY(idt[2], intr2);
    SET_IDT_ENTRY(idt[3], intr3);
    SET_IDT_ENTRY(idt[4], intr4);
    SET_IDT_ENTRY(idt[5], intr5);
    SET_IDT_ENTRY(idt[6], intr6);
    SET_IDT_ENTRY(idt[7], intr7);
    SET_IDT_ENTRY(idt[8], intr8);
    SET_IDT_ENTRY(idt[9], intr9);
    SET_IDT_ENTRY(idt[10], intr10);
    SET_IDT_ENTRY(idt[11], intr11);
    SET_IDT_ENTRY(idt[12], intr12);
    SET_IDT_ENTRY(idt[13], intr13);
    SET_IDT_ENTRY(idt[14], intr14);
    SET_IDT_ENTRY(idt[16], intr16);
    SET_IDT_ENTRY(idt[17], intr17);
    SET_IDT_ENTRY(idt[18], intr18);
    SET_IDT_ENTRY(idt[19], intr19);
    SET_IDT_ENTRY(idt[128], sys_call);

    SET_IDT_ENTRY(idt[NUM_TRAP + 0], irq0);
    SET_IDT_ENTRY(idt[NUM_TRAP + 1], irq1);
    SET_IDT_ENTRY(idt[NUM_TRAP + 2], irq2);
    SET_IDT_ENTRY(idt[NUM_TRAP + 3], irq3);
    SET_IDT_ENTRY(idt[NUM_TRAP + 4], irq4);
    SET_IDT_ENTRY(idt[NUM_TRAP + 5], irq5);
    SET_IDT_ENTRY(idt[NUM_TRAP + 6], irq6);
    SET_IDT_ENTRY(idt[NUM_TRAP + 7], irq7);
    SET_IDT_ENTRY(idt[NUM_TRAP + 8], irq8);
    SET_IDT_ENTRY(idt[NUM_TRAP + 9], irq9);
    SET_IDT_ENTRY(idt[NUM_TRAP + 10], irq10);
    SET_IDT_ENTRY(idt[NUM_TRAP + 11], irq11);
    SET_IDT_ENTRY(idt[NUM_TRAP + 12], irq12);
    SET_IDT_ENTRY(idt[NUM_TRAP + 13], irq13);
    SET_IDT_ENTRY(idt[NUM_TRAP + 14], irq14);
    SET_IDT_ENTRY(idt[NUM_TRAP + 15], irq15);


    lidt(idt_desc_ptr);

    return;
}

/* idtn()
 * these are the function handlers for the 
 * first 19 vectors in the IDT. They share 
 * this function header. At present, they 
 * simply print a statement about the error
 * that they correspond to, and catch in a 
 * while loop. Currently the system call handler
 * is also in this group of functions, but
 * may be moved later as needed.
 */
void intr0(){
    cli();
    printf("Divide Error Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr1(){
    cli();
    printf("Debug Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr2(){
    cli();
    printf("NMI Interrupt\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr3(){
    cli();
    printf("Breakpoint Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr4(){
    cli();
    printf("Overflow Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr5(){
    cli();
    printf("BOUND Range Exceeded Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr6(){
    cli();
    printf("Invalid Opcode Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr7(){
    cli();
    printf("Device Not Available Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr8(){
    cli();
    printf("Double Fault Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr9(){
    cli();
    printf("Compressor Segment Overrun\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr10(){
    cli();
    printf("Invalid TSS Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr11(){
    cli();
    printf("Segment Not Present\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr12(){
    cli();
    printf("Stack Fault Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr13(){
    cli();
    printf("General Protection Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr14(){
    cli();
    printf("Page Fault Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
/* Interrupt vector 15 is omitted as it is reserved by Intel */
void intr16(){
    cli();
    printf("x86 FPU Floating-Point Error\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr17(){
    cli();
    printf("Alignment Check Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr18(){
    cli();
    printf("Machine-Check Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}
void intr19(){
    cli();
    printf("SIMD Floating-Point Exception\n");
    sysHalt((uint8_t)EXCEPTION_HALT_STATUS);
    return;
}


/* do_irq
 * takes an irq number, finds it's descriptor in in the irq_desc table
 * and calls the handler function associated with that descriptor
 */
void do_irq(){

    register int irq asm("eax");
    int real_irq = ~(irq - 1);

    if(real_irq == 0){
        handle_pit();
    }
    if(real_irq == 1){
        handle_keyboard();
    }
    if(real_irq == RTC_IRQ){
        handle_rtc();
    }

}




