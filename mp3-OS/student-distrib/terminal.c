
#include "terminal.h"
#include "kb.h"
#include "lib.h"
#include "types.h"
#include "scheduler.h"



extern term_t terminals[3];
extern int currentTerminal;


/* 
 * terminal_init
 *   DESCRIPTION: initializes terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void terminal_init(){
    terminal_clr();


}

/* 
 * terminal_read
 *   DESCRIPTION: reads from buffer
 *   INPUTS: fd - not used
 *           buf - to be read from
 *           nbytes - not used
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 */
int32_t terminal_read(int32_t fd, uint8_t* buff, int32_t nbytes){
    int i;

    sti();
    while(terminals[currentTerminal].enter_pressed != 1){}

    for(i = 0; i < terminals[currentTerminal].typing_buf_idx+1; i++){
        if(i > nbytes-1){
            break;
        }
        ((char *)buff)[i] = terminals[currentTerminal].typing_buf[i];
    }

    terminals[currentTerminal].enter_pressed = 0;
    
    return i;
}

/* 
 * terminal_write
 *   DESCRIPTION: writes to buffer
 *   INPUTS: fd - not used
 *           buf - buffer to write to
 *           nbytes - not used
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 */
int32_t terminal_write(int32_t fd, uint8_t* buff, int32_t numToWrite){ 

    terminal_puts((char*)buff, numToWrite);

    int i;
    for(i = 0; i < CHAR_LIM; i++){
        ((char*)terminals[currentTerminal].typing_buf)[i] = '\0';
    }

    terminals[currentTerminal].typing_buf_idx = 0;

    return numToWrite;
    
}

/* 
 * terminal_open
 *   DESCRIPTION: does nothing
 *   INPUTS: filename - not used
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 */
int32_t terminal_open(const uint8_t* filename){
    return 0;
}

/* 
 * terminal_close
 *   DESCRIPTION: does nothing
 *   INPUTS: fd - not used
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 */
int32_t terminal_close(int32_t fd){
    return 0;
}

/* 
 * terminal_puts
 *   DESCRIPTION: prints num of chars stored to the terminal sequentially 
 *   INPUTS: buff - to be read from
 *           numChars - not used
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void terminal_puts(char * buff, int numChars){
    register int32_t index = numChars;
    int i;
    for(i = 0; i < index; i++){
        if(buff[i] == '\t'){
            puts("   ");
        }
        else{
            putc(buff[i]);
        }
    }
}

/* 
 * terminal_read
 *   DESCRIPTION: clears terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void terminal_clr(){

    clear();
    //puts("DrEaM TeAm> ");
    //terminal_puts(term_buff, term_buf_idx+1);


    //if(terminal_on == 1){
    //    puts("DrEaM TeAm> ");
    //}


}

/* bad_call
 * DESCRIPTION: returns -1. This is the fops mapping for 
 *              if you try to open or close stdin or stdout
 * INPUTS:      ignore - is ignored
 * RETURNS:     -1
 */
int32_t bad_call(int32_t ignore){
    return -1;
}

/* bad_read_write
 * DESCRIPTION: like bad_call, but its function signature
 *              matches the fops->read or fops->write 
 *              signature.
 * INPUTS:      fd_ignore - is ignored
 *              buf_ignore - is ignored
 *              n_ignore - is ignored
 * OUTPUTS:     -1
 */
int32_t bad_read_write(int32_t fd_ignore, uint8_t* buf_ignore, int32_t n_ignore){
    return -1;
}

