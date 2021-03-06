//
// Created by 朕与将军解战袍 on 2022/5/6.
//

#include "core.h"
#include <stdlib.h>
#include <stdio.h>

static __attribute__ ((noinline)) void stack_switch_call();
static __attribute__ ((noinline)) void co_yield_noinline();
static __attribute__ ((noinline)) void co_yield_impl_switch();

#ifdef ENABLE_MULTITHREAD
#if __STDC_VERSION__ >= 201112L
_Thread_local
#else
#error "To enable multithread, you need to enable C11"
#endif
#endif
struct __co * current = NULL;

#ifdef ENABLE_MULTITHREAD
#if __STDC_VERSION__ >= 201112L
_Thread_local
#else
#error "To enable multithread, you need to enable C11"
#endif
#else
static
#endif
uintmax_t   next_cid = 0;

#ifdef ENABLE_MULTITHREAD
#if __STDC_VERSION__ >= 201112L
_Thread_local
#else
#error "To enable multithread, you need to enable C11"
#endif
#else
static
#endif
size_t stack_size = 4 * 1024 * 1024;

Coroutine co_start(Func func, void *arg) {
    if (current == NULL) {
        current = (Coroutine)malloc(sizeof(struct __co));
        current->cid = next_cid;
        current->status = CO_RUNNING;
        current->waiters = current; // 成环
        next_cid++;
    }
    Coroutine coroutine = (Coroutine) malloc(sizeof(struct __co));
    coroutine->cid = next_cid;
    next_cid++;
    coroutine->entry = func;
    coroutine->arg = arg;
    coroutine->status = CO_NEW;
    // 保证地址 16 字节对齐, 防止 movaps 指令引发错误:
    // 引用: 64-ia-32-architectures-software-developer-vol-1-manual
    // 10.4.1.1 SSE Data Movement Instructions
    // The MOVAPS (move aligned packed single-precision floating-point values) instruction transfers a double quadword
    // operand containing four packed single-precision floating-point values from memory to an XMM register and vice
    // versa, or between XMM registers. The memory address must be aligned to a 16-byte boundary; otherwise, a
    // general-protection exception (#GP) is generated.
    coroutine->stack = (uint8_t*) malloc(sizeof(uint8_t) * stack_size);
    coroutine->stack = &coroutine->stack[stack_size - 1];
    uintptr_t address = (uintptr_t)coroutine->stack;
    uintptr_t reminder = address % 16;
    coroutine->stack = (uint8_t *)(address - reminder);
    coroutine->stack_size = stack_size - reminder;
    
    Coroutine waiters = current->waiters;
    current->waiters = coroutine;
    coroutine->waiters = waiters;

    co_yield();

    return coroutine;
}

void co_yield() {
    co_yield_noinline();
}

void co_cancel(Coroutine *p_co) {
    if (p_co == NULL) {
        return;
    }
    Coroutine co = *p_co;
    if (co == NULL || co->cid == current->cid) {
        return ;
    }
    Coroutine worker = current;
    Coroutine start = worker;
    while (worker->waiters->cid != co->cid) {
        worker = worker->waiters;
        if (worker->cid == start->cid) {
            goto CO_CANCEL_SET_NULL;
        }
    }
    // 将当前协程踢出任务队列
    worker->waiters = co->waiters;
    // 还原栈指针
    co->stack -= co->stack_size - 1;
    // 释放栈空间
    free(co->stack);
    // 释放 co
    free(co);
    CO_CANCEL_SET_NULL:
    // 将已经 wait 的 coroutine 置为 NULL
    *p_co = NULL;
}

void* co_wait(Coroutine *p_co) {
    Coroutine co = *p_co;
    while (co->status != CO_DEAD) {
        co_yield();
    }
    void *ret = co->ret;

    co_cancel(p_co);

    return ret;
}

void co_set_default_stack_size(size_t size) {
    stack_size = size;
}

size_t co_get_default_stack_size() {
    return stack_size;
}

size_t co_get_stack_size(Coroutine co) {
    return co->stack_size;
}


static __attribute__ ((noinline)) void stack_switch_call() {
#if __x86_64__
    asm volatile ("movq %0, %%rsp\n" ::"r"((uintptr_t)current->stack));
    void *ret = current->entry(current->arg);
    current->ret = ret;
#else
    asm volatile ("movl %0, %%esp\n" ::"r"((uintptr_t)current->stack);
    void *ret = current->entry(current->arg);
    current->ret = ret;
#endif
    current->status = CO_DEAD;
    co_yield_impl_switch();
}

static __attribute__ ((noinline)) void co_yield_noinline() {
    current->status = CO_WAITING;
    // save registers
#if __x86_64__ 
    REGS_SAVE(current->regs, rbx);
    REGS_SAVE(current->regs, rsp);
    REGS_SAVE(current->regs, rbp);
    REGS_SAVE(current->regs, rdi);
    REGS_SAVE(current->regs, rsi);
    REGS_SAVE(current->regs, r12);
    REGS_SAVE(current->regs, r13);
    REGS_SAVE(current->regs, r14);
    REGS_SAVE(current->regs, r15);
    asm volatile (
        "jmp .save_ip_to_stack\n .load_ip_to_regs:\n pop %0" 
        :"=r"(current->regs.rip)
    );
#else
    REGS_SAVE(current->regs, ebx);
    REGS_SAVE(current->regs, esp);
    REGS_SAVE(current->regs, ebp);
    REGS_SAVE(current->regs, edi);
    REGS_SAVE(current->regs, esi);
    asm volatile (
        "jmp .save_ip_to_stack\n .load_ip_to_regs:\n pop %0" 
        :"=r"(current->regs.eip)
    );
#endif
    co_yield_impl_switch();
    asm volatile ("jmp .yield_return");

    asm volatile (
        ".save_ip_to_stack:\n call .load_ip_to_regs\n .yield_return:\n"
        :   :   :   "memory"
    );
}

static __attribute__ ((noinline)) void co_yield_impl_switch() {
    // 在循环队列里面搜索处于 CO_WAITING 状态的任务
    do {
        current = current->waiters;
    } while (current->status != CO_WAITING && current->status != CO_NEW);
    // 搜到后根据状态切换执行
    switch (current->status)
    {
    case CO_NEW:
        current->status = CO_RUNNING;
        stack_switch_call();
        break;
    case CO_WAITING:
        current->status = CO_RUNNING;
        // load registers
#if __x86_64__
        REGS_LOAD(current->regs, rsp);
        REGS_LOAD(current->regs, rdi);
        REGS_LOAD(current->regs, rsi);
        REGS_LOAD(current->regs, r12);
        REGS_LOAD(current->regs, r13);
        REGS_LOAD(current->regs, r14);
        REGS_LOAD(current->regs, r15);
        REGS_LOAD(current->regs, rbp);
        REG_LOAD(rcx, current->regs.rip);
        asm volatile ("push %rcx");
        REGS_LOAD(current->regs, rbx);
#else
        REGS_LOAD(current->regs, esp);
        REGS_LOAD(current->regs, ebp);
        REGS_LOAD(current->regs, edi);
        REGS_LOAD(current->regs, esi);
        REG_LOAD(ecx, current->regs.eip);
        asm volatile ("push %ecx");
        REGS_LOAD(current->regs, ebx);
#endif
        asm volatile("ret");
        break;
    }
}