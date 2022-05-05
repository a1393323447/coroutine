#include <stdint.h>

#define REG_SAVE(REG, VAL) asm volatile ("mov %%" #REG ", %0" :"=r"(VAL))
#define REG_LOAD(REG, VAL) asm volatile ("mov %0, %%" #REG    ::"r"(VAL))

#define REGS_SAVE(REGS, REG) REG_SAVE(REG, REGS.REG)
#define REGS_LOAD(REGS, REG) REG_LOAD(REG, REGS.REG)

struct regs {
#if __x86_64__
    volatile uint64_t rbx;
    volatile uint64_t rsp;
    volatile uint64_t rbp;
    volatile uint64_t rip;
    volatile uint64_t r12;
    volatile uint64_t r13;
    volatile uint64_t r14;
    volatile uint64_t r15;
#else
    volatile uint32_t ebx;
    volatile uint32_t ebp;
    volatile uint32_t esp;
    volatile uint32_t eip;
#endif
};

