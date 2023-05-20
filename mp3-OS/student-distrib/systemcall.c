#include "systemcall.h"
#include "types.h"
#include "terminal.h"
#include "rtc.h"
#include "x86_desc.h"
#include "filesys.h"
#include "scheduler.h"


int retval;
extern int bootFlag;
extern int currentlyServing;

/* parseCmd
 * DESCRIPTION: currently parses the first word (space separated)
 *              from input into buf and null terminates it
 * INPUTS:      input - the command to be parsed, e.g. "ls" or "cat grep"
 *              buf - buffer that first word is stored in
 * RETURNS:     NONE
 * EFFECTS:     writes into buf
 */
void parseCmd (uint8_t* input, uint8_t* cmd, uint8_t* arg){
    int i=0;
    int j;
    int k = 0;

    while(input[k] == ' ' && input[k] != '\0'){
        k++;
    }

    i = k;


    /* loop i so it's either at the end of the command or the first space */
    while(input[i] != '\n' && input[i] != '\0'){
        if (input[i] == ' '){
            break;
        }
        i++;
    }


    //copying command into command buffer
    strncpy((int8_t*)cmd, (int8_t*)input+k, i-k);
    cmd[i]='\0';
    
    while(input[i] == ' '){
        i++;
    }

    for(j = i; j < NAME_LEN; j++){
        if(input[j] == '\0' || input[j] == '\n'){
            break;
        }
    }
    //copying arg into arg buffer
    strncpy((int8_t*)arg, (int8_t*)(input + i), j);
    arg[j] = '\0';
}

/* sysExecute
 * DESCRIPTION: handler function for execute system call
 * INPUTS:      command - command to be executed, e.g. "shell" or "cat ls"
 * RETURNS:     0 on success, -1 on failure
 * EFFECTS:     returns into user space of new process. Changes user space
 *              page, as well as user and kernel space stack pointers. Also
 *              initializes (or re-initializes) the new process' pcb and sets
 *              curr_pcb poiter to that
 */
