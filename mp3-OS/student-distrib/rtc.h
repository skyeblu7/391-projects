
#ifndef _RTC_H_
#define _RTC_H_

#define RTC_IRQ 8
#define REGISTER_A 0x8A
#define REGISTER_B 0x8B
#define REGISTER_C 0x0C
#define RTC_INDEX 0x70
#define RTC_CMOS 0x71
#define DEFAULT_RATE 0x06 // rate for 1024 Hz
#define MAX_FREQ 512

#include "types.h"
// initializes the rtc
void rtc_init(void);

/* RTC Interrupt Handler */
void handle_rtc();

int32_t rtc_open(int32_t fd_t);
int32_t rtc_read(int32_t fd, uint8_t* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, uint8_t* buf, int32_t nbytes);
int32_t rtc_close(int32_t fd);

fops_t rtc_fops;

#endif
