#include <stdlib.h>
#include "coroutine.h"
#include "regs.h"

#define STACK_SIZE 4096 * 1024

static __attribute__ ((noinline)) void stack_switch_call(void *csp, Func entry, void *arg);
static __attribute__ ((noinline)) void co_yield_noinlne();
static void __attribute__ ((noinline)) co_yield_impl_switch();

enum co_status {
    CO_NEW = 1, // 新创建，还未执行过
    CO_RUNNING, // 已经执行过
    CO_WAITING, // 在 co_wait 上等待
    CO_DEAD,    // 已经结束，但还未释放资源
};
struct co {
    const char* name;
    Func func;
    void *arg;
    void *ret;

    struct co* waiters;
    enum co_status status;
    struct  regs regs;
    uint8_t* stack;
};
static struct co *current = NULL;

struct co *co_start(const char *name, Func func, void *arg) {
    if (current == NULL) {
        current = (struct co*)malloc(sizeof(struct co));
        current->name = "main";
        current->status = CO_RUNNING;
        current->waiters = current; // 成环
    }
    struct co *coroutine = (struct co*) malloc(sizeof(struct co));
    coroutine->name = name;
    coroutine->func = func;
    coroutine->arg = arg;
    coroutine->stack = (uint8_t*) malloc(sizeof(uint8_t) * STACK_SIZE);
    coroutine->stack = &coroutine->stack[STACK_SIZE - 1];
    coroutine->status = CO_NEW;
    
    struct co* waiters = current->waiters;
    current->waiters = coroutine;
    coroutine->waiters = waiters;

    co_yield();

    return coroutine;
}

void* co_wait(struct co *co) {
    while (co->status != CO_DEAD) {
        co_yield();
    }
    return co->ret;
}

static __attribute__ ((noinline)) void co_yield_noinlne() {
    // 进入等待状态, 注意不要让死人复活(手动狗头)
    if (current->status != CO_DEAD) current->status = CO_WAITING;
    // save registers
#if __x86_64__ 
    REGS_SAVE(current->regs, rbx);
    REGS_SAVE(current->regs, rsp);
    REGS_SAVE(current->regs, rbp);
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
    REGS_SAVE(current->regs, edi);
    REGS_SAVE(current->regs, esi);
    REGS_SAVE(current->regs, ebp);
    REGS_SAVE(current->regs, esp);
    asm volatile (
        "jmp .save_ip_to_stack\n .load_ip_to_regs:\n pop %0" 
        :"=r"(current->regs.eip)
    );
#endif
    co_yield_impl_switch();
    asm volatile ("jmp .yeild_return");

    asm volatile (
        ".save_ip_to_stack:\n call .load_ip_to_regs\n .yeild_return:\n" 
        :   :   :   "memory"
    );

    return ;
}

void co_yield() {
    co_yield_noinlne();
}

static __attribute__ ((noinline)) void stack_switch_call(void *csp, Func entry, void* arg) {
#if __x86_64__
    asm volatile ("movq %0, %%rsp\n" ::"r"((uintptr_t)csp));
    void *ret = entry(arg);
    current->ret = ret;
#else
    asm volatile ("movl %0, %%esp\n" ::"r"((uintptr_t)csp));
    void *ret = entry(arg);
    current->ret = ret;
#endif
    current->status = CO_DEAD;
    co_yield();
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
        stack_switch_call(current->stack, current->func, current->arg);
        break;
    case CO_WAITING:
        current->status = CO_RUNNING;
        // 保存寄存器
#if __x86_64__
        REGS_LOAD(current->regs, rsp);
        REGS_LOAD(current->regs, r12);
        REGS_LOAD(current->regs, r13);
        REGS_LOAD(current->regs, r14);
        REGS_LOAD(current->regs, r15);
        REGS_LOAD(current->regs, rbp);
        REG_LOAD(rcx, current->regs.rip);
        asm volatile ("push %rcx");
        REGS_LOAD(current->regs, rbx);
        asm volatile ("ret");
#else
        REGS_LOAD(current->regs, esp);
        REGS_LOAD(current->regs, ebp);
        REGS_LOAD(current->regs, edi);
        REGS_LOAD(current->regs, esi);
        REG_LOAD(ecx, current->regs.eip);
        asm volatile ("push %ecx");
        REGS_LOAD(current->regs, ebx);
        asm volatile ("ret");
#endif
        break;
    default:
        break;
    }
}