#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesys.h"
#include "rtc.h"

#include "terminal.h"
#include "systemcall.h"

#define PASS 1
#define FAIL 0
#define VIDEO       0xB8000 

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 15; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}


// add more tests here
/* IDT Test - Divide By Zero
 * 
 * Asserts that the divide by zero test works
 * by dividing 1 by 0, and seeing if the 
 * correct exception is raised
 * Inputs: None
 * Outputs: None
 * Side Effects: Currently throws the system
 * 				 into a while(1) loop, as that
 * 				 is how the software exceptions
 * 			     are handled
 * Coverage: Divide Error Exception handler,
 * 			 all other software exceptions
 * 	         since they were defined in the 
 * 			 same way
 */
void div_zero(){
	// int x = 0;
	// x = 1 / x;
	return;
}

// tests for paging 
/* paging Test - accessing video mem
 * 
 * this tests to see if the memory at the video location is accessible 
 * Inputs: None
 * Outputs: 1 (if no problem), throw exception if problem
 * Side Effects: none
 * Coverage: tests the edge case of making sure that the video mem is accessible 
 */
int access_video_mem(){
	TEST_HEADER;

	int* video_mem = (int*) VIDEO;
	int check = *video_mem;
	check = 0;
	return PASS;
}

/* paging Test - memory not in range is inaccessible
 * 
 * this tests to see if accessing memory not in range throws a page fault 
 * Inputs: None
 * Outputs: 1 (if no problem), throw exception if problem
 * Side Effects: none
 * Coverage: tests that memory outside present range cannot be accessed
 */
void access_not_in_range(){
	TEST_HEADER;
	/* random address past 8Mb of present memory */
	int* not_in_range = (int*) (0x800000+4);
	int check = *not_in_range;
	check = 0;
	/* throw exception */
	return;
}
/* paging Test - not present table entry is inaccessible
 * 
 * this tests to see if accessing memory not in table throws a page fault 
 * Inputs: None
 * Outputs: 1 (if no problem), throw exception if problem
 * Side Effects: none
 * Coverage: tests that memory not in table cannot be accessed
 */
void access_no_table_entry (){
	TEST_HEADER;
	int* no_table = (int*) (0xB7FFF);
	int check = *no_table;
	check = 0;
	/* throw exception */
	return;
}

/* paging Test - accessing kernel mem
 * 
 * this tests to see if the memory at the kernel location is accessible 
 * Inputs: None
 * Outputs: 1 (if no problem), throw exception if problem
 * Side Effects: none
 * Coverage: tests that kernel mem is accessible 
 */
int access_kernel_mem(){
	TEST_HEADER;
	/* random address past 4MB of present memory */
	int* kernel_mem = (int*) (0x400000+4);
	int check = *kernel_mem;
	check = 0;
	return PASS;
}

/* paging Test - structure testing
 * 
 * this tests to see if the layout is set properly in which case the first entry in page_dir
 * is present and so is the second one.
 * Inputs: None
 * Outputs: 1/ Pass (if no problem), throw exception if problem
 * Side Effects: none
 * Coverage: tests that structure of paging was set up right 
 */
int page_structure(){
	TEST_HEADER;
	/* first page_dir exists */
	int retval;
 	if(page_directory[0].pagedir_KB.present == 1){
		retval = 1;
	}else{
		retval = 0;
	}if(page_directory[0].pagedir_MB.present == 1){
		retval = 1;
	}else{
		retval = 0;
	}
	if(retval==1){
		return PASS;
	}
	return FAIL;
}




/* Checkpoint 2 tests */






/* terminal test - reading from the keyboard, writing out to the terminal
 * clearing the terminal, and echoing out what was typed.
 * 
 * helps test if overflows are handled, everything is echoed back correctly
 * and all the keys are supported.
 * Inputs: none
 * Outputs: none
 */

char my_buff[128];

void test_terminal(){
	clear();
	while(1){
		terminal_read(0, (uint8_t*)my_buff, 128);
		terminal_write(0, (uint8_t*)my_buff, 128);
	}
}










/* File System Test - Read_Directory
 * tests filesys_open() and read_directory
 * by mimicking the behavior of ls and
 * attempting to read the files in the directory
 * Inputs: None
 * Returns: None
 * Effects: prints a list of files to the screen
 */
void ls_test(){
	// this used an old version of the file system before pcb and
	// proper fda was set up
	// clear();
	// int32_t fdret, cnt;
    // uint8_t buf[32];
	// if (-1 == (fdret = filesys_open ((uint8_t*)"."))) {
    //     printf("directory open failed\n");
    // }
	// while( (cnt = read_directory(fdret, buf, 32)) != 0){
	// 	printf("file_name: %s\n", buf);
	// }
	// filesys_close(fdret);
	// return;
}

