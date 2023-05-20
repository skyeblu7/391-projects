#include "i8259.h"
#include "pit.h"
#include "lib.h"
#include "systemcall.h"
#include "scheduler.h"

/* pit_init()
 * Description: sets the configeration of the PIT to run 30 times per sec
 * INPUTS:      None
 * OUTPUTS:     None
 * RETURN VALUE: none
 */
void pit_init(void){

    int divisor = PIT_INPUT_CLK / PIT_HZ;       /* Calculate our divisor */
    outb(PIT_MODE, COMMAND_REG);             /* Set our command byte 0x36, specifies how the PIT's command port behaves */
    outb(divisor & PIT_MASK, CHANNEL_0);   /* Set low byte of divisor */
    outb(divisor >> PIT_SHIFT, CHANNEL_0);     /* Set high byte of divisor */


    enable_irq(0);

}


/* 
 * handle_pit
 *   DESCRIPTION: handles pit interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void handle_pit(){
    cli();

    //scheduler();

    send_eoi(0);

}












