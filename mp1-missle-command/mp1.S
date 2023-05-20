
# mp1.S - missile-command version
# Good luck, and have fun!
.ALIGN 4

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  	= 0
	X	  		= 4
	Y	  		= 8
	VX	  		= 12
	VY	  		= 16
	DEST_X	  	= 20
	DEST_Y	  	= 24
	EXPLODED  	= 28
	C	  		= 32

	MAX_COL_INDEX 	= 79
	MAX_ROW_INDEX	= 24
	MIN_COL_INDEX	= 0
	MIN_ROW_INDEX	= 0

	Y_COORD_OFFSET = 80 # used to calculate video memory
	NUM_CHARACTERS = 5 # number of characters to draw for the base
	MAX_BASE_ARRAY_INDEX = 2	# the max index number when traversing base_alive
	SUB_SQUARE_BITS = 16	# Shift 16 high bits, for video memory extraction
	BYTES_4			= 4	# to pop off 1 argument from that stack after calling a function
	BYTES_8			= 8 # to pop off 2 arguments from the stack after calling a function
	BYTES_12		= 12 # to pop off 3 arugments from the stack after calling a function
	EXPLOSION = 64 # Character to draw for an explosion - '@'
	CROSSHAIR = 43 # character to draw for crosshair '+'
	ARG_1	= 8 # used to get the first arugment in a function call
	ARG_2	= 12 # used to get the second argument in a function call
	VIDEO_MEMORY_X_FACOTR	= 2 # multiplied by x to calculate the video memory offset
	VIDEO_MEMORY_Y_FACOTR	= 160 # multiplied by y to calc the video memory offset
	VIDEO_MEMORY_MAX = 3998 # the maximum video memory value, used to decide whether to call poke or not
	ERASE_CHAR 	= 32 # the space character, used to erase missiles and crosshairs
	BASE_PIC_SIZE = 5 # the size of our base pic, used in a loop to draw each char
	CROSSHAIR_START_X = 40 # sets the starting x coord of crosshairs
	CROSSHAIR_START_Y = 12 # sets the starting y coord of crosshairs
	MISSILE_SIZE = 33 # number of bytes in each missile
	SHIFT_DELTA_X_Y = 16 # used to shift over the delta values in movecrosshairs
	CLEAR_HIGH_BITS = 16 # used to clear the high 16 bits, removing negatives


# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs

.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
	# stack frame
	pushl %ebp
	movl %esp, %ebp

	# CALLEE SAVED REGISTERS
	pushl %ebx
	pushl %esi
	pushl %edi
	


	CALL update_missiles
	CALL redraw_cities_crosshairs



	# CALLEE RESTORE REGISTERS
	popl %edi
	popl %esi
	popl %ebx

	popl %ebp
	ret





# ----------------- Functions private to this file -------------------



