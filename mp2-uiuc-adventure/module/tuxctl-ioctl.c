/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)





// GLOBAL VARIABLES
static unsigned int btn_pkt[NUM_BTNS];
static unsigned int pkt_acknowledged;
static spinlock_t kernel_lock = SPIN_LOCK_UNLOCKED;
static unsigned char curr_leds[MAX_BYTES];



/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	unsigned long flags;
	unsigned char turn_on_btns[1];

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

	switch(a)
	{
	case MTCP_BIOC_EVENT:
		spin_lock_irqsave(&kernel_lock, flags);

		// [right, left, down, up, c, b, a, start]
		btn_pkt[RIGHT] = LSB_MASK & (c >> SHIFT_3);
		btn_pkt[LEFT] = LSB_MASK & (c >> SHIFT_1);
		btn_pkt[DOWN] = LSB_MASK & (c >> SHIFT_2);
		btn_pkt[UP] = LSB_MASK & c;
		btn_pkt[C] = LSB_MASK & (b >> SHIFT_3);
		btn_pkt[B] = LSB_MASK & (b >> SHIFT_2);
		btn_pkt[A] = LSB_MASK & (b >> SHIFT_1);
		btn_pkt[START] = LSB_MASK & b;

		spin_unlock_irqrestore(&kernel_lock, flags);
		break;

	case MTCP_ACK:
		spin_lock_irqsave(&kernel_lock, flags);
		pkt_acknowledged = 1;
		spin_unlock_irqrestore(&kernel_lock, flags);
		break;

	case MTCP_RESET:

		spin_lock_irqsave(&kernel_lock, flags);
		pkt_acknowledged = 1;
		tuxctl_ldisc_put(tty, curr_leds, BYTES_6);
		spin_unlock_irqrestore(&kernel_lock, flags);

		turn_on_btns[0] = MTCP_BIOC_ON;
		tuxctl_ldisc_put(tty, turn_on_btns, 1);
		

		break;
	
	default:
		break;
	}
	



    //printk("packet : %x %x %x\n", a, b, c); 
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{

	// TUX_SET_LED
	unsigned char buff[MAX_BYTES];
	unsigned int led_mapped_values[NUM_LEDS];
	int decimal_points;
	int leds_to_keep_on;
	int i;

	// TUX_BUTTONS
	unsigned int btn_int;
	unsigned char turn_on_btns[1];

	// SYNC
	unsigned long flags;


    switch (cmd) {
	// Initializes all variables needed for other TUX_SET_LED and TUX_BUTTONS
	case TUX_INIT:

		spin_lock_irqsave(&kernel_lock, flags);
		pkt_acknowledged = 1;
		btn_pkt[RIGHT] = BTN_OFF;
		btn_pkt[LEFT] = BTN_OFF;
		btn_pkt[DOWN] = BTN_OFF;
		btn_pkt[UP] = BTN_OFF;
		btn_pkt[C] = BTN_OFF;
		btn_pkt[B] = BTN_OFF;
		btn_pkt[A] = BTN_OFF;
		btn_pkt[START] = BTN_OFF;

		curr_leds[0] = MTCP_LED_SET;
		curr_leds[1] = MASK; // leds to be updated (0xf)
		curr_leds[led0] = ZERO_LED_MAPPING;
		curr_leds[led1] = ZERO_LED_MAPPING;
		curr_leds[led2] = ZERO_LED_MAPPING;
		curr_leds[led3] = CLEAR;
		spin_unlock_irqrestore(&kernel_lock, flags);

		turn_on_btns[0] = MTCP_BIOC_ON;
		tuxctl_ldisc_put(tty, turn_on_btns, 1);

		
		return 0;
		break;

	// Writes the status of the tux buttons into the 8 least significant
	// bits of an integer given by the user and writes this information
	// back to user space
	case TUX_BUTTONS:
		if((unsigned int*)arg == NULL){
			return -EINVAL;
		}

		btn_int = CLEAR;


		// [right, left, down, up, c, b, a, start]
		spin_lock_irqsave(&kernel_lock, flags);

		for(i = 0; i < NUM_BTNS; i++){
			btn_int += 1;
			btn_int -= (~btn_pkt[i]) & LSB_MASK;
			if(i != NUM_BTNS-1){
				btn_int <<= 1;
			}
		}
		spin_unlock_irqrestore(&kernel_lock, flags);

		//printk("%x\n",btn_int);
		

		copy_to_user((unsigned int*)arg, &btn_int, 1);
		

		return 0;
		break;

	// sets the LED displays according to the argument given
	// which is visualized on line 216
	case TUX_SET_LED:

		spin_lock_irqsave(&kernel_lock, flags);	
		if(pkt_acknowledged == 0){
			return 0;
		}
		pkt_acknowledged = 0;
		spin_unlock_irqrestore(&kernel_lock, flags);
		


		// arg = 0000 decimalPoints 0000 LEDsToBeUsed fourthLED thirdLED secondLED firstLED

		// bytes to send to tux controller
		// buff[0] = tux_set_led opcode
		// buff[1] = which led's are used: 0000 0111 or 0000 1111
		// buff[2] = led0 configuration
		// buff[3] = led1 configuration
		// buff[4] = led2 configuration
		// buff[5] = led3 configuration

		decimal_points = ((unsigned int)arg >> SHIFT_24);
		leds_to_keep_on = (arg >> SHIFT_16) & MASK;

		

		buff[0] = MTCP_LED_SET; 
		buff[1] = MASK; // always update all displays



		// led_mapped_values[0] = the number led0 should display
		// led_mapped_values[1] = the number led1 should display
		// led_mapped_values[2] = the number led2 should display
		// led_mapped_values[3] = the number led3 should display
		led_mapped_values[0] = (unsigned int) arg & MASK;
		led_mapped_values[1] = ((unsigned int) arg >> SHIFT_4) & MASK;
		led_mapped_values[2] = ((unsigned int) arg >> SHIFT_8) & MASK;
		led_mapped_values[3] = ((unsigned int) arg >> SHIFT_12) & MASK;





		// led configuration:
		// Mapping from 7-segment to bits
		// The 7-segment display is:
		//  _A
		// F| |B
		//  -G
		// E| |C
		//  -D .dp
		//
		// The map from bits to segments is:
		//
		// __7___6___5___4____3___2___1___0__
		// | A | E | F | dp | G | C | B | D | 
		// +---+---+---+----+---+---+---+---+
		//		
		for(i = 0; i < NUM_LEDS; i++){

			switch(led_mapped_values[i]){
				case 0: // the displayed number should be 0
					buff[i+BUFF_OFFSET] = ZERO_LED_MAPPING; // 0 mapped to LED
					break;
				case 1: // the displayed number should be 1
					buff[i+BUFF_OFFSET] = ONE_LED_MAPPING; // 1 mapped to LED
					break;
				case 2: // the displayed number should be 2
					buff[i+BUFF_OFFSET] = TWO_LED_MAPPING; // 2 mapped to LED
					break;
				case 3: // the displayed number should be 3
					buff[i+BUFF_OFFSET] = THREE_LED_MAPPING; // 3 mapped to LED
					break;
				case 4: // the displayed number should be 4
					buff[i+BUFF_OFFSET] = FOUR_LED_MAPPING; // 4 mapped to LED
					break;
				case 5: // the displayed number should be 5
					buff[i+BUFF_OFFSET] = FIVE_LED_MAPPING; // 5 mapped to LED
					break;
				case 6: // the displayed number should be 6
					buff[i+BUFF_OFFSET] = SIX_LED_MAPPING; // 6 mapped to LED
					break;
				case 7: // the displayed number should be 7
					buff[i+BUFF_OFFSET] = SEVEN_LED_MAPPING; // 7 mapped to LED
					break;
				case 8: // the displayed number should be 8
					buff[i+BUFF_OFFSET] = EIGHT_LED_MAPPING; // 8 mapped to LED
					break;
				case 9: // the displayed number should be 9
					buff[i+BUFF_OFFSET] = NINE_LED_MAPPING; // 9 mapped to LED
					break;
				case 10: // the displayed number should be 10
					buff[i+BUFF_OFFSET] = A_LED_MAPPING; // A mapped to LED
					break;
				case 11: // the displayed number should be 11
					buff[i+BUFF_OFFSET] = B_LED_MAPPING; // b mapped to LED
					break;
				case 12: // the displayed number should be 12
					buff[i+BUFF_OFFSET] = C_LED_MAPPING; // C mapped to LED
					break;
				case 13: // the displayed number should be 13
					buff[i+BUFF_OFFSET] = D_LED_MAPPING; // d mapped to LED
					break;
				case 14: // the displayed number should be 14
					buff[i+BUFF_OFFSET] = E_LED_MAPPING; // E mapped to LED
					break;
				case 15: // the displayed number should be 15
					buff[i+BUFF_OFFSET] = F_LED_MAPPING; // F mapped to LED
					break;

			}


		// buff[2] = led0 configuration
		// buff[3] = led1 configuration
		// buff[4] = led2 configuration
		// buff[5] = led3 configuration

		// Clear all LEDs not being used
		switch (leds_to_keep_on)
		{
		case 0x0: // 0x0 = 0000: means 0 leds are being used
			buff[led0] = CLEAR;
		    buff[led1] = CLEAR;
			buff[led2] = CLEAR;
			buff[led3] = CLEAR;
			break;

		case 0x1: // 0x1 = 0001: means 1 led is being used
			buff[led1] = CLEAR;
			buff[led2] = CLEAR;
			buff[led3] = CLEAR;
			break;

		case 0x2: // 0x2 = 0010: means 1 led is being used
			buff[led0] = CLEAR;
			buff[led2] = CLEAR;
			buff[led3] = CLEAR;
			break;

		case 0x3: // 0x3 = 0011: means 2 leds are being used
			buff[led2] = CLEAR;
			buff[led3] = CLEAR;
			break;
			
		case 0x4: // 0x4 = 0100: means 1 led is being used
			buff[led0] = CLEAR;
		    buff[led1] = CLEAR;
			buff[led3] = CLEAR;
			break;
			
		case 0x5: // 0x5 = 0101: means 2 leds are being used
		    buff[led1] = CLEAR;
			buff[led3] = CLEAR;
			break;
			
		case 0x6: // 0x6 = 0110: means 2 leds are being used
			buff[led0] = CLEAR;
			buff[led3] = CLEAR;
			break;
			
		case 0x7: // 0x7 = 0111: means 3 leds are being used
			buff[led3] = CLEAR;
			break;
			
		case 0x8: // 0x8 = 1000: means 1 led is being used
			buff[led0] = CLEAR;
			buff[led1] = CLEAR;
			buff[led2] = CLEAR;
			break;
			
		case 0x9: // 0x9 = 1001: means 2 leds are being used
		    buff[led1] = CLEAR;
			buff[led2] = CLEAR;
			break;
			
		case 0xa: // 0xa = 1010: means 2 leds are being used
			buff[led0] = CLEAR;
			buff[led2] = CLEAR;
			break;
			
		case 0xb: // 0xb = 1011: means 3 leds are being used
			buff[led2] = CLEAR;
			break;
			
		case 0xc: // 0xc = 1100: means 2 leds are being used
			buff[led0] = CLEAR;
		    buff[led1] = CLEAR;
			break;
			
		case 0xd: // 0xd = 1101: means 3 leds are being used
		    buff[led1] = CLEAR;
			break;
			
		case 0xe: // 0xe = 1110: means 3 leds are being used
			buff[led0] = CLEAR;
			break;
			
		case 0xf: // 0xf = 1111: means all leds are being used
			break; // none of the led's need to be cleared
			
		}

			// if the value of the decimal_points variable corresponding to the 
			// current led is 1, then we must make sure the decimal point is on
			if(((decimal_points >> i) & LSB_MASK) == 1){ 
				buff[i+BUFF_OFFSET] += SET_LED_DECIMAL; // Sets the bit that turns on the decimal
			}

		}

		for(i = 0; i < MAX_BYTES; i++){
			spin_lock_irqsave(&kernel_lock, flags);
			curr_leds[i] = buff[i];
			spin_unlock_irqrestore(&kernel_lock, flags);
		}

		tuxctl_ldisc_put(tty, buff, BYTES_6); // write to tux controller
		return 0;

	case TUX_LED_ACK:
		return 0;


	case TUX_LED_REQUEST:
		return 0;


	case TUX_READ_LED:
		return 0;


	default:
	    return -EINVAL;
    }
}