/* File System Test - Read_File
 * tests filesys_open() and read_file
 * by attempting to read the files to the
 * screen
 * Inputs: None
 * Returns: None
 * Effects: outputs a file to the screen
 */
void fileRead_test(){
	// int32_t fd, cnt;
    // uint8_t buf[1024];
	// if((fd = open((uint8_t*)"grep")) != 0){
	// 	printf("Could not open file.\n");
	// 	return;
	// }
	// while (0 != (cnt = read_file(fd, buf, 1024))) {
    //     int i;
	// 	for(i = 0; i<cnt; i++){
	// 		putc(buf[i]);
	// 	}
    // }
	// printf("\n");
	// filesys_close(fd);
}

//for very large, read across data blocks, read up to file size, past file size, with offset, with offset past file size, and hardcode compare to expected output

// tests for rtc driver 

/* rtc driver Test - using read and write for all possible frequencies
 * 
 * tests that updates to frequency are made and reflected
 * Inputs: None
 * Outputs: prints out 1 each time an interrupt is recieved, returns 1 upon completion
 * Coverage: tests all valid frequencies that can be passed to write and checks if they are read correctly 
 */
int rtc_driver() {
	TEST_HEADER;

	uint32_t freq;
 	rtc_open(NULL);
	clear();

	for (freq = 2; freq <= 1024; freq *= 2) {
		int i;
 		for(i = 0; i <= 2 * freq; i++) {
 			rtc_read(0, NULL, 0);
 			printf("%d", 1);
 		}

 		rtc_write(0, (void*) &freq, sizeof(uint16_t));
 		clear();
	}
 	
 	rtc_close(0);
 	return PASS;
}

/* rtc driver Test - calling write on a buf that points to NULL
 * 
 * checks that if buf is a nullptr -1 is recieved
 * Inputs: None
 * Outputs: 1 if rtc_write fails, 0 otherwise
 * Coverage: tests handling of nullptrs by rtc_write 
 */
int rtc_write_null() {
	TEST_HEADER;
	rtc_open(NULL);
	uint32_t freq = NULL;

	int ret = rtc_write(0, (void*) &freq, 0);

	if (ret == -1) {
		return PASS;
	}

	rtc_close(0);

	return FAIL;
}

/* rtc driver Test - calling write on frequency that is not a power of 2
 * 
 * checks that for a frequency that is not a power of 2, -1 is recieved
 * Inputs: None
 * Outputs: 1 if rtc_write fails, 0 otherwise
 * Coverage: tests handling of invalid frequencies by rtc_write 
 */
int rtc_write_invalid_frequency() {
	TEST_HEADER;
	rtc_open(NULL);
	uint32_t freq = 13;

	int ret = rtc_write(0, (void*) &freq, 0);

	if (ret == -1) {
		return PASS;
	}

	rtc_close(0);

	return FAIL;
}

/* rtc driver Test - calling write on frequency that is greater than 1024
 * 
 * checks that for a frequency greater than 1024, -1 is recieved
 * Inputs: None
 * Outputs: 1 if rtc_write fails, 0 otherwise
 * Coverage: tests handling of out of bounds frequencies by rtc_write 
 */
int rtc_write_out_of_bounds_frequency() {
	TEST_HEADER;
	rtc_open(NULL);
	uint32_t freq = 2048;

	int ret = rtc_write(0, (void*) &freq, 0);

	if (ret == -1) {
		return PASS;
	}

	rtc_close(0);

	return FAIL;
}

/* Checkpoint 3 tests */


/* garbage value and return values Test
 * 
 *  ensures a few valid and invalid cases behave as expected
 * Inputs: None
 * Outputs: none
 * Coverage: tests handling of invalid frequencies by rtc_write 
 * sysExecute:
 * 	valid - "ls", "shell", "cat", "pingpong"
 *	invlid - "@#$FQCR\tT$A#Y$S\negrfs", " ", "\n"
 *
 *
 *
 */