# NAME: 
# INPUT: 
# OUTPUT: 
# DESCRIPTION: 
update_missiles:

	movl mp1_missile_list, %eax	# eax = cur_missile
	xorl %esi, %esi				# esi = prev_missile (null for now)

								# esi is also a general use register
								# each time it is used, it is first
								# saved on the stack, then restored

	

	cmpl $0, %eax
	je return_tasklet

	iterate_list_loop: 			# iterates through all the missiles
	

	xorl %edx, %edx 			# edx = num of times missile_explode returned non-zero
	movl X(%eax), %ebx			# ebx = old_x
	movl Y(%eax), %ecx			# ecx = old_y
	shrl $SUB_SQUARE_BITS, %ebx	# to get the video screen loc
	shrl $SUB_SQUARE_BITS, %ecx	# to get the video screen loc

	# local vars for specific missile
	# X					= X(%eax)
	# Y					= Y(%eax)
	# velocity X 		= VX(%eax)	
	# velocity Y 		= VY(%eax)
	# dest X 			= DEST_X(%eax)
	# dest Y 			= DEST_Y(%eax)
	# exploded 			= EXPLODED(%eax)
	# draw character 	= C(%eax)


	# #if missile not exploding, update x,y and deal with offscreen
	cmpl $0, EXPLODED(%eax)
	jne is_exploding

	pushl %esi	# save prev_missile ptr

	# update x and y positions
	movl X(%eax), %edi	# current x in edi
	movl Y(%eax), %esi	# current y in esi

	addl VX(%eax), %edi	# add vel_x to x
	addl VY(%eax), %esi # add vel_y to y

	movl %edi, X(%eax)	# move new x pos in struc
	movl %esi, Y(%eax)	# move new y pos in struc

	# restore prev_missile ptr
	popl %esi

	# check for offscreen
	cmpl $MIN_COL_INDEX, %ebx
	jl erase_remove_free

	cmpl $MAX_COL_INDEX, %ebx
	jg erase_remove_free

	cmpl $MIN_ROW_INDEX, %ecx
	jl erase_remove_free

	cmpl $MAX_ROW_INDEX, %ecx
	jg erase_remove_free

	# save prev_missile ptr
	pushl %esi

	# check if reached destination
	movl DEST_X(%eax), %edi
	movl DEST_Y(%eax), %esi
	xorl %ebx, %edi			# if x = dest_x then edi = 0
	xorl %ecx, %esi			# if y = dest_y then esi = 0

	orl %edi, %esi			# if either edi or esi has a 1, then esi has a 1
	cmpl $0, %esi			# if x or y are not at dest, then esi is not 0, otherwise esi is 0

	# restore prev_missile ptr
	popl %esi

	je is_exploding			# if both x and y are at their dest, the missile is exploding

	
	# save prev_missile ptr
	pushl %esi

	# check if screen position has changed
	movl X(%eax), %edi
	movl Y(%eax), %esi
	xorl %ebx, %edi			# If x = old_x then edi = 0
	xorl %ecx, %esi			# If y = old_y then esi = 0

	orl %edi, %esi			# if either edi or esi has a 1, then esi has a 1
	cmpl $0, %esi			# if x or y are changed, then esi is not 0, otherwise esi is 0

	# restore prev_missile ptr
	popl %esi

	je next_missile # if they didn't change, skip the redrawing



	erase_redraw:
		# ERASE

		# save changing registers
		pushl %eax
		pushl %ecx
		pushl %edx
		pushl %esi
		pushfl

		# calculate video offset
		movl %ebx, %edi		# old x into edi
		movl %ecx, %esi		# old y into esi

		imull $VIDEO_MEMORY_X_FACOTR,%edi		# multiply x_old*2
		imull $VIDEO_MEMORY_Y_FACOTR,%esi		# multiply y_old*2*80
		addl %edi, %esi		# add x_old*2+y_old*2*80 to get the video offset
		
		movl %esi, %eax		# give offset to mp1_poke
		movb $ERASE_CHAR, %cl	# replace video memory with space (0x20 or 32) to erase

		# to stop random segfaults in mp1_poke
		cmpl $VIDEO_MEMORY_MAX, %eax 
		jg skip_poke_3

		CALL mp1_poke

		skip_poke_3:

		# RESTORE CHANGING REGISTERS
		popfl
		popl %esi
		popl %edx
		popl %ecx
		popl %eax


		# save changing registers
		pushl %eax
		pushl %ecx
		pushl %edx
		pushl %esi
		pushfl

		# REDRAW

		movl X(%eax), %edi	# moving the current x pos into edi
		movl Y(%eax), %esi	# moving the current y pos into esi
		shrl $SUB_SQUARE_BITS, %edi	# shift to get the x grid coord
		shrl $SUB_SQUARE_BITS, %esi	# shift to get the y grid coord



		# calculate video offset
		imull $VIDEO_MEMORY_X_FACOTR,%edi		# multiply x*2
		imull $VIDEO_MEMORY_Y_FACOTR,%esi		# multiply y*2
		addl %edi,%esi		# add x*2+y*2*80 to get the video offset

		leal C, %edx
		addl %eax, %edx
		movl (%edx), %edx
		movb %dl, %cl # replace video memory with the char to draw

		movl %esi, %eax		# give offset to mp1_poke

		# to stop random segfaults in mp1_poke
		# 3998 is the maximum video memory address, don't draw
		# any larger than this
		cmpl $VIDEO_MEMORY_MAX, %eax 
		jg skip_poke_2

		CALL mp1_poke

		skip_poke_2:

		# restore changing registers
		popfl
		popl %esi
		popl %edx
		popl %ecx
		popl %eax


	next_missile:


	cmpl $0, (%eax)				# if cur_missile->next == NULL
	je leave_the_loop			# then there are no more missiles so we leave the loop

	movl %eax,%esi				# prev_missile = cur_missile
	movl (%eax),%eax			# cur_missile = cur_missile->next
	jmp iterate_list_loop		# iterate another loop for the next missile

	leave_the_loop:

	cmpl $0,%edx				# if edx > 0, then game state changed at least once
	je return_tasklet			# so we must call mp1_notify_user once

	# CALLER SAVED REGISTERS
	pushl %eax
	pushl %ecx
	pushl %edx
	pushfl

	CALL mp1_notify_user		

	# RESTORE CALLER SAVED REGISTERS
	popfl
	pop %edx
	pop %ecx
	pop %eax

	return_tasklet:
	ret



	is_exploding:
	# the missile is exploding or has reached dest
	#	1) call missile_explode 
	#	2) decrement 'exploded' field
	#	3) removes missile if exploded is 0 
	#	4) draws missile if exploded is not 0

	# CALLER SAVED REGISTERS
	pushl %eax
	pushl %ecx
	pushl %edx

	pushl %eax 				# push arugment (cur_missile pointer)
	CALL missile_explode 	# call missile_explode
	addl $BYTES_4, %esp			# pop arugment (cur_missile pointer)

	cmpl $0, %eax			# if zero, then game state has not changed


	# RESTORE CALLER SAVED REGISTERS
	popl %edx
	popl %ecx
	popl %eax



	je no_gamestate_change

	incl %edx				# inc edx to keep track of number of game state changes

	no_gamestate_change:


	movb $EXPLOSION, C(%eax)
	decl EXPLODED(%eax)		# decrement the 'exploded' field
	cmpl $0, EXPLODED(%eax)	# see if 'exploded' has reached zero

	jg erase_redraw			# if it has not, just erase and redraw
							# otherwise erase, remove, and free

	erase_remove_free:
	
	# PART 1: ERASE THE MISSILE
	# save changing registers
	pushl %eax
	pushl %ecx
	pushl %edx
	pushl %esi
	pushfl

	# move y_old into esi for video memory calculation
	movl %ecx, %esi

	# calculate video offset
	imull $VIDEO_MEMORY_X_FACOTR,%ebx,%edi	# multiply x_old*2
	imull $VIDEO_MEMORY_Y_FACOTR,%esi		# multiply y_old*2*80
	addl %edi, %esi		# add x_old*2+y_old*2*80 to get the video offset
		
	movl %esi, %eax		# give offset to mp1_poke
	movb $ERASE_CHAR, %cl		# replace video memory with space (0x20 or 32) to erase

	# to stop random segfaults in mp1_poke
	cmpl $VIDEO_MEMORY_MAX, %eax 
	jg skip_poke_1

	CALL mp1_poke

	skip_poke_1:

	# restore changing registers
	popfl
	popl %esi
	popl %edx
	popl %ecx
	popl %eax



	# PART 2: REMOVE THE MISSLE FROM LINKED LIST
	# CALLER SAVED REGISTERS
	pushl %eax
	pushl %ecx
	pushl %edx
	pushfl

	# arguments
	push %esi	# push next_missile ptr
	push %eax	# push cur_missile ptr

	CALL mp1_missile_remove

	# pop arguments off the stack
	addl $BYTES_8, %esp

	# FREE CALLER SAVED REGISTERS
	popfl
	popl %edx
	popl %ecx
	popl %eax



	# PART 3: FREE THE MISSILE FROM MEMORY
	# CALLER SAVED REGISTERS
	pushl %eax
	pushl %ecx
	pushl %edx
	pushfl

	push %eax	# push pointer to current missile

	CALL mp1_free

	addl $BYTES_4, %esp 	# pop argument from stack

	# FREE CALLER SAVED REGISTERS
	popfl
	popl %edx
	popl %ecx
	popl %eax

	movl mp1_missile_list, %eax

	jmp next_missile










