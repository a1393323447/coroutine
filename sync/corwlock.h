//
// Created by 朕与将军解战袍 on 2022/5/6.
//

#ifndef __COROUTINE_CORWLOCK_H__
#define __COROUTINE_CORWLOCK_H__

typedef struct co_rwlock* CoRwLock;

CoRwLock    co_rwlock_new(void* data);
CoRwLock    co_rwlock_clone(CoRwLock rwlock);
const void* co_rwlock_read(CoRwLock rwlock);
void        co_rwlock_read_done(CoRwLock rwlock);
void**      co_rwlock_write(CoRwLock rwlock);
void        co_rwlock_write_done(CoRwLock rwlock);

#endif //__COROUTINE_CORWLOCK_H__