int sysExecute(uint8_t* command){
    sti();
    uint8_t cmd[NAME_LEN];
    uint8_t arg[NAME_LEN];
    dentry_t temp;
    int i;
    int newpid = -1;
    int fullProcs = 0;
    for(i=0;i<NUM_PROC;i++){
        if(pcbarray[i]->active == 1){
            fullProcs++;
        }
    }
    if(fullProcs == NUM_PROC && terminals[currentTerminal].active == 1){
        printf("No Available Processes\n");
        return -2;
    }

    parseCmd(command, cmd, arg);

    if(read_dentry_by_name(cmd, &temp)< 0){
        return -2;
    }
    uint8_t buf[4]; 
    (void) read_data(temp.inode, 0, buf, 4);
    /* these are the values the first 4 bytes of an executable should have */
    if(buf[0] !=0x7f || buf[1] != 0x45 || buf[2]!=0x4c || buf[3]!=0x46){
        /* if the ELF magic number is wrong, return -1 */
        return -2;
    }

    /* now that we know the file is executable, try to find a process id if we aren't starting a terminal for the first time */
    if(terminals[currentTerminal].active == 1){
        for(i = 0; i<NUM_PROC; i++){
            if(pcbarray[i]->active == 0){
                int j;
                newpid = i;
                /*set up the pcb*/
                pcbarray[newpid]->active = 1;
                fd_t stdin_temp = {0, 0, 0, &stdin_fops, 1}; //stdin fd has to be set to 1 (fd in use)
                pcbarray[newpid]->fda[0] = stdin_temp;
                fd_t stdout_temp = {0, 0, 0, &stdout_fops, 1}; //same for stdout fd
                pcbarray[newpid]->fda[1] = stdout_temp;
                fd_t empty_temp = {0, 0, 0, NULL, 0}; // the rest are empty
                for(j = 2; j<SIZEOF_FDA; j++){
                    pcbarray[newpid]->fda[j] = empty_temp;
                }
                memcpy(pcbarray[newpid]->cmd, &cmd, NAME_LEN);
                memcpy(pcbarray[newpid]->arg, &arg, NAME_LEN);
                if(terminals[currentTerminal].active){
                    pcbarray[newpid]->parent_pid = curr_pcb->curr_pid;
                }
                pcbarray[newpid]->curr_pid = newpid;
                terminals[currentTerminal].top_pid = newpid;
                break;
            }
        }
        curr_pcb = pcbarray[newpid];
    }
    else{
        /* we are starting a terminal for the first time */
        newpid = curr_pcb->curr_pid; //the pcb has already been set up by the switcher but the rest of execute uses newpid so we set it
    }
    

    /* if no processes were available, return -1 */
    if(newpid < 0){
        printf("No processes available!");
        return -2;
    }

    page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.present = 1;
    page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.read_write = 1;
    page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.user_supervisor = 1;
    page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.page_size = 1;
    page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.PB_addr = PHYS_BASE_ADDR  + newpid;
    /*flushing tlb by reloading cr3 look at page_enabling.S for more info. */
    flushingTLB();
    /* gets the number of bytes in the file and then calls read data for that many bytes to 
    write to buffer and then eventually to physical memory for the task */
    uint32_t* inodeptr = (uint32_t*) (((uint8_t*)bootBlock.addr) + ((temp.inode + 1) * FOURKB));
    uint32_t inode_len_b = *inodeptr;
    int fd = sysOpen(cmd);
    if(fd < 0){
        printf("No available fds to read executable into user memory");
        return -2;
    }
    (void) sysRead(fd, (uint8_t*)(USER_VIRTUAL + IMG_OFFSET), inode_len_b);
    (void) sysClose(fd);
    /* reading the first 4 bytes (24-27) of which is the virtual address of the first instruction 
    to be executed */
    uint8_t eip_buf[4];
    (void) read_data(temp.inode, 24, eip_buf, 4);
    int32_t eip = *((int32_t*) eip_buf);
    

    /* save esp and ebp */
    uint32_t esp_temp, ebp_temp;
    asm volatile ("movl %%esp, %0;"
            "movl %%ebp, %1;"
            : "=g" (esp_temp), "=g" (ebp_temp)
            :
            : "memory", "cc"
    );
    // if(terminals[currentTerminal].active == 1){
    //     curr_pcb->parent_esp = esp_temp;
    //     curr_pcb->parent_ebp = ebp_temp;
    // }

    curr_pcb->parent_esp = esp_temp;
    curr_pcb->parent_ebp = ebp_temp;
    
    terminals[currentTerminal].active = 1;

    /* get the address for the new kernel stack */
    uint32_t new_kernel_stack_addr = P0KESP - (OFFSET8KB * newpid);
    uint32_t new_user_stack_addr = USER_SP_VIRTUAL;

    /* context switch back to user stack for child process */
    tss.esp0 = new_kernel_stack_addr;
    tss.ss0 = KERNEL_DS;

    sti();
    asm volatile (
            "pushl %3;"
            "pushl %2;"
            "pushfl;"
            "pushl %1;"
            "pushl %0;"
            "iret;"
            : 
            : "g" (eip), "g" (USER_CS), "g" (new_user_stack_addr), "g" (USER_DS)
            : "memory", "cc"
    );

    asm volatile (
        "execute_return:"
    );
    //register int retval asm("eax");
    //printf("%d", retval);
    return retval;
}

