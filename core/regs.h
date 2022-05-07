//
// Created by 朕与将军解战袍 on 2022/5/6.
//

#ifndef __COROUTINE_REGS_H__
#define __COROUTINE_REGS_H__
#include <stdint.h>

#define REG_SAVE(REG, VAL) asm volatile ("mov %%" #REG ", %0" :"=r"(VAL) :: "memory")
#define REG_LOAD(REG, VAL) asm volatile ("mov %0, %%" #REG    ::"r"(VAL)  : "memory")

#define REGS_SAVE(REGS, REG) REG_SAVE(REG, REGS.REG)
#define REGS_LOAD(REGS, REG) REG_LOAD(REG, REGS.REG)

struct __regs {
#if __x86_64__
    volatile uint64_t rsp;
    volatile uint64_t rip;
    // callee-saved
    volatile uint64_t rbx;
    volatile uint64_t rbp;
    volatile uint64_t rdi;
    volatile uint64_t rsi;
    volatile uint64_t r12;
    volatile uint64_t r13;
    volatile uint64_t r14;
    volatile uint64_t r15;
#else
    volatile uint32_t esp;
    volatile uint32_t eip;
    // callee-saved
    volatile uint32_t ebx;
    volatile uint32_t edi;
    volatile uint32_t esi;
    volatile uint32_t ebp;
#endif
};

#endif //__COROUTINE_REGS_H__