void testGarbageVals(){
	TEST_HEADER;


	// checking bad or garbage input and return values for any funtion you write


	// // Test Execute
	// // valid - "ls", "shell", "cat", "pingpong"
	// // invlid - "@#$FQCR\tT$A#Y$S\negrfs", " ", "\n"

	// // valid tests
	// // "ls", "shell", "cat", "pingpong"
	// if(sysExecute((uint8_t*)"ls") == 0){
	// 	printf("sysExecute with 'ls': Pass\n");
	// }
	// else{
	// 	printf("sysExecute with 'ls': Fail\n");
	// }

	// if(sysExecute((uint8_t*)"shell") == 0){
	// 	printf("sysExecute with 'shell': Pass\n");
	// }
	// else{
	// 	printf("sysExecute with 'shell': Fail\n");
	// }

	// if(sysExecute((uint8_t*)"cat") == 0){
	// 	printf("sysExecute with 'cat': Pass\n");
	// }
	// else{
	// 	printf("sysExecute with 'cat': Fail\n");
	// }

	// if(sysExecute((uint8_t*)"pingpong") == 0){
	// 	printf("sysExecute with 'pingpong': Pass\n");
	// }
	// else{
	// 	printf("sysExecute with 'pingpong': Fail\n");
	// }


	// // invalid tests
	// // "@#$FQCR\tT$A#Y$S\negrfs", " ", ""
	// if(sysExecute((uint8_t*)"@#$FQCR\tT$A#Y$S\negrfs") == -1){
	// 	printf("sysExecute with '@#$FQCR\tT$A#Y$S\negrfs': Pass\n");
	// }
	// else{
	// 	printf("sysExecute with '@#$FQCR\tT$A#Y$S\negrfs': Fail\n");
	// }

	// if(sysExecute((uint8_t*)" ") == -1){
	// 	printf("sysExecute with ' ': Pass\n");
	// }
	// else{
	// 	printf("sysExecute with ' ': Fail\n");
	// }

	// if(sysExecute((uint8_t*)"") == -1){
	// 	printf("sysExecute with '': Pass\n");
	// }
	// else{
	// 	printf("sysExecute with '': Fail\n");
	// }


	

	// // Test Read
	// // valid - (stdin,buf,1)
	// // invlid - (-1,buf,1),(stdin,NULL,1),(stdin,buf,-1), (5,buf,1) 4MB-8MB, 128MB-132MB 

	// uint8_t* buf;
	// #define stdin 1

	// // valid
	// if(sysRead((int32_t)stdin, buf, (int32_t)1) == 0){
	// 	printf("sysRead with '(stdin,buf,1)': Pass\n");
	// }
	// else{
	// 	printf("sysRead with '(stdin,buf,1)': Fail\n");
	// }


	// // invalid
	// if(sysRead((int32_t)(-1*stdin), buf, (int32_t)1) == -1){
	// 	printf("sysRead with '(-1,buf,1)': Pass\n");
	// }
	// else{
	// 	printf("sysRead with '(-1,buf,1)': Fail\n");
	// }

	// if(sysRead((int32_t)stdin, NULL, (int32_t)1) == -1){
	// 	printf("sysRead with '(stdin,NULL,1)': Pass\n");
	// }
	// else{
	// 	printf("sysRead with '(stdin,NULL,1)': Fail\n");
	// }

	// if(sysRead((int32_t)stdin, buf, (int32_t)-1) == 0){
	// 	printf("sysRead with '(stdin,buf,-1)': Pass\n");
	// }
	// else{
	// 	printf("sysRead with '(stdin,buf,-1)': Fail\n");
	// }

	// if(sysRead((int32_t)5,buf,(int32_t)1) == -1){
	// 	printf("sysRead with '(5,buf,1)': Pass\n");
	// }
	// else{
	// 	printf("sysRead with '(5,buf,1)': Fail\n");
	// }

	




	// // Test Write
	// // valid - (stdout,buf,1)
	// // invlid - (-2,buf,1),(stdout,NULL,1),(stdout,buf,-1), (7,buf,1)

	// #define stdout 2
	// uint8_t* write_buf = (uint8_t*)"DrEaM TeAm";

	// // valid
	// if(sysWrite((int32_t)stdout, write_buf, (int32_t)1) == 0){
	// 	printf("sysWrite with '(stdout,write_buf,1)': Pass\n");
	// }
	// else{
	// 	printf("sysWrite with '(stdout,write_buf,1)': Fail\n");
	// }


	// // invalid
	// if(sysWrite((int32_t)(-1*stdout), write_buf, (int32_t)1) == -1){
	// 	printf("sysWrite with '(-1,write_buf,1)': Pass\n");
	// }
	// else{
	// 	printf("sysWrite with '(-1,write_buf,1)': Fail\n");
	// }
	// if(sysWrite((int32_t)stdout, NULL, (int32_t)1) == -1){
	// 	printf("sysWrite with '(stdout,NULL,1)': Pass\n");
	// }
	// else{
	// 	printf("sysWrite with '(stdout,NULL,1)': Fail\n");
	// }
	// if(sysWrite((int32_t)stdout, write_buf, (int32_t)-1) == -1){
	// 	printf("sysWrite with '(stdout,write_buf,-1)': Pass\n");
	// }
	// else{
	// 	printf("sysWrite with '(stdout,write_buf,-1)': Fail\n");
	// }
	// if(sysWrite((int32_t)7, write_buf, (int32_t)1) == -1){
	// 	printf("sysWrite with '(7,write_buf,1)': Pass\n");
	// }
	// else{
	// 	printf("sysWrite with '(7,write_buf,1)': Fail\n");
	// }


	// // Test Open
	// // valid - cmd
	// // invlid - NULL

	// uint8_t open_test[32];

	// // valid
	// if(sysOpen((uint8_t*)open_test) >= 0){
	// 	printf("sysOpen with 'open_test': Pass\n");
	// }
	// else{
	// 	printf("sysOpen with 'open_test': Fail\n");
	// }

	// // invalid
	// if(sysOpen(NULL) == -1){
	// 	printf("sysOpen with 'NULL': Pass\n");
	// }
	// else{
	// 	printf("sysOpen with 'NULL': Fail\n");
	// }




	// // Test Close
	// // valid - open_test
	// // invlid - NULL, close_test

	// uint8_t close_test[32];

	// // valid
	// if(sysClose((int32_t)open_test) == 0){
	// 	printf("sysClose with 'open_test': Pass\n");
	// }
	// else{
	// 	printf("sysClose with 'open_test': Fail\n");
	// }

	// // invalid
	// if(sysClose(NULL) == -1){
	// 	printf("sysClose with 'NULL': Pass\n");
	// }
	// else{
	// 	printf("sysClose with 'NULL': Fail\n");
	// }
	// if(sysClose((int32_t)close_test) == -1){
	// 	printf("sysClose with 'close_test': Pass\n");
	// }
	// else{
	// 	printf("sysClose with 'close_test': Fail\n");
	// }


}

