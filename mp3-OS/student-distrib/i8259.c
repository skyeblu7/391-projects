/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {


    // save masks
    master_mask = 0xFF; // disable all IRQs for now
    slave_mask = 0xFF; // disable all IRQs for now


    //master_mask = inb(MASTER_8259_DATA); // save masks
    //slave_mask = inb(SLAVE_8259_DATA);


    outb(ICW1, MASTER_8259_PORT); // starts init seq for master
    outb(ICW2_MASTER, MASTER_8259_DATA); // master PIC vector offset
    outb(ICW3_MASTER, MASTER_8259_DATA); // tell master PIC there is a slave PIC at 0x4
    outb(ICW4, MASTER_8259_DATA); // gives additional info about the environment


    outb(ICW1, SLAVE_8259_PORT); // starts init seq for slave
    outb(ICW2_SLAVE, SLAVE_8259_DATA); // slave PIC vector offset
    outb(ICW3_SLAVE, SLAVE_8259_DATA);   // tell slave PIC its cascade identity is 0x2
    outb(ICW4, SLAVE_8259_DATA);

    outb(master_mask, MASTER_8259_DATA);    // set masks
    outb(slave_mask, SLAVE_8259_DATA);

    

    enable_irq(2);


}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t val;



    if(irq_num < MIDDLE_NUM){
        port = MASTER_8259_DATA;
        master_mask = master_mask & ~(1 << irq_num); 
		val = master_mask; 
    }
    else{
        port = SLAVE_8259_DATA;
        irq_num -= MIDDLE_NUM;

        master_mask = master_mask & 0xFB;  
		slave_mask = slave_mask & ~(1 << irq_num);
        val = slave_mask;

        outb(master_mask, MASTER_8259_DATA); 
    }

    outb(val, port);

}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {

    if(irq_num < MIDDLE_NUM){
        master_mask = master_mask | (1 << irq_num);
        outb(master_mask, MASTER_8259_DATA);
    }
    else{
        irq_num -= MIDDLE_NUM;
        slave_mask = slave_mask | (1 << irq_num);
        outb(slave_mask, SLAVE_8259_DATA);
    }

}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {


    if(irq_num >= MIDDLE_NUM){ // is it a slave IRQ?
        outb(EOI | (irq_num - MIDDLE_NUM), SLAVE_8259_PORT); // send EOI signal to slave with the slave's interrupt IRQ number
        outb((EOI | 2), MASTER_8259_PORT); // tell master a slave interrupt has been handled
    }
    else{
        outb((EOI | irq_num), MASTER_8259_PORT); // send EOI signal to master with master's irq number
    }

    
}
