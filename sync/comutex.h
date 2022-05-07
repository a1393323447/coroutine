//
// Created by 朕与将军解战袍 on 2022/5/6.
//

#ifndef __COROUTINE_COMUTEX_H__
#define __COROUTINE_COMUTEX_H__

typedef struct comutex* CoMutex;

CoMutex co_mutex_new(void* data);
CoMutex co_mutex_clone(CoMutex mutex);
void**  co_mutex_lock(CoMutex mutex);
void    co_mutex_unlock(CoMutex mutex);

#endif //__COROUTINE_COMUTEX_H__
