#include <stdio.h>
#include "coroutine.h"

void* test(void* arg) {
    int i;
    for(i = 0; i < 100; i++) {
        int a = (int)arg;
        printf("Task %d, i = %d\n", a, i);
        co_yield();
    }
    return (void*)(((int)arg) + 1);
}

int main() {
    struct co* task1 = co_start(test, (void*)1);
    struct co* task2 = co_start(test, (void*)2);
    
    int task1_ret = (int)co_wait(&task1);
    int task2_ret = (int)co_wait(&task2);

    printf("task1_ret = %d\n", task1_ret);
    printf("task2_ret = %d\n", task2_ret);

    return 0;
}