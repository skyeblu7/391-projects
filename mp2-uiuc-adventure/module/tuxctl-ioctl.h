// All necessary declarations for the Tux Controller driver must be in this file

#ifndef TUXCTL_H
#define TUXCTL_H

#define TUX_SET_LED _IOR('E', 0x10, unsigned long)
#define TUX_READ_LED _IOW('E', 0x11, unsigned long*)
#define TUX_BUTTONS _IOW('E', 0x12, unsigned long*)
#define TUX_INIT _IO('E', 0x13)
#define TUX_LED_REQUEST _IO('E', 0x14)
#define TUX_LED_ACK _IO('E', 0x15)


// mask to removed unwanted parts of the arguments
#define MASK 0xf
#define NUM_LEDS 4
#define SHIFT_4 4
#define SHIFT_8 8
#define SHIFT_12 12
#define SHIFT_16 16
#define SHIFT_24 24
#define MAX_BYTES 6
#define LSB_MASK 0x1
#define BYTES_6 6
#define CLEAR 0x0
#define BUFF_OFFSET 2
#define NUM_BTNS 8
#define BTN_OFF 0x1

#define ZERO_LED_MAPPING 0xE7
#define ONE_LED_MAPPING 0x06
#define TWO_LED_MAPPING 0xCB
#define THREE_LED_MAPPING 0x8F
#define FOUR_LED_MAPPING 0x2E
#define FIVE_LED_MAPPING 0xAD
#define SIX_LED_MAPPING 0xED
#define SEVEN_LED_MAPPING 0x86
#define EIGHT_LED_MAPPING 0xEF
#define NINE_LED_MAPPING 0xAE
#define A_LED_MAPPING 0xEE
#define B_LED_MAPPING 0x6D
#define C_LED_MAPPING 0xE1
#define D_LED_MAPPING 0x4F
#define E_LED_MAPPING 0xE9
#define F_LED_MAPPING 0xE8

#define led0 2
#define led1 3
#define led2 4
#define led3 5

#define RIGHT 0
#define LEFT 1
#define DOWN 2
#define UP 3
#define C 4
#define B 5
#define A 6
#define START 7

#define SHIFT_3 3
#define SHIFT_2 2
#define SHIFT_1 1

#define SET_LED_DECIMAL 0x10

#endif

