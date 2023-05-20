#ifndef _TERMINAL_H_ 
#define _TERMINAL_H_ 


#include "types.h"


#define READ_TYPED_KEY 0x60
#define MAX_VISIBLE_KEY 57

// initializes the terminal 
void terminal_init();

// reads from kb to a buff 
int32_t terminal_read(int32_t fd, uint8_t* buff, int32_t nbytes);

// writes to the terminal from term buff 
int32_t terminal_write(int32_t fd, uint8_t* buff, int32_t numToWrite);

// opens a given file 
int32_t terminal_open(const uint8_t* filename);

// closes a given file 
int32_t terminal_close(int32_t fd);

/* bad call functions for stdin and stdout */
int32_t bad_call(int32_t ignore);
int32_t bad_read_write(int32_t fd_ignore, uint8_t* buf_ignore, int32_t n_ignore);

// writes given number of characters to terminal 
void terminal_puts(char * buff, int numChars);

// clears the terminal and writes the terminal prompt 
void terminal_clr();

/* fops table for stdin */
fops_t stdin_fops;

/* fops table for stdin */
fops_t stdout_fops;




#endif