# NAME: 
# INPUT: 
# OUTPUT: 
# DESCRIPTION: 
redraw_cities_crosshairs:

	# %edi						- index into base alive_array
	# (%eax, %edi) 				- base_alive entry at index %edi
	# base_pic					- string to draw for live base
	# dead_base_pic				- string to draw for dead base


	# CALLEE SAVED REGISTERS
	pushl %ebx
	pushl %esi
	pushl %edi

	# PART 1: DRAW CITIES
	xorl %edi, %edi			# index into base_alive array
	movl $base_alive, %eax	# current base alive pointer

	cities_loop:

	# calculate location using loc = 20*(i+1)-2
	# this will give us the starting x-coord for each base (18,38,58)
	movl %edi, %esi			
	addl $1, %esi
	imull $20, %esi	 		
	subl $2, %esi


	# CALLER SAVED REGISTERS
	pushl %eax
	pushl %ecx
	pushl %edx

	movl %edi, %ecx		# for math with index
	addl %eax, %ecx		# ptr+indexoffset

	pushl %esi			# starting x-coord
	pushl %ecx			# value that determine dead or alive (0=dead)

	CALL draw_city

	# pop off arugments
	addl $BYTES_8, %esp

	# RESTORE CALLER SAVED REGISTERS
	popl %edx
	popl %ecx
	popl %eax
	

	# increment index, check if still in bounds of array
	addl $1, %edi
	cmpl $MAX_BASE_ARRAY_INDEX, %edi
	jle cities_loop

	



	# PART 2: DRAW CROSSHAIRS
	# crosshairs_x -> %ebx 		x coord
	# crosshairs_y -> %eax 		y coord
	# CROSSHAIR -> %cl			symbol to draw for crosshair

	
	# calculate memory offset
	# formula: 2x+160*y
	movl crosshairs_x, %ebx
	movl crosshairs_y, %eax
	imull $VIDEO_MEMORY_X_FACOTR, %ebx
	imull $VIDEO_MEMORY_Y_FACOTR, %eax
	addl %ebx, %eax

	# character to draw
	leal CROSSHAIR, %ebx
	movb %bl, %cl

	CALL mp1_poke


	# CALLEE RESTORE REGISTERS
	popl %edi
	popl %esi
	popl %ebx

	ret









