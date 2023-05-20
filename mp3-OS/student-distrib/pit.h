#ifndef _PIT_H_
#define _PIT_H_


#define PIT_INPUT_CLK 1193180
#define PIT_HZ 30


#define CHANNEL_0 0x40
#define COMMAND_REG 0x43
#define PIT_MODE 0x36
#define PIT_MASK 0xFF
#define PIT_SHIFT 8

/* sets the configeration of the PIT to run 30 times per sec */
void pit_init(void);

/* handles pit interrupts */
void handle_pit();



#endif
