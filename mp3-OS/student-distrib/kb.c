
#include "kb.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "systemcall.h"
#include "scheduler.h"




// keyboard mapping lower case
char kb_map_l[KEY_MAP_SIZE] = {
                    '_','_','1','2','3','4','5','6','7','8','9','0',
                    '-','=','_','\t','q','w','e','r','t','y','u','i',
                    'o','p','[',']','\n','_','a','s','d','f','g','h', 
                    'j', 'k', 'l', ';', '\'', '`', '_', '\\', 'z', 'x', 
                    'c','v', 'b', 'n', 'm', ',', '.', '/', '_', '_', '_', 
                    ' ', '_', '_', '_', '_'};

// keyboard mapping shift key map
char kb_map_sh[KEY_MAP_SIZE] = {
                    '-','-','!','@','#','$','%','^','&','*','(',')','_',
                    '+','-','-','Q','W','E','R','T','Y','U','I','O','P',
                    '{','}','\n','-','A','S','D','F','G','H', 'J', 'K', 
                    'L', ':', '"', '~', '-', '|', 'Z', 'X', 'C','V', 'B', 
                    'N', 'M', '<', '>', '?', '-', '-', '-', ' ', '-', '-', '-', '-'};

// keyboard mapping caps lock
char kb_map_caps[KEY_MAP_SIZE] = {
                    '_','_','1','2','3','4','5','6','7','8','9','0','-','=',
                    '_','\t','Q','W','E','R','T','Y','U','I','O','P','[',']',
                    '\n','_','A','S','D','F','G','H', 'J', 'K', 'L', ';', '\'', 
                    '`', '_', '\\', 'Z', 'X', 'C','V', 'B', 'N', 'M', ',', '.', 
                    '/', '_', '_', '_', ' ', '_', '_', '_', '_'};

// keyboard pressed state
int kb_map_state[KEY_MAP_SIZE] = {
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                    0, 0, 0, 0, 0, 0, 0, 0, 0};



extern term_t terminals[3];
extern int currentTerminal;

int caps_released;
int caps_locked;