/* Checkpoint 4 tests */

/* test_getArgs
 * 
 * Description: constructs a pcb like the one that
 * 				execute("shell") would, and then tests
 * 				the get args system call
 * Inputs: 		none
 * Outputs: 	none
 * Coverage: 	getargs system call
 */
void test_getArgs(){
	// TEST_HEADER;
	// uint8_t args[NAME_LEN];
	// memcpy(pcbarray[2]->arg, (uint8_t*)"frame0.txt", NAME_LEN);
	// if(sysGetArgs(&args, NAME_LEN) != 0){
	// 	printf("getargs system call failed");
	// }

	// if(strncmp(pcbarray[2]->arg, args, NAME_LEN) != 0){
	// 	puts(args);
	// 	putc('\n');
	// 	puts(pcbarray[2]->arg);
	// 	putc('\n');
	// 	printf("args were not copied correctly");
	// }
	
}


/* validate_vidmap
 * 
 * Description: checks valid and invalid arguments to vidmap
 * Inputs: 		none
 * Outputs: 	none
 * Coverage: 	vidmap system call
 */
int validate_vidmap(){
	if (-1 != vidmap((uint8_t **) 0x0)) {
		return FAIL;
	}
	
	if (-1 != vidmap((uint8_t **) 0x400000)) {
		return FAIL;
	}

	if (-1 != vidmap((uint8_t **) NULL)) {
		return FAIL;
	}

	return PASS;
}

/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	// TEST_OUTPUT("access_video_mem", access_video_mem());
	// TEST_OUTPUT("access_kernel_mem", access_kernel_mem());
	// TEST_OUTPUT("page_structure", page_structure());


	/* comment these functions out if you want to do anything
	 * besides test the exceptions! It calls a while(1) loop
	 */
	//div_zero();
	/* page faults */
	//access_not_in_range();
	//access_no_table_entry();

	
	/* Checkpoint 2 Tests */

	/* terminal test */
	//test_terminal();
	
	/* File system tests */

	//ls_test();
	//fileRead_test();

	/* RTC Driver tests */

	//TEST_OUTPUT("rtc_driver", rtc_driver()); //prints to terminal
	//TEST_OUTPUT("rtc_write_null", rtc_write_null());
	//TEST_OUTPUT("rtc_write_invalid_frequency", rtc_write_invalid_frequency());
	//TEST_OUTPUT("rtc_write_out_of_bounds_frequency", rtc_write_out_of_bounds_frequency());


	/* Checkpoint 3 Tests */

	//testGarbageVals();

	// Running the shell and testprint progams
	// Halting the shell and testprint programs
	// Using the read/write system calls to read/write to the terminal
	// Cheking the multiple steps of the exeute system call
	// Cheking the open/close/read/write system calls properly created, initialized, used, and cleaned up the file descriptor array

	/* Checkpoint 4 Tests */
	//test_getArgs();
	//TEST_OUTPUT("validate vidmap", validate_vidmap());

}
