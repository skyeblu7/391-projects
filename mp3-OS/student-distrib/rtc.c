#include "i8259.h"
#include "rtc.h"
#include "lib.h"
#include "systemcall.h"


volatile int rtc_interrupt_occurred = 0;
int interrupt_count = MAX_FREQ / 2;    // sets interrupt count for frequency of 2 Hz

/* rtc_init()
 * Description: Sets rate to maximum frequency and interrupt_count so that the frequency is treated as 2 Hz
 * INPUTS:      None
 * OUTPUTS:     None
 * RETURN VALUE: none
 */
void rtc_init(void){
    // disable interrupts

    outb(REGISTER_B, RTC_INDEX);                // select register B, and disable NMI
    unsigned char prev = inb(RTC_CMOS);  // read the current value of register B
    outb(REGISTER_B, RTC_INDEX);                // set the index again (a read will reset the index to register D)
    outb((prev | 0x40), RTC_CMOS);       // write the previous value ORed with 0x40. This turns on bit 6 of register B
    
    interrupt_count = MAX_FREQ / 2;    // set frequency to 2 Hz

    uint8_t rate = DEFAULT_RATE;	// set the rate to 6 for 1024 Hz frequency, which is the default
    outb(REGISTER_A, RTC_INDEX);		        // set index to register A, disable NMI
    prev=inb(RTC_CMOS);	                // get initial value of register A
    outb(REGISTER_A, RTC_INDEX);		        // reset index to A
    outb((prev & 0xF0) | rate, RTC_CMOS); //write only our rate to A. rate is the bottom 4 bits, 0xF0 clears them

    enable_irq(8);

}

/* 
 * handle_rtc
 *   DESCRIPTION: handles rtc interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void handle_rtc(){

    //test_interrupts();

    cli();

    send_eoi(8);
    
    outb(REGISTER_C, RTC_INDEX); // select register C
    inb(RTC_CMOS); // just throw away contents

    rtc_interrupt_occurred = 1;    //set interrupt occurred flag

    sti();

}

/* 
 * rtc_open
 *   DESCRIPTION: initializes RTC frequency to 2Hz by calling rtc_init 
 *   INPUTS: fd - the fd index to the rtc
 *   OUTPUTS: fd
 *   RETURN VALUE: 0
 */
int32_t rtc_open(int32_t fd) {
    curr_pcb->fda[fd].flags = 1;
    curr_pcb->fda[fd].db_num = 0;
    curr_pcb->fda[fd].byte_num = 0;
    curr_pcb->fda[fd].inode = 0;
    rtc_init();
    return fd;
}

/* 
 * rtc_read
 *   DESCRIPTION: blocks until next interrupt, then returns 
 *   INPUTS: fd - not used
 *           buf - not used
 *           nbytes - not used
 *   OUTPUTS: none
 *   RETURN VALUE: 0 once enough interrupts recieved for current frequency
 */
int32_t rtc_read(int32_t fd, uint8_t* buf, int32_t nbytes) {
    // set flag to 0 since we are waiting for the next interrupt
    sti();
    rtc_interrupt_occurred = 0;
    int count = 0;

    // loop until we have recieved the correct number of interrupts
    while (count < interrupt_count) {
        // if an interrupt occurred, increment count and set flag back to 0
        if (rtc_interrupt_occurred) {
            count++;
            rtc_interrupt_occurred = 0;
        }
    }

    // once enough interrupts have been recieved, return
    return 0;
}

/* 
 * rtc_write
 *   DESCRIPTION: sets frequency by modifying interrupt_count 
 *   INPUTS: fd - not used
 *           buf - contains frequency to set
 *           nbytes - not used
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 */
int32_t rtc_write(int32_t fd, uint8_t* buf, int32_t nbytes) {
    // null check
    cli();
    if (buf == NULL) {
        return -1;
    }

    int32_t freq = *(int32_t*)buf;

    // check if rate is a power of 2 and less than the maxiumum frequency allowed
    if ((freq != 0) && ((freq & (freq - 1)) == 0) && (freq <= MAX_FREQ)) {
        interrupt_count = MAX_FREQ / freq;

        return 0;
    }
    sti();
    return -1;
}

/* 
 * rtc_close
 *   DESCRIPTION: sets frequency by modifying interrupt_count 
 *   INPUTS: fd - not used
 *           buf - contains frequency to set
 *           nbytes - not used
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 */
int32_t rtc_close(int32_t fd) {
    curr_pcb->fda[fd].flags = 0;
    curr_pcb->fda[fd].fops = NULL;
    curr_pcb->fda[fd].db_num = 0;
    curr_pcb->fda[fd].byte_num = 0;
    curr_pcb->fda[fd].inode = 0;
    return 0;
}