# NAME: 
# INPUT: 
# OUTPUT: 
# DESCRIPTION: 
draw_city:
	# stack frame
	pushl %ebp
	movl %esp, %ebp
	
	# CALLEE SAVED REGISTERS
	pushl %ebx
	pushl %esi
	pushl %edi

	# %ebx -			Value that determines dead or alive (0=dead)
	# %ebp+12 - 		Starting X coord
	# MAX_ROW_INDEX	-	24, lowest cell has y coord index	
	# NUM_CHARACTERS - 	5, number of chars to draw each base
	# %edi -			index counter
	# %ecx - 			current x coord

	movl ARG_1(%ebp), %ebx
	movl (%ebx), %ebx
	movl ARG_2(%ebp), %ecx
	xorl %edi, %edi


	draw_loop:

	# calculate video memory offset
	# formula: 2(x+80*y)
	movl $MAX_ROW_INDEX, %edx			# y coord
	movl %ecx, %eax						# x coord
	imull $VIDEO_MEMORY_Y_FACOTR, %edx	# multiply 2*80*y -> y
	imull $VIDEO_MEMORY_X_FACOTR, %eax	# multiply 2*x -> x
	addl %edx, %eax				# eax <-- 2(x+80*y)

	pushl %ecx	# save the x location so we can call mp1_poke

	cmpb $0, %bl	 			# if base is dead draw dead base
	je draw_dead
	
	# put in char in cl
	leal base_pic, %edx
	movb (%edi,%edx), %cl
	jmp skip_draw_dead

	draw_dead:
	# put in char in cl
	leal dead_base_pic, %edx
	movb (%edi,%edx), %cl

	skip_draw_dead:
	CALL mp1_poke

	popl %ecx	# restore x so we have an x location

	# increment counter and x coord register
	addl $1, %edi
	addl $1, %ecx

	# check if have another char to print
	cmpl $BASE_PIC_SIZE, %edi
	jl draw_loop


	# CALLEE RESTORE REGISTERS
	popl %edi
	popl %esi
	popl %ebx

	# stack teardown + return
	popl %ebp
	ret



