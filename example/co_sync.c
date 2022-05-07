//
// Created by 朕与将军解战袍 on 2022/5/7.
//
// 用于展示 sync 中的结构的用法和使用场景
//
// 示例说明
// 1. modify_across_yield:
// 协程是单线程的异步, 所以不存在多线程中的数据竞争的问题, 但如果两个协程(A, B)共享一个数据 C ,
// 而且在协程 A 修改数据 C 的期间, 通过 co_yield 交出控制权, B 就可能修改 C 。此时 A 就会
// 得到意料之外的数据
// 2. modify_across_with_lock
// 这个例子中, 通过 CoMutex 对 point 上锁, 使得 point 在 yield 期间不会被其它协程修改
// 3. modify_recommend:
// 这个例子中, 展示了我个人推荐的模式: 在 yield 后, 不要依赖 yield 前的协程共享数据

#include <stdio.h>
#include "../core/core.h"
#include "../sync/comutex.h"

struct point {
    int x;
    int y;
};

static int task_id = 1;
void* set_point(void *p) {
    int task_current_id = task_id;
    struct point *point = p;

    for (int i = 0; i < 10; i++) {
        point->x++;
        co_yield();
        point->y++;
        printf("Task %d: (x, y) = (%d, %d)\n",  task_current_id, point->x, point->y);
    }

    return NULL;
}

void* set_point_lock(void* lock) {
    CoMutex mutex = lock;
    int task_current_id = task_id;
    struct point* point = NULL;

    for (int i = 0; i < 10; i++) {
        point = *co_mutex_lock(mutex);
        point->x++;
        co_yield();
        point->y++;
        co_mutex_unlock(mutex);
        printf("Task %d: (x, y) = (%d, %d)\n",  task_current_id, point->x, point->y);
    }

    return NULL;
}

void* set_point_recommend(void* p) {
    int task_current_id = task_id;
    struct point *point_p = p;

    for (int i = 0; i < 10; i++) {
        point_p->x++;
        point_p->y++;
        printf("Task %d: (x, y) = (%d, %d)\n",  task_current_id, point_p->x, point_p->y);
        co_yield();
    }

    return NULL;
}

void modify_across_yield() {
    struct point point = {.x = 0, .y = 0};

    task_id = 1;
    Coroutine task_1 = co_start(set_point, &point);

    task_id = 2;
    Coroutine task_2 = co_start(set_point, &point);

    co_wait(&task_1);
    co_wait(&task_2);
}

void modify_with_lock() {
    struct point point = {.x = 0, .y = 0};
    CoMutex mutex = co_mutex_new(&point);

    task_id = 1;
    Coroutine task_1 = co_start(set_point_lock, mutex);

    task_id = 2;
    Coroutine task_2 = co_start(set_point_lock, co_mutex_clone(mutex));

    co_wait(&task_1);
    co_wait(&task_2);
}

void modify_recommend() {
    struct point point = {.x = 0, .y = 0};

    task_id = 1;
    Coroutine task_1 = co_start(set_point_recommend, &point);

    task_id = 2;
    Coroutine task_2 = co_start(set_point_recommend, &point);

    co_wait(&task_1);
    co_wait(&task_2);
}

int main() {
    puts("\n ----- modify across yield ----- \n");
    modify_across_yield();

    puts("\n ----- modify across with lock -----\n");
    modify_with_lock();

    puts("\n ----- modify across recommend -----\n");
    modify_recommend();

    return 0;
}

