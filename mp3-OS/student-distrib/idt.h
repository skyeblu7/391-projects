
#ifndef IDT_H
#define IDT_H
#include "x86_desc.h"
#include "lib.h"

#define IRQ0 0x20
#define IRQ1 0x21
#define IRQ8 0x28




/* setup function for the IDT */
void idt_setup();

/* handler functions for the interrupt table vectors 0-19 */
void intr0();
void intr1();
void intr2();
void intr3();
void intr4();
void intr5();
void intr6();
void intr7();
void intr8();
void intr9();
void intr10();
void intr11();
void intr12();
void intr13();
void intr14();
void intr16();
void intr17();
void intr18();
void intr19();
void intr128();

/* function that obtains IRQ descriptor from assembly linkage and calls
 * the appropriate handler
 */
void do_irq();




/* pointers to assembly linkage */
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void sys_call();

#endif

