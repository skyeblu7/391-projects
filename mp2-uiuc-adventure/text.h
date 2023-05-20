/*									tab:8
 *
 * text.h - font data and text to mode X conversion utility header file
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
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
 * Version:	    2
 * Creation Date:   Thu Sep  9 22:08:16 2004
 * Filename:	    text.h
 * History:
 *	SL	1	Thu Sep  9 22:08:16 2004
 *		First written.
 *	SL	2	Sat Sep 12 13:40:11 2009
 *		Integrated original release back into main code base.
 */

#ifndef TEXT_H
#define TEXT_H

/* The default VGA text mode font is 8x16 pixels. */
#define FONT_WIDTH   8
#define FONT_HEIGHT 16

// for sizes of the image and accessing the pixels using mode x memory layout
#define IMAGE_X_DIM     320   
#define IMAGE_Y_DIM     182  
#define SCROLL_Y_DIM    IMAGE_Y_DIM       
#define NUM_PLANES      4 
#define SCROLL_X_WIDTH  (IMAGE_X_DIM / 4)             
#define SCROLL_SIZE   (SCROLL_X_WIDTH * SCROLL_Y_DIM)

// constants for easy buffer indexing
#define CHARACTER_OFFSET 2
#define VERTICAL_OFFSET 1
#define ROW_LENGTH 18
#define MAX_CHARACTERS 40

/* Standard VGA text font. */
extern unsigned char font_data[256][16];

// paints the color and adds the correct ascii characters to the status bar
// buff is the buffer to be written to the status bar
// status_message is the status message to be written ("\0" if there isn't one)
// room is the string version of the room the player is currently in
// typed_input is the string of any typed command the player typed
void create_status_bar(unsigned char* buff, char* status_message, const char* room, const char* typed_input);

#endif /* TEXT_H */
