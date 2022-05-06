#include <stdio.h>
#include <assert.h>
#include "coroutine.h"

void* task(void* arg) {
    int i;
    for(i = 0; i < 100; i++) {
        int a = (int)arg;
        printf("Task %d, i = %d\n", a, i);
        co_yield();
    }
    return (void*)(((int)arg) + 1);
}

void test_1() {
    struct co* task1 = co_start(task, (void*)1);
    struct co* task2 = co_start(task, (void*)2);
    
    int task1_ret = (int)co_wait(&task1);
    int task2_ret = (int)co_wait(&task2);

    printf("task1_ret = %d\n", task1_ret);
    printf("task2_ret = %d\n", task2_ret);
}

void test_2() {
    struct co* task1 = co_start(task, (void*)1);
    struct co* task2 = co_start(task, (void*)2);
    struct co* task3 = co_start(task, (void*)3);

    int i;
    for (i = 0; i < 10; i++) {
        co_yield();
    }

    co_cancel(&task2);
    printf("\n\n -------- Cancel task2 ------- \n\n");
    
    assert(task2 == NULL);

    int task1_ret = (int)co_wait(&task1);
    int task3_ret = (int)co_wait(&task3);


    printf("task1_ret = %d\n", task1_ret);
    printf("task3_ret = %d\n", task3_ret);
}

int main() {

    printf("Running Test 1\n");
    test_1();

    printf("Running Test 2\n");
    test_2();

    return 0;
}