jump_table:
	.long mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame




# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
	pushl %ebp
	movl %esp, %ebp

	movl ARG_2(%ebp), %ebx		# ebx = cmd


	cmpl  $4, %ebx 			# if %ebx is above 4, then cmd is an invalid number
	ja return_negative_one


	jmp *jump_table(,%ebx,4)
	
	jmp return_ioctl


	return_negative_one:
	movl $-1, %eax

	return_ioctl:

	# stack frame teardown
	popl %ebp
	ret




# NAME: 
# INPUT: ---
# OUTPUT: ---
# DESCRIPTION: Initializes base_alive = 0xAAAAAA00, 
# (crosshair_x, crosshair_y) = (40,12)
mp1_ioctl_startgame:

	# CALLEE SAVED REGISTERS
	pushl %ebx
	pushl %edi
	pushl %esi

	# fill first 3 bytes of base_alive with non-zero value
	# I arbitrarily chose 0x00AAAAAA
	movl $0x00AAAAAA, %esi # sets top bits to non-zero in base_alive
	movl %esi,  base_alive

	# set crosshair_x to 40, the middle of the screen, as required 
	movl $CROSSHAIR_START_X, %esi
	movl %esi, crosshairs_x

	# set crosshair_y to 12, the middle of the screen, as required
	movl $CROSSHAIR_START_Y, %esi
	movl %esi, crosshairs_y

	# passsed successfully
	movl $0, %eax

	# RESTORE CALLEE SAVED REGISTERS
	popl %esi
	popl %edi
	popl %ebx

	# stack teardown
	popl %ebp
	ret

# NAME: 
# INPUT: 
# OUTPUT: 
# DESCRIPTION: 
mp1_ioctl_addmissile:

	# CALLEE SAVED REGISTERS
	pushl %ebx
	pushl %edi
	pushl %esi

	# %ebx				user_missile
	# %eax				return_copy_value
	# %edx				allocated_ptr
	movl ARG_1(%ebp), %ebx		# gets the user_missile ptr


	# PART 1: MALLOC
	# CALLER SAVED REGISTERS
	pushl %ecx
	pushl %edx

	# push argument to malloc
	pushl $MISSILE_SIZE	# 33 bytes of memory for one missile

	CALL mp1_malloc

	# pop off argument
	addl $BYTES_4, %esp

	# RESTORE CALLER SAVED REGISTERS
	popl %edx
	popl %ecx

	movl %eax, %edx		# moves the allocated_ptr to %edx

	cmpl $0, %edx		# if malloc returned NULL, then error
	je error_addmissile



	# PART 2: COPY TO KERNEL
	# CALLER SAVED REGISTERS
	pushl %ecx
	pushl %edx

	# push arguments to copy_from_user
	pushl $MISSILE_SIZE	# 33 bytes of memory for one missile
	pushl %ebx	# user_missile
	pushl %edx	# allocated_ptr

	CALL mp1_copy_from_user

	# pop off arguments
	addl $BYTES_12, %esp

	# RESTORE CALLER SAVED REGISTERS
	popl %edx
	popl %ecx

	# ;if ret of copy from user is nonzero, then error
	cmpl $0, %eax
	jne error_addmissile


	# PART 3: ADD TO LINKED LIST
	# make new missile point to head of linked list
	# user_missile: %ebx
	# allocated_ptr: %edx
	pushl %esi						# need esi to help

	cmpl $0, mp1_missile_list
	je add_to_null_list


	movl mp1_missile_list, %esi
	movl %esi, (%edx)
	movl %edx, mp1_missile_list
	jmp done_adding_to_list

	add_to_null_list:
	movl $0, (%edx)
	movl %edx, mp1_missile_list


	done_adding_to_list:
	popl %esi						# needed esi to help

	movl $0, %eax
	jmp return_addmissile


	error_addmissile:

	# deallocate ptr (%edx)
	pushl %edx 
	CALL mp1_free
	addl $BYTES_4, %esp

	# return -1, as required by documentation for an error
	movl $-1, %eax


	return_addmissile:

	# RESTORE CALLEE SAVED REGISTERS
	popl %esi
	popl %edi
	popl %ebx

	# stack teardown
	popl %ebp
	ret

