# `core`
## 简介
`core` 模块提供了 `Coroutine` 的核心实现, 以及对协程的配置。

## 函数类型
为了能够适配多种函数, 本库调度的协程函数声明应为:

```C
void* foo(void* args);
```

传递参数的时候, 结构体必须通过指针传递, 而基础类型可以通过强转为 `void*` 类型传递, 但可能会触发编译器警告。

## 创建协程
可以通过函数 `co_start` 启动一个协程:

```C
void* foo(void*) {
    // ...
}

Coroutine foo_routine = co_start(foo, args);
```

协程被创建后就会进入待调度状态, 等待调度 。

## yield
可以通过 `co_yield` 函数, 将控制权主动交给调度器, 调度器会切换到其它的 `Coroutine` 中运行:

```C
void* foo(void*) {
    // ...
    co_yield(); // --------------------------
    // ...      //                          |
}               //                          |
//                                          |
//                        -----------       |
//                   -----| excutor | <-----|
//                   |    -----------
//                   |
//                   |
void* bar(void*) {// |
    // ...           |
    co_yield();//    |
    // ...        <---
}

```

## 等待协程结束 (获取协程返回值)
可以通过函数 `co_wait` 来等待协程结束, 并获取返回值:

```C
void *ret = co_wait(foo_routine);
```

**注意：不通过 `co_wait` 等待协程结束, 而协程又没有被销毁的话, 会造成内存泄漏**

## 取消协程 (销毁协程)
可以通过函数 `co_cancel` 取消 (或者说) 销毁一个协程:

```C
co_cancel(foo_routine);
// foo_routine 被销毁, 不会再被调度
```

## 获得协程默认栈大小
可以通过函数 `co_get_default_stack_size` 获得创建协程时默认分配的栈空间大小。
本库初始默认栈大小为 `4 Mb` 。

```c
size_t default_stack_size = co_get_default_stack_size();
assert(default_stack_size == 4 * 1024 * 1024);
```

## 设置默认协程栈大小
可以通过函数 `co_set_default_stack_size` 设置协程的默认栈大小, 设置完成后, 再通过 `co_start` 创建
的协程的栈空间大小就是设置的值。

```c
size_t default_stack_size = co_get_default_stack_size();
assert(default_stack_size == 4 * 1024 * 1024);

co_set_default_stack_size(8 * 1024 * 1024);

size_t new_stack_size = co_get_default_stack_size();
assert(new_stack_size == 8 * 1024 * 1024);
```

## 获得当前协程栈大小
可以通过函数 `co_get_stack_size` 获取当前协程的栈大小。

```c
size_t default_stack_size = co_get_default_stack_size();
assert(default_stack_size == 4 * 1024 * 1024);

Coroutine foo_routine = co_start(foo, NULL);
size_t foo_stack_size = co_get_default_stack_size();
assert(foo_stack_size == 4 * 1024 * 1024);

co_set_default_stack_size(8 * 1024 * 1024);
size_t new_stack_size = co_get_default_stack_size();
assert(new_stack_size == 8 * 1024 * 1024);

Coroutine bar_routine = co_start(bar, NULL);
size_t bar_stack_size = co_get_default_stack_size();
assert(bar_stack_size == 8 * 1024 * 1024);
```