/* 
 * kb_init
 *   DESCRIPTION: initializes keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void kb_init(void){

    caps_locked = 0;
    caps_released = 1;


    enable_irq(1);
}

/* 
 * kb_init
 *   DESCRIPTION: handles keyboard interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void handle_keyboard(void){

    cli();

    uint8_t scancode_idx = inb(READ_TYPED_KEY);
    int key_out;
    int is_special = 0;



    switch(scancode_idx){

        // pressed
        case BACKSPACE_PRESSED:
        terminals[currentTerminal].typing_buf_idx--;

        if(terminals[currentTerminal].typing_buf_idx < 0){
            terminals[currentTerminal].typing_buf_idx = 0;
            break;
        }

        if(terminals[currentTerminal].typing_buf[terminals[currentTerminal].typing_buf_idx] == '\t'){
            print_backspace();
            print_backspace();
            print_backspace();
        }
        else{
            print_backspace();
        }

        kb_map_state[BACKSPACE_PRESSED] = 1;
        is_special = 1;
        break;

        case ESC_PRESSED:
        kb_map_state[ESC_PRESSED] = 1;
        is_special = 1;
        break;

        case ENTER_PRESSED:
        kb_map_state[ENTER_PRESSED] = 1;
        putc('\n');
        terminals[currentTerminal].typing_buf[terminals[currentTerminal].typing_buf_idx] = '\n';
        terminals[currentTerminal].enter_pressed = 1;
        is_special = 1;
        break;


        case PRT_SC_PRESSED:
        kb_map_state[PRT_SC_PRESSED] = 1;
        is_special = 1;
        break;

        case L_SHIFT_PRESSED:
        kb_map_state[L_SHIFT_PRESSED] = 1;
        is_special = 1;
        break;

        case R_SHIFT_PRESSED:
        kb_map_state[R_SHIFT_PRESSED] = 1;
        is_special = 1;
        break;

        case CTRL_PRESSED:
        kb_map_state[CTRL_PRESSED] = 1;
        is_special = 1;
        break;

        case ALT_PRESSED:
        kb_map_state[ALT_PRESSED] = 1;
        is_special = 1;
        break;

        case TAB_PRESSED:
        kb_map_state[TAB_PRESSED] = 1;
        if(kb_map_state[L_SHIFT_PRESSED] != 1 && kb_map_state[R_SHIFT_PRESSED] != 1){
            putc(' ');
            putc(' ');
            putc(' ');
            terminals[currentTerminal].typing_buf[terminals[currentTerminal].typing_buf_idx] = '\t';
            terminals[currentTerminal].typing_buf_idx++;
        }
        is_special = 1;
        break;


        case CAPS_LOCK_PRESSED:
        kb_map_state[CAPS_LOCK_PRESSED] = 1;
        if(caps_released == 1){
            caps_locked = ~(caps_locked-2);
            caps_released = 0;
        }
        is_special = 1;
        break;


        case F1_PRESSED:
        kb_map_state[F1_PRESSED] = 1;
        is_special = 1;
        break;

        case F2_PRESSED:
        kb_map_state[F2_PRESSED] = 1;
        is_special = 1;
        break;

        case F3_PRESSED:
        kb_map_state[F3_PRESSED] = 1;
        is_special = 1;
        break;




        // released
        case BACKSPACE_RELEASED:
        kb_map_state[BACKSPACE_PRESSED] = 0;
        is_special = 1;
        break;

        case ESC_RELEASED:
        kb_map_state[ESC_PRESSED] = 0;
        is_special = 1;
        break;

        case ENTER_RELEASED:
        kb_map_state[ENTER_PRESSED] = 0;
        is_special = 1;
        terminals[currentTerminal].enter_pressed = 0;
        break;

        case PRT_SC_RELEASED:
        kb_map_state[PRT_SC_PRESSED] = 0;
        is_special = 1;
        break;

        case L_SHIFT_RELEASED:
        kb_map_state[L_SHIFT_PRESSED] = 0;
        is_special = 1;
        break;

        case R_SHIFT_RELEASED:
        kb_map_state[R_SHIFT_PRESSED] = 0; 
        is_special = 1;
        break;

        case CTRL_RELEASED:
        kb_map_state[CTRL_PRESSED] = 0;
        is_special = 1;
        break;

        case ALT_RELEASED:
        kb_map_state[ALT_PRESSED] = 0;
        is_special = 1;
        break;

        case CAPS_LOCK_RELEASED:
        kb_map_state[CAPS_LOCK_PRESSED] = 0;
        caps_released = 1;
        is_special = 1;
        break;

        case TAB_RELEASED:
        kb_map_state[TAB_PRESSED] = 0;
        is_special = 1;
        break;


        case F1_RELEASED:
        kb_map_state[F1_PRESSED] = 0;
        is_special = 1;
        break;

        case F2_RELEASED:
        kb_map_state[F2_PRESSED] = 0;
        is_special = 1;
        break;

        case F3_RELEASED:
        kb_map_state[F3_PRESSED] = 0;
        is_special = 1;
        break;

    }

    if(scancode_idx > 0 && scancode_idx <= MAX_VISIBLE_KEY && is_special == 0 && kb_map_state[ALT_PRESSED] == 0){  
        if(kb_map_state[CTRL_PRESSED] == 1 && kb_map_l[scancode_idx] == 'l'){

            terminal_clr();

            send_eoi(1);
            sti();
            return;
        }
        else if(kb_map_state[CTRL_PRESSED] == 1 && kb_map_l[scancode_idx] == 'c'){
            send_eoi(1);
            sti();
            sysHalt(0);
        }
        else if(kb_map_state[L_SHIFT_PRESSED] == 1 || kb_map_state[R_SHIFT_PRESSED] == 1) {
            key_out = kb_map_sh[scancode_idx];
        }
        else if(caps_locked == 1){
            key_out = kb_map_caps[scancode_idx];
        }
        else if(scancode_idx == BACKSPACE_PRESSED){
            key_out = '\0';
        }
        else{
            key_out = kb_map_l[scancode_idx];
        }
        if(terminals[currentTerminal].typing_buf_idx < CHAR_LIM-1 && key_out != '\0'){
            terminals[currentTerminal].typing_buf[terminals[currentTerminal].typing_buf_idx] = key_out;
            terminals[currentTerminal].typing_buf_idx++;
            putc(key_out);
        }
    }
    else if(kb_map_state[ALT_PRESSED] == 1){
        // needs to restore the screen to each terminal
        if(kb_map_state[F1_PRESSED] == 1){
            send_eoi(1);
            sti();
            switch_terminal(0);
        }
        else if(kb_map_state[F2_PRESSED] == 1){
            send_eoi(1);
            sti();
            switch_terminal(1);
        }
        else if(kb_map_state[F3_PRESSED] == 1){
            send_eoi(1);
            sti();
            switch_terminal(2);
        }
    }

    

    send_eoi(1);
    sti();
}