int sysHalt(uint8_t status){
    if(curr_pcb->parent_pid == -1)
    {
        clear();
        terminals[currentTerminal].active = 0;
        switch_terminal(currentTerminal);
    }

    putc('\n');

    if(status == EXCEPTION_HALT_STATUS){
        retval = EXCEPTION_HALT_RETURN;
    }
    else{
        retval = status;
    }

    uint8_t parent_id = curr_pcb->parent_pid;
    int i;
    /* close all open fds */
    for(i = 0; i < SIZEOF_FDA; i++){
        if(curr_pcb->fda[i].fops){
            curr_pcb->fda[i].fops->close(i);
        }
    }


    /* move user page back to parent process */
    page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.present = 1;
    page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.read_write = 1;
    page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.user_supervisor = 1;
    page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.page_size = 1;
    page_directory[USER_VIRTUAL>>ADDR_OFFSET].pagedir_MB.PB_addr = PHYS_BASE_ADDR + parent_id;
    flushingTLB();
    /* move kernel stack pointer and SS to parent process */
    uint32_t ret_kernel_stack = P0KESP - (OFFSET8KB * parent_id);
    tss.esp0 = ret_kernel_stack;
    tss.ss0 = KERNEL_DS;

    /* restore parent esp and ebp */
    uint32_t esp_temp = curr_pcb->parent_esp;
    uint32_t ebp_temp = curr_pcb->parent_ebp;
    

    /* clear out pcb*/
    curr_pcb->active = 0;
    memset(&(curr_pcb->cmd), 0, NAME_LEN);
    memset(&(curr_pcb->arg), 0, NAME_LEN);
    curr_pcb->curr_pid = 0;
    curr_pcb->parent_pid = 0;
    curr_pcb->parent_esp = 0;
    curr_pcb->parent_ebp = 0;

    terminals[currentTerminal].top_pid = parent_id;

    /* restore curr_pcb to parent pcb */
    curr_pcb = pcbarray[parent_id];
    asm volatile (
            "movl %0, %%eax;"
            : 
            : "g" (retval)
            : "memory", "cc"
    );
    asm volatile (
            "movl %0, %%esp;"
            "movl %1, %%ebp;"
            : 
            : "g" (esp_temp), "g" (ebp_temp)
            : "memory", "cc"
    );
    sti();
    asm volatile (
        "jmp execute_return;"
    );
    /* this code won't run */
    return 0;
}


/* sysRead
 * DESCRIPTION: handler function for system call read.
 *              Just calls the given fd's read function
 * INPUTS:      fd - the fd for the file to read
 *              buf - buffer to read into
 *              nbytes - number of bytes to read
 * RETURNS:     number of bytes read (-1 on fail)
 * EFFECTS:     reads into buf
 */
int sysRead(int32_t fd, uint8_t* buf, int32_t nbytes){
    /* if fd is invalid, return -1 */
    if(fd < 0 || fd >= SIZEOF_FDA){
        return -1;
    }
    if(curr_pcb->fda[fd].flags == 0){
        return -1;
    }
    /* if memory is not in kernel page or user page, return -1 */
    if((int)buf < PHYS4MB || (int)buf >= PHYS8MB){
        if((int)buf <= USER_VIRTUAL || (int)buf > (USER_VIRTUAL + PHYS4MB)){
            return -1;
        }
    } 
    return curr_pcb->fda[fd].fops->read(fd, (uint8_t*)buf, nbytes);
}

/* sysWrite
 * DESCRIPTION: handler function for system call write.
 *              Just calls the given fd's write function,
 *              which may just be a bad_call depending on
 *              fda[fd]'s file type
 * INPUTS:      fd - the fd for the file to write
 *              buf - buffer to write from
 *              nbytes - number of bytes to written
 * RETURNS:     number of bytes written (-1 on fail)
 * EFFECTS:     writes into file
 */
int sysWrite(int32_t fd, uint8_t* buf, int32_t nbytes){
    /* if fd is invalid, return -1 */
    if(fd < 0 || fd >= SIZEOF_FDA){
        return -1;
    }
    if(curr_pcb->fda[fd].flags == 0){
        return -1;
    }
    /* if memory is not in kernel page or user page, return -1 */
    if((int)buf < PHYS4MB || (int)buf >= PHYS8MB){
        if((int)buf <= USER_VIRTUAL || (int)buf > (USER_VIRTUAL + PHYS4MB)){
            return -1;
        }
    } 
    return curr_pcb->fda[fd].fops->write(fd, (uint8_t*)buf, nbytes);
}

/* sysOpen
 * DESCRIPTION: handler function for system call open.
 *              allocates an fda entry for the given
 *              file and calls it's respective open
 *              function
 * INPUTS:      filename - the file to open
 * OUTPUS:      fd index on success, -1 on failure
 * EFFECTS:     allocates an fda entry
 */