# NAME: 
# INPUT: 
# OUTPUT: 
# DESCRIPTION: 
mp1_ioctl_movexhairs:

	# CALLEE SAVED REGISTERS
	pushl %ebx
	pushl %edi
	pushl %esi

	# making variables
	movl ARG_1(%ebp), %ebx	# 32-bit number to ebx
	xorl %ecx, %ecx		# clear ecx
	movw %bx, %cx		# lower ebx to lower ecx
	shrl $SHIFT_DELTA_X_Y, %ebx		# shift higher bits ebx into lower bits ebx

	# getting current location
	movl crosshairs_x, %eax
	movl crosshairs_y, %edx

	# %ecx: delta_x
	# %ebx: delta_y
	# %eax: cur_x
	# %edx: cur_y

	# updating X movement

	# C: if cur_x == 0, jmp to left edge case
	cmpw $MIN_COL_INDEX, %ax
	je x_left_border

	# C: if cur_x < 0, jmp to outside left side frame case
	jl x_outside_left

	# C: if cur_x == 79, jmp to right edge case
	cmpw $MAX_COL_INDEX, %ax
	je x_right_border

	# C: if cur_x > 79, jmp to outside right side frame case
	jg x_outside_right

	# C: if cur_x > 0 AND cur_x < 79, jmp to normal execution
	jmp x_normal_movement

	# C: if cur_x == 0, and delta_x <= 0, jmp to NO movement case
	x_left_border:
	cmpw $0, %cx
	jle x_no_movement

	# C: if curr_x == 0, and delta_x > 0, jmp to normal execution
	jmp x_normal_movement

	# C: if curr_x == 79, and delta_x >= 0, jmp to NO movement case
	x_right_border:
	cmpw $0, %cx
	jge x_no_movement

	# C: if curr_x == 79, and delta_x < 0, jmp to normal execution
	jmp x_normal_movement

	# C: if curr_x < 0, set crosshair_x = 0, curr_x = 0, delta_x = 0
	x_outside_left:
	movl $0, crosshairs_x
	movl $0, %eax
	movl $0, %ecx
	jmp update_y_crosshair

	# C: if curr_x > 79, set crosshair_x = 79, curr_x = 79, delta_x = 0
	x_outside_right:
	movl $MAX_COL_INDEX, crosshairs_x
	movl $MAX_COL_INDEX, %eax
	movl $0, %ecx
	jmp update_y_crosshair

	# C: if curr_x < 79 or curr_x > 0 or 
	#		curr_x == 79 AND delta_x < 0 or
	#		curr_x == 0 AND delta_x > 0
	x_normal_movement:
	addl %eax, %ecx
	shll $CLEAR_HIGH_BITS, %ecx
	shrl $CLEAR_HIGH_BITS, %ecx
	movl %ecx, crosshairs_x
	jmp update_y_crosshair

	# C: curr_x == 79, and delta_x >= 0 or
	# 	 curr_x == 0, and delta_x <= 0
	x_no_movement:
	movl %eax, %ecx
	movl %ecx, crosshairs_x



	# C: if curr_y == 0, go to top border case
	update_y_crosshair:
	cmpw $MIN_ROW_INDEX, %dx
	je y_top_border

	# C: if curr_y == 24, go to bottom border case
	cmpw $MAX_ROW_INDEX, %dx
	je y_bottom_border

	# C: if curr_y != 0 AND curr_y != 24, go to normal execution
	jmp y_normal_movement

	# C: curr_y == 0 AND delta_y <= 0, jmp to no movement case
	y_top_border:
	cmpw $0, %bx
	jle y_no_movement

	# C: if curr_y == 0 AND delta_y > 0, jmp to normal execution
	jmp y_normal_movement

	# C: if curr_y == 24 AND delta_y >= 0, jmp to no movement case
	y_bottom_border:
	cmpw $0, %bx
	jge y_no_movement

	# C: if curr_y == 24 AND delta_y < 0, jmp to normal exeuction
	jmp y_normal_movement

	# C: if curr_y != 24 or curr_y != 0 or
	# 	 	curr_y == 0 AND delta_y > 0 or
	# 		curr_y == 24 AND delta_y < 0
	y_normal_movement:
	addl %edx, %ebx
	shll $CLEAR_HIGH_BITS, %ebx
	shrl $CLEAR_HIGH_BITS, %ebx
	movl %ebx, crosshairs_y
	jmp erase_old_crosshair

	y_no_movement:
	movl %edx, %ebx


	

	erase_old_crosshair:
	# ERASE OLD

	# %eax: old_x --> video offset
	# %edx: old_y
	# %cl: character to draw

	# CALLER SAVED REGISTERS
	pushl %eax
	pushl %ecx
	pushl %edx

	# calculate video memory
	# formula: 2*x+160*y
	imull $VIDEO_MEMORY_X_FACOTR, %eax
	imull $VIDEO_MEMORY_Y_FACOTR, %edx
	addl %edx, %eax

	movb $ERASE_CHAR, %cl	# put in SPACE (0x20) to erase character

	CALL mp1_poke
	
	# RESTORE CALLER SAVED REGISTERS
	popl %edx
	popl %ecx
	popl %eax



	# DRAW NEW

	# %ecx: new_x
	# %ebx: new_y
	# %eax: video_offset
	# %cl: character to draw

	# CALLER SAVED REGISTERS
	pushl %eax
	pushl %ecx
	pushl %edx

	# calculate video memory
	# formula: 2*x+160*y
	imull $VIDEO_MEMORY_X_FACOTR, %ecx
	imull $VIDEO_MEMORY_Y_FACOTR, %ebx
	addl %ecx, %ebx

	movl %ebx, %eax	# video memory offset in eax
	movb $CROSSHAIR, %cl	# put in + to draw crosshairs

	CALL mp1_poke
	
	# RESTORE CALLER SAVED REGISTERS
	popl %edx
	popl %ecx
	popl %eax




	movl $0, %eax

	# RESTORE CALLEE SAVED REGISTERS
	popl %esi
	popl %edi
	popl %ebx

	# stack teardown
	popl %ebp
	ret





