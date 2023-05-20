#include "filesys.h"
#include "types.h"
#include "lib.h"
#include "systemcall.h"

/* open_file
 * DESCRIPTION: called immediately after the file descriptor is allocated
 *              and fops table is set up, and inode is copied in. Sets 
 *              db_num and byte_num for the fd to 0
 * INPUTS:      fd - the index to the fda for this file
 * RETURNS:     fd
 * EFFECTS:     Sets up this fda entry
 */
int32_t open_file(int32_t fd){
    curr_pcb->fda[fd].db_num = 0;
    curr_pcb->fda[fd].byte_num = 0;
    curr_pcb->fda[fd].flags = 1;
    return fd;
}

/* close_file
 * DESCRIPTION: clears the fda for the specified file
 * INPUTS:      fd - the index to the fda for the file
 * RETURNS:     0
 * EFFECTS:     Clears the fda
 */
int32_t close_file(int32_t fd){
    curr_pcb->fda[fd].db_num = 0;
    curr_pcb->fda[fd].byte_num = 0;
    curr_pcb->fda[fd].inode = 0;
    curr_pcb->fda[fd].fops = NULL;
    curr_pcb->fda[fd].flags = 0;
    return 0;
}

/* write_file
 * DESCRIPTION: this just returns -1 because writing is not allowed
 * INPUTS:      fd_ignore - gets ignored
 *              buf_ignore - gets ignored
 *              nbytes_ignore - gets ignored
 */
int32_t write_file(int32_t fd_ignore, uint8_t* buf_ignore, int32_t n){
    return -1;
}

/* open_dir
 * DESCRIPTION: called immediately after the file descriptor is allocated
 *              and fops table is set up. Sets db+num and byte_num to 0
 * INPUTS:      fd - the index to the fda for this file
 * RETURNS:     fd
 * EFFECTS:     Sets up this fda entry
 */
int32_t open_dir(int32_t fd){
    curr_pcb->fda[fd].db_num = 0;
    curr_pcb->fda[fd].byte_num = 0;
    curr_pcb->fda[fd].inode = 0;
    curr_pcb->fda[fd].flags = 1;
    return fd;
}

/* close_dir
 * DESCRIPTION: clears the fda for the specified file
 * INPUTS:      fd - the index to the fda for the file
 * RETURNS:     0
 * EFFECTS:     Clears the fda
 */
int32_t close_dir(int32_t fd){
    curr_pcb->fda[fd].db_num = 0;
    curr_pcb->fda[fd].byte_num = 0;
    curr_pcb->fda[fd].inode = 0;
    curr_pcb->fda[fd].fops = NULL;
    curr_pcb->fda[fd].flags = 0;
    return 0;
}

/* write_dir
 * DESCRIPTION: this just returns -1 because writing is not allowed
 * INPUTS:      fd_ignore - gets ignored
 *              buf_ignore - gets ignored
 *              nbytes_ignore - gets ignored
 */
int32_t write_dir(int32_t fd_ignore, uint8_t* buf_ignore, int32_t n){
    return -1;
}

/* read_dentry_by_name
 * DESCRIPTION: Traverses dentries in boot block to find dentry
 *              with name fname. Calls read_dentry_by_index with
 *              the index it finds to populate dentry_t dentry
 * INPUTS: fname  - pointer to char, or the string that holds the
 *                  name of the file we're locating
 *         dentry - the dentry_t struct that we populate with the
 *                  desired file's information 
 * RETURNS: 0
 * EFFECTS: Reads into dentry_t struct (through calling read_dentry_by_index)
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    uint32_t index;
    uint8_t* dentries_addr = ((uint8_t*)bootBlock.addr) + (DENTRY_OFFSET);
    //uint8_t* curname;
    uint8_t curname[NAME_LEN];

    /* for each of the dentries, check if their fname is equal to the given fname */
    for(index = 0; index < BOOT_BLOCK_MAX_DENTRIES; index++){

        memcpy((void*)&curname, (void*)(dentries_addr+DENTRY_OFFSET * index), 32);
        curname[32] = '\0';

        //curname = (dentries_addr + (DENTRY_OFFSET * index));
        //curname[32] = '\0';
        if(strncmp((int8_t*)curname, (int8_t*)fname, NAME_LEN) == 0){
            read_dentry_by_index(index, dentry);
            return 0;
        }
    }

    /* if we reach here, file was not found in dentries */
    return -1;
}