int sysOpen(uint8_t* filename){
    int i;
    int fd = -1;
    dentry_t dentry;
    /* if you attempt to open a file with name "", read_dentry_by_name
     * will find a file with that name, because the entry dentries
     * have "" as their file name. This if statement prevents that from
     * happening
     */
    if(strncmp((int8_t*)filename, (int8_t*)"", 1) == 0){
        return -1;
    }
    /* find the first available fda entry */
    for(i = 0; i < SIZEOF_FDA; i++){
        if (curr_pcb->fda[i].flags == 0){
            fd = i;
            break;
        }
    }
    /* if there are none, return -1 */
    if(fd < 0){
        return -1;
    }
    /* file names cannot be longer than 32 characters. Set the 33rd character to \0 */
    /* if filename doesn't exist, return -1 */
    if(read_dentry_by_name(filename, &dentry) < 0){
        return -1;
    }

    /* set up fops table and potentially inode (if file is data)
     * this is all the data we only have now
     */
    switch(dentry.ftype){
        case 0:
        /* file is a device (the rtc) */
        curr_pcb->fda[fd].fops = &rtc_fops;
        break;
        case 1:
        /* file is a directory */
        curr_pcb->fda[fd].fops = &dir_fops;
        break;
        case 2:
        /* file is a data file. We also set the inode here
         * because the alternative is passing extra data
         * through to the fops open functio
         */
        curr_pcb->fda[fd].fops = &file_fops;
        curr_pcb->fda[fd].inode = dentry.inode;
        break;
        default:
        /* if file isn't a device, directory, or data return -1 */
        return -1;
    }
    /* call the correct open function */
    return curr_pcb->fda[fd].fops->open(fd);
}

/* sysClose
 * DESCRIPTION: handler function for system call close.
 *              Just calls the given fd's close function
 * INPUTS:      fd - the fd for the file to close
 * RETURNS:     0
 * EFFECTS:     clears curr_pcb->fda[fd]
 */
int sysClose(int32_t fd){
    /* if fd is invalid, return -1 */
    if(fd < 0 || fd >= SIZEOF_FDA){
        return -1;
    }
    if(curr_pcb->fda[fd].flags == 0){
        return -1;
    }
    return curr_pcb->fda[fd].fops->close(fd);
}

/* vidmap
 * DESCRIPTION: Points user program to video memory
 * INPUTS:      screen_start: pointer to point to video memory
 * RETURNS:     0 on success, -1 on failure
 * EFFECTS:     modifies page directory
 */
int32_t vidmap (uint8_t** screen_start){    
    //validating parameters 
    if(screen_start==NULL || screen_start<0 || screen_start>=(uint8_t**)(PHYS4MB + USER_VIRTUAL) || screen_start<= (uint8_t**)(PHYS4MB)){
        return -1;
    }
    //else map the video memory part of it

    page_directory[USER_VIDMAP_ADDR].pagedir_KB.present = 1;
    page_directory[USER_VIDMAP_ADDR].pagedir_KB.read_write = 1;
    //need to set the DPL to 3 so that it can be accessed by userspace
    page_directory[USER_VIDMAP_ADDR].pagedir_KB.user_supervisor = 1;
    page_directory[USER_VIDMAP_ADDR].pagedir_KB.write_through = 0;
    page_directory[USER_VIDMAP_ADDR].pagedir_KB.cache_disabled = 0;
    page_directory[USER_VIDMAP_ADDR].pagedir_KB.accessed = 0;
    page_directory[USER_VIDMAP_ADDR].pagedir_KB.reserved0 = 0;
    page_directory[USER_VIDMAP_ADDR].pagedir_KB.page_size = 0;
    page_directory[USER_VIDMAP_ADDR].pagedir_KB.ignored = 0;
    page_directory[USER_VIDMAP_ADDR].pagedir_KB.available = 0;
    /* moving from virtual to physical address */
    page_directory[USER_VIDMAP_ADDR].pagedir_KB.PTB_addr = ((unsigned long) page_table>>TABLE_ADDR_OFFSET);
    page_table[0].PB_addr = VIDEO >> TABLE_ADDR_OFFSET;
    page_table[0].present = 1;
    // flush tlb
    flushingTLB();
    *screen_start = (uint8_t*) (USER_VIDMAP);
    return 0;
}

/* sysGetArgs
 * DESCRIPTION: Reads program's command line arguments
 *              into user level buffer
 * INPUTS:      buf - the user level buffer that will
 *              hold the arguments after the system
 *              call
 *              nbytes - the number of bytes to read
 *              into that buffer
 * RETURNS:     0 on success, -1 on failure
 */
int sysGetArgs(uint8_t* buf, int32_t nbytes){
    if(strlen((int8_t*)curr_pcb->arg) == 0){
        return -1;
    }

    if(memcpy(buf, curr_pcb->arg, nbytes) < 0){
        return -1;
    }
    return 0;
}
