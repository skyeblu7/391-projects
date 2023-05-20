/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "idt.h"
#include "paging.h"
#include "systemcall.h"
#include "filesys.h"


// new files
#include "rtc.h"
#include "kb.h"
#include "terminal.h"
#include "pit.h"
#include "scheduler.h"

#define RUN_TESTS 

/* Macros.
 * Check if the bit BIT in FLAGS is set. 
 */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))\

/* Check if MAGIC is valid and print the Multiboot information structure
 *  pointed by ADDR.
 */
void entry(unsigned long magic, unsigned long addr) {

    multiboot_info_t *mbi;

    /* Clear the screen. */
    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%#x\n", (unsigned)magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf("flags = 0x%#x\n", (unsigned)mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG(mbi->flags, 0))
        printf("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG(mbi->flags, 1))
        printf("boot_device = 0x%#x\n", (unsigned)mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG(mbi->flags, 2))
        printf("cmdline = %s\n", (char *)mbi->cmdline);

    if (CHECK_FLAG(mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        while (mod_count < mbi->mods_count) {
            if(mod_count == 0){
                /* the file system is module 0. mod->mod_start is the address of the file system
                 * so while we have access to it, define the bootBlock struct so file system can
                 * work
                 */
                bootBlock.addr = (void*)mod->mod_start;
                /* the first 12 bytes of the boot block contain metadata about the file system.
                 * mod->mod_start plus some pointer arithmetic will point to them, cast them as
                 * 32-bit (4-byte) ints and dereference them to access the values
                 */
                bootBlock.dir_count = *((uint32_t*) mod->mod_start + NUM_DENTRY_OFFSET);
                bootBlock.inode_count = *((uint32_t*) mod->mod_start + NUM_INODES_OFFSET);
                bootBlock.data_count = *((uint32_t*) mod->mod_start + NUM_DATABLOCKS_OFFSET);
            }
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for (i = 0; i < 16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
        printf("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(mbi->flags, 5)) {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);
        printf("elf_sec: num = %u, size = 0x%#x, addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned)elf_sec->num, (unsigned)elf_sec->size,
                (unsigned)elf_sec->addr, (unsigned)elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(mbi->flags, 6)) {
        memory_map_t *mmap;
        printf("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
        for (mmap = (memory_map_t *)mbi->mmap_addr;
                (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *)((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
            printf("    size = 0x%x, base_addr = 0x%#x%#x\n    type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned)mmap->size,
                    (unsigned)mmap->base_addr_high,
                    (unsigned)mmap->base_addr_low,
                    (unsigned)mmap->type,
                    (unsigned)mmap->length_high,
                    (unsigned)mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity = 0x0;
        the_ldt_desc.opsize      = 0x1;
        the_ldt_desc.reserved    = 0x0;
        the_ldt_desc.avail       = 0x0;
        the_ldt_desc.present     = 0x1;
        the_ldt_desc.dpl         = 0x0;
        the_ldt_desc.sys         = 0x0;
        the_ldt_desc.type        = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity   = 0x0;
        the_tss_desc.opsize        = 0x0;
        the_tss_desc.reserved      = 0x0;
        the_tss_desc.avail         = 0x0;
        the_tss_desc.seg_lim_19_16 = TSS_SIZE & 0x000F0000;
        the_tss_desc.present       = 0x1;
        the_tss_desc.dpl           = 0x0;
        the_tss_desc.sys           = 0x0;
        the_tss_desc.type          = 0x9;
        the_tss_desc.seg_lim_15_00 = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }

    clear();

    /* hardcode the memory addresses for backup terminal screens */
    terminals[A].vidmem = (void*)ACACHE;
    terminals[B].vidmem = (void*)BCACHE;
    terminals[C].vidmem = (void*)CCACHE;

    /* initializing the PCB array getting the variable from 
    systemcall.h */
    int i, j;
    for(i = 0; i<3; i++){
        terminals[i].active = 0;
        terminals[i].enter_pressed = 0;
        terminals[i].screen_x = 0;
        terminals[i].screen_y = 0;
        terminals[i].typing_buf_idx = 0;
    }
    for (i=0; i<6; i++){
        pcbarray[i] = (pcb_t*)(PHYS8MB - (OFFSET8KB*(i+1)));
        //marking it not present and clearing all the values
        pcbarray[i]->active = 0;
        fd_t empty_fd = {0, 0, 0, NULL, 0};
        for(j = 0; j < SIZEOF_FDA; j++){
            pcbarray[i]->fda[j] = empty_fd;
        } 
        memset(&(pcbarray[i]->cmd), '\0', NAME_LEN);
        memset(&(pcbarray[i]->arg), '\0', NAME_LEN);
        pcbarray[i]->curr_pid = 0;
        pcbarray[i]->parent_pid = -1;
        pcbarray[i]->parent_esp = 0;
        pcbarray[i]->parent_ebp = 0;
        pcbarray[i]->esp = 0;
        pcbarray[i]->ebp = 0;
    }

    pcbarray[A]->active = 1;
    pcbarray[B]->active = 1;
    pcbarray[C]->active = 1;

    curr_pcb = pcbarray[0];
    terminals[A].top_pid = 0;


    /* Populate the IDT */
    idt_setup();
    /* enabling paging and setting up paging */
    paging_setup();
    /* Init the PIC */
    i8259_init();

    /* Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff... */
    rtc_init();
    pit_init();

    kb_init();
    
    

    /* set up fops tables */
    file_fops.open = open_file;
    file_fops.close = close_file;
    file_fops.read = read_file;
    file_fops.write = write_file;

    dir_fops.open = open_dir;
    dir_fops.close = close_dir;
    dir_fops.read = read_directory;
    dir_fops.write = write_dir;

    stdin_fops.open = bad_call;
    stdin_fops.close = bad_call;
    stdin_fops.read = terminal_read;
    stdin_fops.write = bad_read_write;

    stdout_fops.open = bad_call;
    stdout_fops.close = bad_call;
    stdout_fops.read = bad_read_write;
    stdout_fops.write = terminal_write;

    rtc_fops.open = rtc_open;
    rtc_fops.close = rtc_close;
    rtc_fops.read = rtc_read;
    rtc_fops.write = rtc_write;
    


    /* Enable interrupts */
    /* Do not enable the following until after you have set up your
     * IDT correctly otherwise QEMU will triple fault and simple close
     * without showing you any output */
    printf("Enabling Interrupts\n");
    sti();


#ifdef RUN_TESTS
    /* Run tests */
    //launch_tests();
#endif
    /* Execute the first program ("shell") ... */
    switch_terminal(0);




    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".1: hlt; jmp .1;");
}


