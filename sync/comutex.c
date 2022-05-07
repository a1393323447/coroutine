//
// Created by 朕与将军解战袍 on 2022/5/6.
//

#include "comutex.h"
#include "../core/core.h"

#include <stdlib.h>

extern Coroutine current;

#define UNLOCK ((int)0xc00c)
#define LOCK   ((int)0xd00b)

struct comutex {
    void   *data;
    int    stat;
    coid_t owner;
};

CoMutex co_mutex_new(void* data) {
    CoMutex mutex = (CoMutex) malloc(sizeof(struct comutex));

    if (mutex == NULL) return NULL;

    mutex->data = data;
    mutex->stat = UNLOCK;
    mutex->owner = 0;

    return mutex;
}

CoMutex co_mutex_clone(CoMutex mutex) {
    return mutex;
}

void** co_mutex_lock(CoMutex mutex) {
    while (mutex->stat == LOCK) {
        co_yield();
    }
    mutex->stat = LOCK;
    mutex->owner = current->cid;
    return &mutex->data;
}

void co_mutex_unlock(CoMutex mutex) {
    if (mutex->owner == current->cid) {
        mutex->stat = UNLOCK;
    }
}