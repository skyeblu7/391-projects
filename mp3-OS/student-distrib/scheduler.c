#include "scheduler.h"
#include "lib.h"
#include "x86_desc.h"
#include "systemcall.h"
#include "filesys.h"
#include "terminal.h"
#include "rtc.h"


extern int interrupt_count;

void scheduler(){
    // int nextTerm;
    // int nextProc;
    // uint32_t espTemp;
    // uint32_t ebpTemp;

    // if(bootFlag >= 0){
    //     uint8_t* buf = (uint8_t*)"shell";
    //     sysExecute(buf);
    // }
    
    // /* Determine, based on what terminal we are currently serving, where we are going next */
    // switch(currentlyServing){
    //     case A:
    //     nextTerm = B;
    //     nextProc = procB;
    //     break;
    //     case B:
    //     nextTerm = C;
    //     nextProc = procC;
    //     break;
    //     case C:
    //     nextTerm = A;
    //     nextProc = procA;
    //     break;
    // }

    // /* update currentlyServing to reflect who we're serving after return */
    // currentlyServing = nextTerm;

    
    // /* new kernel stack address is the address of the kernel stack for the process we're switching to */
    // uint32_t newKernelStack = P0KESP - (OFFSET8KB * nextProc);

    // /* store ebp and esp from current process in it's pcb */
    // asm volatile (
    //     "movl %%esp, %0;"
    //     "movl %%ebp, %1;"
    //     : "=g" (espTemp), "=g" (ebpTemp)
    //     :
    //     : "memory", "cc"
    // );

    // curr_pcb->esp = espTemp;
    // curr_pcb->ebp = ebpTemp;

    // /* get new esp and ebp from next process' pcb */
    // espTemp = pcbarray[nextProc]->esp;
    // ebpTemp = pcbarray[nextProc]->ebp;

    // /* switch kernel stack */
    // tss.esp0 = newKernelStack;

    // tss.ss0 = KERNEL_DS;

    // /* redirect user space page to what will be the active process */
    // if(pcbarray[nextProc]->is_viewed){
    //     first_page_table[VIDEO >> TABLE_ADDR_OFFSET].PB_addr = VIDEO >> TABLE_ADDR_OFFSET;
    // }
    // else{
    //     first_page_table[VIDEO >> TABLE_ADDR_OFFSET].PB_addr = first_page_table[nextTerm+1].PB_addr;
    // }
    // //page_table[0] is where the pointer to video memory is
    // //1,2,3 will be for A,B,C
    // //move those values around
    // page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.PB_addr = PHYS_BASE_ADDR + nextProc;
    // flushingTLB();
    
    // curr_pcb = pcbarray[nextProc];

    // /* move next process' esp and ebp into those registers */
    // asm volatile(
    //         "movl %0, %%esp;"
    //         "movl %1, %%ebp;"
    //         :
    //         : "g" (espTemp), "g" (ebpTemp)
    //         : "memory", "cc"
    // );
    
    // /* return to new process */
    // return;
    
}


void switch_terminal(int terminal_num){

    interrupt_count = MAX_FREQ / 32;

    int leaving = currentTerminal;
    int j;
    uint32_t newKernelStack;
    uint32_t espTemp, ebpTemp;

    /* update currentTerminal */
    currentTerminal = terminal_num;

    set_cursor(terminals[currentTerminal].screen_x, terminals[currentTerminal].screen_y);

    /* save esp and ebp */
    asm volatile (
        "movl %%esp, %0;"
        "movl %%ebp, %1;"
        : "=g" (espTemp), "=g" (ebpTemp)
        :
        : "memory", "cc"
    );

    pcbarray[terminals[leaving].top_pid]->esp = espTemp;
    pcbarray[terminals[leaving].top_pid]->ebp = ebpTemp;

    memcpy(terminals[leaving].vidmem, (void*)VIDEO, FOURKB);

    /* if the terminal we are switching to has never been switched to, we have special behavior */
    if(!terminals[terminal_num].active){

        /* if the terminal we are switching to is inactive, we need to initialize it */
        terminals[currentTerminal].active = 0;
        terminals[currentTerminal].screen_x = 0;
        terminals[currentTerminal].screen_y = 0;
        terminals[currentTerminal].typing_buf_idx = 0;
        terminals[currentTerminal].top_pid = currentTerminal;
        terminals[currentTerminal].enter_pressed = 0;


        /* dedicate processes 0,1,2 to terminals 0,1,2 */
        pcbarray[currentTerminal]->active = 1;
        fd_t stdin_temp = {0, 0, 0, &stdin_fops, 1}; //stdin fd has to be set to 1 (fd in use)
        pcbarray[currentTerminal]->fda[0] = stdin_temp;
        fd_t stdout_temp = {0, 0, 0, &stdout_fops, 1}; //same for stdout fd
        pcbarray[currentTerminal]->fda[1] = stdout_temp;
        fd_t empty_temp = {0, 0, 0, NULL, 0}; // the rest are empty
        for(j = 2; j<SIZEOF_FDA; j++){
            pcbarray[currentTerminal]->fda[j] = empty_temp;
        }
        pcbarray[currentTerminal]->curr_pid = currentTerminal;

        curr_pcb = pcbarray[currentTerminal];
        
        /* switch the screens */
        memcpy(terminals[leaving].vidmem, (void*)VIDEO, FOURKB);
        memcpy((void*)VIDEO, terminals[currentTerminal].vidmem, FOURKB);
        //putc(currentTerminal+48);

        uint8_t* buf = (uint8_t*)"shell";
        sysExecute(buf);
    }
    
    curr_pcb = pcbarray[terminals[currentTerminal].top_pid];

    newKernelStack = P0KESP - (OFFSET8KB * terminals[currentTerminal].top_pid);
    tss.esp0 = newKernelStack;
    tss.ss0 = KERNEL_DS;
    page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.PB_addr = PHYS_BASE_ADDR + terminals[currentTerminal].top_pid;
    flushingTLB();    

    memcpy((void*)VIDEO, terminals[currentTerminal].vidmem, FOURKB);

    espTemp = pcbarray[terminals[currentTerminal].top_pid]->esp;
    ebpTemp = pcbarray[terminals[currentTerminal].top_pid]->ebp;
    asm volatile(
            "movl %0, %%esp;"
            "movl %1, %%ebp;"
            :
            : "g" (espTemp), "g" (ebpTemp)
            : "memory", "cc"
    );

    return;
}

