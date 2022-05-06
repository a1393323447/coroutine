typedef void* (*Func)(void *);

struct co *co_start(Func func, void *arg);
void  co_yield();
void* co_wait(struct co **p_co);