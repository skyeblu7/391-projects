#ifndef FILESYS_H
#define FILESYS_H

#include "types.h"
#define FILENAME_LEN 32
#define DENTRY_NUM_RESERVED_BYTES 24
#define BOOT_BLOCK_NUM_RESERVED_BYTES 52
#define BOOT_BLOCK_MAX_DENTRIES 63
#define INODE_NUM_DATABLOCKS 1023
#define DENTRY_OFFSET 64
#define NUM_DENTRY_OFFSET 0
#define NUM_INODES_OFFSET 1
#define NUM_DATABLOCKS_OFFSET 2
#define LEN_OFFSET 4
#define FOURKB 4096
#define ONEKB 1024
#define ENDOFDB 32
#define NAME 32
#define INODE 36






/*global boot block that keeps track of boot block info*/
extern bootBlock_t bootBlock;
bootBlock_t bootBlock;

// Driver functions

/* system call support for opening files */
int32_t open_file(int32_t fd);
/* system call support for closing files */
int32_t close_file(int32_t fd);
/* system call support for writing files */
int32_t write_file(int32_t fd_ignore, uint8_t* buf_ignore, int32_t n);

/* system call support for opening directories */
int32_t open_dir(int32_t fd);
/* system call support for closing directories */
int32_t close_dir(int32_t fd);
/* system call support for writing directories */
int32_t write_dir(int32_t fd_ignore, uint8_t* buf_ignore, int32_t n);


/* file system read function is slightly more complex
 * the read syscall when passed an fd for a file, will 
 * handle the call with read_data
 */
/* read_dentry_by_name
 * reads directory entry with given fname into the dentry_t
 * passed as an argument
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

/* read_dentry_by_index
 * reads dentry with given index and into the dentry_t passed
 * as an argument
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

/* read_data
 * reads up to length bytes starting from position offset
 * in the file with inode number inode into the buffer.
 * returns the number of bytes read - returning 0 means EOF
 * has been reached
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* read_directory
 * reads the next file name from the directory. Tracked by fd (the global
 * variable, for now)
 */
int32_t read_directory(int32_t fd_idx, uint8_t* buf, int32_t n);

/* read_file
 * reads the next n bytes from the file described by fd (currently the global
 * variable) into buf. Used when system call read() is called on a file with type
 * == 2
 */
int32_t read_file(int32_t fd_idx, uint8_t* buf, int32_t n);

/* fops tables for files and directories */
fops_t file_fops;

fops_t dir_fops;

#endif