/* read_dentry_by_index
 * DESCRIPTION: Copies the contents of the specified dentry into
 *              dentry_t dentry passed as an arg, then returns 0
 * INPUTS: index  - the index referring to the desired dentry's 
 *                  location in the boot block
 *         dentry - the dentry_t struct that we populate with the
 *                  desired file's information 
 * RETURNS: 0
 * EFFECTS: Reads into dentry_t struct
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    dentry->fname = ((uint8_t*)bootBlock.addr) + DENTRY_OFFSET + (DENTRY_OFFSET * index);
    dentry->ftype = *((uint8_t*)(dentry->fname) + NAME);
    dentry->inode = *((uint8_t*)(dentry->fname) + INODE);
    return 0;
}

/* read_directory
 * DESCRIPTION: Reads up to n bytes of the next file
 *              name in the directory into buf
 * INPUTS: buf - buffer to place data into
 *         fd  - fd of directory being read (currently ignored)
 *         n   - number of bytes to be read into buf
 * RETURNS: number of bytes read 
 * EFFECTS: reads into buf
 */
int32_t read_directory(int32_t fd, uint8_t* buf, int32_t n){
    int index =  curr_pcb->fda[fd].db_num;
    int startingByte =  curr_pcb->fda[fd].byte_num;
    uint8_t* cur_name = ((uint8_t*)bootBlock.addr) + ((index + 1) * DENTRY_OFFSET) + startingByte;
    int i;

    if(startingByte == 0 && *cur_name == '\0'){
        return 0;
    }
    for(i = 0; i < n; i++){
        memcpy(buf, cur_name, 1);
        buf++;
        cur_name++;
        curr_pcb->fda[fd].byte_num++;
        if(curr_pcb->fda[fd].byte_num == ENDOFDB){
            curr_pcb->fda[fd].db_num++;
            curr_pcb->fda[fd].byte_num = 0;
            cur_name = ((uint8_t*)bootBlock.addr) + ((curr_pcb->fda[fd].db_num + 1) * DENTRY_OFFSET);
        }
    }
    return i;
}

/* read_file
 * DESCRIPTION: Reads up to n bytes of the specified file
 *              into buf
 * INPUTS: buf - buffer to place data into
 *         fd  - fd of file being read (currently ignored)
 *         n   - number of bytes to be read into buf
 * RETURNS: number of bytes read 
 * EFFECTS: reads into buf
 */
int32_t read_file(int32_t fd, uint8_t* buf, int32_t n){
    int n2; //temp variable used if read goes over data block boundaries
    uint32_t inode = curr_pcb->fda[fd].inode;
    uint32_t offset = curr_pcb->fda[fd].db_num*FOURKB + curr_pcb->fda[fd].byte_num;
    uint32_t* inodeptr = (uint32_t*) (((uint8_t*)bootBlock.addr) + ((inode + 1) * FOURKB));
    uint32_t inode_len_b = *inodeptr;
    if(offset > inode_len_b){
        return 0;
    }
    if(curr_pcb->fda[fd].byte_num + n > FOURKB){
        n2 = n;
        n = FOURKB - curr_pcb->fda[fd].byte_num;
        n2 -= n;
        read_data(inode, offset, buf, n);
        curr_pcb->fda[fd].byte_num = 0;
        curr_pcb->fda[fd].db_num++;
        return read_file(fd, buf+n, n2);
    }
    if(offset + n > inode_len_b){
        n = inode_len_b - offset;
        curr_pcb->fda[fd].byte_num += n;
        return read_data(inode, offset, buf, n);
    }
    
    curr_pcb->fda[fd].byte_num += n;
    return read_data(inode, offset, buf, n);
}

/* read_data
 * DESCRIPTION: Reads up to length bytes starting from position
 *              offset in the file with inode number inode into
 *              the buffer. 
 * INPUTS: inode  - the inode number of the file we're reading from
 *         offset - the offset applied to determine where we start
 *                  reading from
 *         buf    - buffer to place data into
 *         length - number of bytes to read (or until end of file)
 *         fd_idx - currently ignored, but will be the file descriptor
 * RETURNS: number of bytes read (0 means end of file)
 * EFFECTS: reads into buf
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    int bytes_read;
    uint8_t* inode_ptr = ((uint8_t*) bootBlock.addr) + ((inode + 1) * FOURKB);
    uint8_t* db_ptr = ((uint8_t*) bootBlock.addr) + ((bootBlock.inode_count + 1) * FOURKB) + ( *(inode_ptr + LEN_OFFSET + (offset / ONEKB)) * FOURKB);
    db_ptr += offset % FOURKB;

    /* actually copy the data */
    for(bytes_read = 0; bytes_read < length; bytes_read++){
        memcpy(buf, db_ptr, 1);
        buf++;
        db_ptr++;
    }
    return bytes_read;
}
