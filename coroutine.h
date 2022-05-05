typedef void* (*Func)(void *);

struct co *co_start(const char *name, Func func, void *arg);
void  co_yield();
void* co_wait(struct co *co);