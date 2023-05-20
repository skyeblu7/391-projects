.text
.globl loadPageDirectory
.globl enablePaging
.globl enableMixing
.globl flushingTLB

/* 
 * loadPageDirectory
 *   DESCRIPTION: loads the page directory by loading its address into cr3 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: modifies cr3.
 */
loadPageDirectory:
pushl %ebp
movl %esp, %ebp
movl $page_directory, %eax
movl %eax, %cr3
leave
ret

/* 
 * enableMixing
 *   DESCRIPTION: enables having pages of 4MB by changing the 5th bit of the cr4.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: modifies cr4.
 */
enableMixing:
pushl %ebp
movl %esp, %ebp
movl %cr4, %eax
/* 0x000000010 sets the 5th bit in cr4 */
orl  $0x00000010, %eax
movl %eax, %cr4
leave
ret

/* 
 * enablePaging
 *   DESCRIPTION: enables paging by setting the 32nd bit in cr0.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: modifies cr0.
 */
enablePaging:
pushl %ebp
movl %esp, %ebp
movl %cr0, %eax
/* 0x80000000 sets the 32nd bit in cr0 */
orl $0x80000000, %eax
movl %eax, %cr0
leave
ret

/* 
 * flushingTLB
 *   DESCRIPTION: flushes TLB by reloading cr3
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: flushes TLB.
 */
flushingTLB:
pushl %ebp
movl %esp, %ebp
movl %cr3, %eax
movl %eax, %cr3
leave
ret


