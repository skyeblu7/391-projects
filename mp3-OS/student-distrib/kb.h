#ifndef _KB_H_ 
#define _KB_H_ 

#include "types.h"

// special keys
#define L_SHIFT_PRESSED 0x2A
#define R_SHIFT_PRESSED 0x36
#define CTRL_PRESSED 0x1D
#define ALT_PRESSED 0x38
#define CAPS_LOCK_PRESSED 0x3A

#define L_SHIFT_RELEASED 0xAA
#define R_SHIFT_RELEASED 0xB6 
#define CTRL_RELEASED 0x9D
#define ALT_RELEASED 0xB8
#define CAPS_LOCK_RELEASED 0xBA

#define TAB_PRESSED 0x0F
#define ESC_PRESSED 0x01
#define BACKSPACE_PRESSED 0x0E
#define ENTER_PRESSED 0x1C
#define PRT_SC_PRESSED 0x37

#define TAB_RELEASED 0x8F
#define ESC_RELEASED 0x81
#define BACKSPACE_RELEASED 0x8E
#define ENTER_RELEASED 0x9C
#define PRT_SC_RELEASED 0xB7

#define F1_PRESSED 0x3B
#define F2_PRESSED 0x3C
#define F3_PRESSED 0x3D

#define F1_RELEASED 0xBB
#define F2_RELEASED 0xBC
#define F3_RELEASED 0xBD


#define KEY_MAP_SIZE 62




/* keyboard initialization */
void kb_init(void);


/* Keyboard Interrupt Handler */
void handle_keyboard(void);

/* returns the keyboard buffer */
//char* get_kb_buff();

/* returns the size of the keyboard buffer */
//int get_buff_size();

/* returns 1 if buff is ready, 0 if not */
//int get_is_buff_ready();

/* if is_buff_ready is 0, sets it to 1 */
/* if is_buff_ready is 1, sets it to 0 */
//void toggle_is_buff_ready();


#endif
