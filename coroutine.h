typedef void* (*Func)(void *);

typedef struct co* Coroutine;

Coroutine co_start(Func func, void *arg);
void      co_cancel(Coroutine *p_co);
void*     co_wait(Coroutine *p_co);
void      co_yield();