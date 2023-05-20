/*									tab:8
 *
 * input.h - header file for input control to maze game
 *
 * "Copyright (c) 2004-2011 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:	    Steve Lumetta
 * Version:	    3
 * Creation Date:   Thu Sep  9 22:22:00 2004
 * Filename:	    input.h
 * History:
 *	SL	1	Thu Sep  9 22:22:00 2004
 *		First written.
 *	SL	2	Sun Sep 13 04:11:44 2009
 *		Changed display interface for Tux controller.
 *	SL	3	Wed Sep 14 02:06:59 2011
 *		Updated command names and numbers for adventure game.
 */

#ifndef INPUT_H
#define INPUT_H

/* possible commands from input device, whether keyboard or game controller */
typedef enum {
    CMD_NONE, CMD_RIGHT, CMD_LEFT, CMD_UP, CMD_DOWN,
    CMD_MOVE_LEFT, CMD_ENTER, CMD_MOVE_RIGHT,
    CMD_TYPED,
    CMD_QUIT,
    NUM_COMMANDS
} cmd_t;

#define MAX_TYPED_LEN 20
#define RIGHT 0
#define LEFT 1
#define DOWN 2
#define UP 3
#define C 4
#define B 5
#define A 6
#define START 7
#define NUM_BTNS 8
#define ONE_BIT_MASK 0x1

#define SET_DECIMAL 0x4

#define SEC_IN_MIN 60
#define MAX_TIME_DISPLAY 160
#define BASE_10 10

#define SHIFT_24 24
#define SHIFT_16 16
#define SHIFT_12 12
#define SHIFT_8 8
#define SHIFT_4 4

#define ACTIVATE_3_LEDS 0x7
#define ACTIVATE_4_LEDS 0xf

/* Initialize the input device. */
extern int init_input ();

/* Read a command from the keyboard device. */
extern cmd_t get_kb_cmd ();

/* Get currently typed command string. */
extern const char* get_typed_command ();

/* Reset typed command. */
extern void reset_typed_command ();

/* Shut down the input device. */
extern void shutdown_input ();

/*
 * Show the elapsed seconds on the Tux controller (no effect when
 * compiled for a keyboard).
 */
extern void display_time_on_tux (int num_seconds);

// get commands from tux controller
extern cmd_t get_tux_cmd();

#endif /* INPUT_H */
