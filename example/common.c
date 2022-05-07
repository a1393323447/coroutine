//
// Created by 朕与将军解战袍 on 2022/5/6.
//
// 用于展示 co_start, co_wait, co_cancel 的用法

#include <stdio.h>
#include <assert.h>
#include "../prelude.h"

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
    Coroutine task1 = co_start(task, (void*)1);
    Coroutine task2 = co_start(task, (void*)2);
    
    int task1_ret = (int)co_wait(&task1);
    int task2_ret = (int)co_wait(&task2);

    printf("task1_ret = %d\n", task1_ret);
    printf("task2_ret = %d\n", task2_ret);
}

void test_2() {
    Coroutine task1 = co_start(task, (void*)1);
    Coroutine task2 = co_start(task, (void*)2);
    Coroutine task3 = co_start(task, (void*)3);

    int i;
    for (i = 0; i < 10; i++) {
        co_yield();
    }

    co_cancel(&task2);
    puts("\n -------- Cancel task2 ------- \n");
    
    assert(task2 == NULL);

    int task1_ret = (int)co_wait(&task1);
    int task3_ret = (int)co_wait(&task3);


    printf("task1_ret = %d\n", task1_ret);
    printf("task3_ret = %d\n", task3_ret);
}

int main() {
    puts("\n --------  Start Test 1 -------- \n");
    test_1();

    puts("\n --------  Start Test 2 -------- \n");
    test_2();

    return 0;
}