# NAME: 
# INPUT: 
# OUTPUT: 
# DESCRIPTION: 
mp1_ioctl_getstatus:
	# %ebx			write data here
	# %edx			ptr in kernel memory
	# %edi			loop counter
	# %ecx			base_alive ptr


	# CALLEE SAVED REGISTERS
	pushl %ebx
	pushl %edi
	pushl %esi


	# STEP 1: CALL MALLOC
	# CALLER SAVED REGISTERS
	pushl %ecx
	pushl %edx

	# arguement to malloc, allocate 4 bytes because a pointer is 4 bytes
	pushl $4

	CALL malloc

	# pop arguments off stack
	addl $BYTES_4, %esp

	# RESTORE CALLER SAVED REGISTERS
	popl %edx
	popl %ecx

	# so we can use eax as our return value
	movl %eax, %edx

	# ;if malloc returns null pointer, return -1
	cmpl $0, %edx
	je error_getstatus


	# STEP 2: ADD BASE_ALIVE AND SCORE VALUES TO EBX
	# clear our counter + register to write to and get our base_alive pointer
	xorl %edi, %edi
	xorl %ebx, %ebx
	movl $base_alive, %ecx

	iterate_base_alive_loop:
	# ;if base_alive = 0, then the base is dead, don't add
	cmpb $0, (%ecx, %edi)
	je skip_add

	add $1, %ebx # this will be the 16th, 17th and 18th bits

	skip_add:

	shll $1, %ebx	# shifts to the left so we get each bit individually

	# inc counter and see if we are still in bounds of the base_alive array
	addl $1, %edi
	cmpl $MAX_BASE_ARRAY_INDEX, %edi
	jle iterate_base_alive_loop


	# PART 3: CALL COPY TO USER
	# shift the remaining bits to their correct locations
	# ebx has been shifted 4 times already, placing the MSB at index 3
	# to get to the correct location, 3+15=16
	shll $15, %ebx
	
	# moving the score into the lower 16 bits
	movw mp1_score, %bx 

	movl %ebx, (%edx)	# moves the values into memory

	# CALLER SAVED REGISTERS
	pushl %ecx
	pushl %edx

	# user status pointer in ecx
	movl ARG_1(%ebp), %ecx	

	# push 4 bytes to be copied, from %ebx, to 8(%ebp)
	pushl $BYTES_4
	pushl %edx
	pushl %ecx

	CALL mp1_copy_to_user

	# pop off arguments
	addl $BYTES_12, %esp

	# RESTORE CALLER SAVED REGISTERS
	popl %edx
	popl %ecx

	cmpl $0, %eax
	jne error_getstatus

	# return 0
	movl $0, %eax
	jmp return_getstatus

	error_getstatus:

	# deallocate ptr (%edx)
	pushl %edx 
	CALL mp1_free
	# pop off argument
	addl $BYTES_4, %esp

	# return -1, as required by the docs when error occurs
	movl $-1, %eax

	return_getstatus:

	# RESTORE CALLEE SAVED REGISTERS
	popl %esi
	popl %edi
	popl %ebx

	# stack teardown
	popl %ebp	
	
	ret


