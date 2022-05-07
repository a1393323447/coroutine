//
// Created by 朕与将军解战袍 on 2022/5/7.
//

#include "corwlock.h"
#include "../core/core.h"

#include <stdlib.h>

#define STAT_WRITE  ((int)0xcffc)
#define STAT_READ   ((int)0xdeeb)

#define TICKET_NULL ((coid_t)(~((coid_t)0)))

extern Coroutine current;

struct co_rwlock_core {
    void     *data;
    int       stat;
    uintmax_t reader_cnt;
    coid_t    owner;
};

struct co_rwlock {
    struct co_rwlock_core *core;
    coid_t ticket;
};

CoRwLock co_rwlock_new(void* data) {
    struct co_rwlock_core *core = 
            (struct co_rwlock_core *)malloc(sizeof(struct co_rwlock_core));
    core->stat = STAT_READ;
    core->reader_cnt = 0;
    core->data = data;

    CoRwLock rwlock = (CoRwLock) malloc(sizeof(struct co_rwlock));
    rwlock->core = core;
    rwlock->ticket = TICKET_NULL;

    return rwlock;
}

CoRwLock co_rwlock_clone(CoRwLock rwlock) {
    CoRwLock rwlock_clone = (CoRwLock) malloc(sizeof(struct co_rwlock));
    rwlock_clone->core = rwlock->core;
    rwlock_clone->ticket = TICKET_NULL;

    return rwlock_clone;
}

const void* co_rwlock_read(CoRwLock rwlock) {
    while (rwlock->core->stat != STAT_READ) {
        co_yield();
    }
    rwlock->ticket = current->cid;
    rwlock->core->reader_cnt++;
    return rwlock->core->data;
}

void co_rwlock_read_done(CoRwLock rwlock) {
    if (rwlock->ticket == current->cid) {
        rwlock->ticket = TICKET_NULL;
        rwlock->core->reader_cnt--;
    }
}

void** co_rwlock_write(CoRwLock rwlock) {
    while (rwlock->core->reader_cnt != 0) {
        co_yield();
    }
    rwlock->core->stat = STAT_WRITE;
    rwlock->core->owner = current->cid;
    return &rwlock->core->data;
}

void co_rwlock_write_done(CoRwLock rwlock) {
    if (rwlock->core->stat == STAT_WRITE &&
        rwlock->core->owner == current->cid) {
        rwlock->core->stat = STAT_READ;
    }
}