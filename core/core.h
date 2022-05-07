//
// Created by 朕与将军解战袍 on 2022/5/6.
//

#ifndef __COROUTINE_CORE_H__
#define __COROUTINE_CORE_H__

#include "regs.h"

typedef void* (*Func)(void *);
typedef uintmax_t coid_t;

enum __co_status {
    CO_NEW = 1, // 新创建，还未执行过
    CO_RUNNING, // 已经执行过
    CO_WAITING, // 在 co_wait 上等待
    CO_DEAD,    // 已经结束，但还未释放资源
};

struct __co {
    coid_t cid;
    Func entry;
    void *arg;
    void *ret;

    struct __co*     waiters;
    enum __co_status status;
    struct __regs    regs;
    uint8_t*         stack;
    size_t           stack_size;
};
typedef struct __co* Coroutine;

// coroutine core API
Coroutine co_start(Func func, void *arg);
void      co_cancel(Coroutine *p_co);
void*     co_wait(Coroutine *p_co);
void      co_yield();

void   co_set_defualt_stack_size(size_t size);
size_t co_get_defualt_stack_size();
size_t co_get_stack_size(Coroutine co);

#endif // __COROUTINE_CORE_H__