# NAME: 
# INPUT: 
# OUTPUT: 
# DESCRIPTION: 
mp1_ioctl_endgame:
	
	cmpl $0, mp1_missile_list
	je endgame_return
	
	movl mp1_missile_list, %eax
	movl $0, mp1_missile_list
	
	free_loop:
	movl (%eax), %ebx


	# CALLER SAVED REGISTERS
	pushl %eax
	pushl %ecx
	pushl %edx

	# push the missile to be freed
	pushl %eax

	CALL mp1_free

	# pop off argument
	addl $BYTES_4, %esp

	# RESTORE CALLER SAVED REG
	popl %edx
	popl %ecx
	popl %eax

	# cur_missile = cur_missile->next
	movl %ebx, %eax
	cmpl $0, %eax
	jne free_loop

	
	endgame_return:
	# stack frame teardown
	popl %ebp
	ret


# NAME: 
# INPUT: 
# OUTPUT: 
# DESCRIPTION: 
mp1_missile_remove:
	pushl %ebp
	movl %esp, %ebp

	# CALLEE SAVED REGISTERS
	pushl %ebx
	pushl %edi
	pushl %esi
	
	
	# remove from list
	movl ARG_1(%ebp), %eax	# cur_missile ptr
	movl ARG_2(%ebp), %esi	# prev_missile ptr

	movl (%eax), %edi  # cur_missile = cur_missile->next

	# check if previous pointer is null, if so, deal with that
	cmpl $0, %esi
	je prev_ptr_is_null

	movl %edi,(%esi)	# prev_missile->next = cur_missile->next
	jmp missile_remove_return

	prev_ptr_is_null:
	movl %edi, mp1_missile_list  # point beginning of list to cur_missil->next



	missile_remove_return:

	# CALLEE SAVED REGISTERS
	popl %esi
	popl %edi
	popl %ebx

	# stack teardown
	popl %ebp
	ret

