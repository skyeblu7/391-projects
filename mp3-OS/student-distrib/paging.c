#include "paging.h"
#include "x86_desc.h"
#include "lib.h"



/* 
 * paging_setup
 *   DESCRIPTION: sets up the table and the page directories for MB pages and KB pages
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: creates present and non present pages.
 */
void paging_setup(){
    int i = 0;
    /* initialize page table */
    for (i = 0; i < NUM_TABLE_ENTRIES; i++){
        first_page_table[i].present = 0;
        first_page_table[i].read_write = 0;
        // only video memory should be set to present and read/write enabled
        // bottom 12 bits not used for address
        if((i << 12) == VIDEO){
            first_page_table[i].present = 1;
            first_page_table[i].read_write = 1;
        }
        if((i << 12) == ACACHE){
            first_page_table[i].present = 1;
            first_page_table[i].read_write = 1;
        }
        if((i << 12) == BCACHE){
            first_page_table[i].present = 1;
            first_page_table[i].read_write = 1;
        }
        if((i << 12) == CCACHE){
            first_page_table[i].present = 1;
            first_page_table[i].read_write = 1;
        }
        first_page_table[i].user_supervisor = 0;
        first_page_table[i].write_through = 0;
        first_page_table[i].cache_disabled = 0;
        first_page_table[i].accessed = 0;
        first_page_table[i].dirty = 0;
        first_page_table[i].reserved0 = 0;
        first_page_table[i].ignored = 0;
        first_page_table[i].available = 0;
        first_page_table[i].PB_addr = i;
    }

    for (i = 0; i < NUM_TABLE_ENTRIES; i++){
        page_table[i].present = 0;
        page_table[i].read_write = 1;
        page_table[i].user_supervisor = 1;
        page_table[i].write_through = 0;
        page_table[i].cache_disabled = 0;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].reserved0 = 0;
        page_table[i].ignored = 0;
        page_table[i].available = 0;
        switch(i){
            case 1:
            page_table[i].PB_addr = ACACHE;
            break;
            case 2:
            page_table[i].PB_addr = BCACHE;
            break;
            case 3:
            page_table[i].PB_addr = CCACHE;
            break;
            default:
            page_table[i].PB_addr = i;
            break;
        }
        
    }
 
     /* set present and enable read/write */
    page_directory[0].pagedir_KB.present = 1;
    page_directory[0].pagedir_KB.read_write = 1;
    page_directory[0].pagedir_KB.user_supervisor = 0;
    page_directory[0].pagedir_KB.write_through = 0;
    page_directory[0].pagedir_KB.cache_disabled = 0;
    page_directory[0].pagedir_KB.accessed = 0;
    page_directory[0].pagedir_KB.reserved0 = 0;
    page_directory[0].pagedir_KB.page_size = 0;
    page_directory[0].pagedir_KB.ignored = 0;
    page_directory[0].pagedir_KB.available = 0;
    /* moving from virtual to physical address */
    page_directory[0].pagedir_KB.PTB_addr = ((unsigned long) first_page_table >> 12);

    // set present, enable read/write, set page size to 4MB
    page_directory[1].pagedir_MB.present = 1;
    page_directory[1].pagedir_MB.read_write = 1;
    page_directory[1].pagedir_MB.user_supervisor = 0;
    page_directory[1].pagedir_MB.write_through = 0;
    page_directory[1].pagedir_MB.cache_disabled = 0;
    page_directory[1].pagedir_MB.accessed = 0;
    page_directory[1].pagedir_MB.dirty = 0;
    page_directory[1].pagedir_MB.page_size = 1;
    page_directory[1].pagedir_MB.ignored = 0;
    page_directory[1].pagedir_MB.available = 0;
    page_directory[1].pagedir_MB.reserved0 = 0;
    page_directory[1].pagedir_MB.reserved = 0;
    page_directory[1].pagedir_MB.PB_addr = 1;

    // initialize rest of page directory to be not present
    // start from 2 since first 2 entries are already initialized
    for (i = 2; i < NUM_DIR_ENTRIES; i++){
        if (i == 34) {
            continue;
        }
        page_directory[i].pagedir_MB.present = 0;
        page_directory[i].pagedir_MB.read_write = 0;
        page_directory[i].pagedir_MB.user_supervisor = 0;
        page_directory[i].pagedir_MB.write_through = 0;
        page_directory[i].pagedir_MB.cache_disabled = 0;
        page_directory[i].pagedir_MB.accessed = 0;
        page_directory[i].pagedir_MB.dirty = 0;
        page_directory[i].pagedir_MB.page_size = 0;
        page_directory[i].pagedir_MB.ignored = 0;
        page_directory[i].pagedir_MB.available = 0;
        page_directory[i].pagedir_MB.reserved0 = 0;
        page_directory[i].pagedir_MB.reserved = 0;
        page_directory[i].pagedir_MB.PB_addr = i;
    }

    
    // configure control registers to enable paging
    loadPageDirectory();
    enableMixing();
    enablePaging();

    return;
}
