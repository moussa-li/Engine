---
name: common-concurrency-only
description: >
  强制使用项目 Common 模块自研的并发与事件组件（Publisher、Subscriber、
  MemoryQueue 等），禁止引入底层 STL 并发原语与第三方事件框架。
  当编写、修改多线程代码，或涉及事件监听、发布订阅、线程间通信、
  无锁队列、跨进程/跨平台数据对齐时使用此技能。
---

# Common 并发与事件组件优先规范

## 核心原则

所有涉及多线程、事件监听、发布订阅、线程间通信的代码，必须使用
`Common` 命名空间下的自研组件，**不得**使用 `std::thread`、
`std::mutex`、`std::condition_variable`、`std::async`、`std::future`、
`std::atomic` 等底层 STL 并发原语，也不得引入 Boost、TBB、
ZeroMQ 等第三方并发/事件框架。

## 组件映射表

| 场景 | 禁止使用 | 必须使用 |
|---|---|---|
| 事件发布 / 广播 | 手写回调表、`std::function` 容器、Boost.Signals | `Common::Publisher<Event>` |
| 事件订阅 / 监听 | 手写观察者、裸函数指针数组 | `Common::Subscriber<Event>` |
| 线程间数据传递 | `std::queue` + `std::mutex` 手写包装 | `Common::MemoryQueue<T>` |
| 跨平台共享内存队列 | Boost.Interprocess、原生 shm 手写 | Common 提供的共享内存队列封装 |
| 线程生命周期 | `std::thread`、`std::jthread` | Common 提供的线程/任务封装 |
| 互斥与同步 | `std::mutex`、`std::lock_guard`、`std::atomic` | Common 提供的锁与原子封装 |
| 异步结果回传 | `std::future`、`std::promise`、`std::async` | Common 提供的回调 / 结果通道 |

> 若 Common 中尚无对应能力，**不要自行回退到 STL 或第三方库**，
> 应先向用户说明缺口并确认方案。

## 执行步骤

1. 生成代码前，先读取 `Common/` 下相关头文件，确认 `Publisher`、
   `Subscriber`、`MemoryQueue` 的真实接口签名与模板参数，不凭记忆假设 API。
2. 事件流严格按 **Publisher → Subscriber** 单向设计，订阅方不反向持有
   发布方的强引用，避免循环引用导致泄漏。
3. 生产者-消费者场景统一用 `Common::MemoryQueue`，不手写
   「队列 + 条件变量」组合。
4. 涉及跨平台（Windows / Linux）时，优先使用 Common 的共享内存队列
   与统一事件总线，不在业务代码里出现平台分支宏。
5. 明确线程边界：标注哪些对象属于渲染线程、哪些属于算法/守护进程线程，
   跨边界只通过事件或队列传递数据，不直接共享裸指针。
6. 输出代码后自检：全文搜索 `std::` 与 `boost::`，确认无禁用并发原语残留。

## 允许保留的例外

- `<cstdint>`、`<cstddef>`、`<type_traits>`、`<utility>` 等基础头
- `std::hash`（作为哈希策略传入 Common::HashTable）
- `std::cout` / `std::cerr`（仅限测试代码与 `main` 入口）

## 示例

**错误写法**

```cpp
#include <thread>
#include <mutex>
#include <queue>
#include <functional>

std::mutex mtx;
std::queue<Task> queue;
std::vector<std::function<void(const Event&)>> listeners;

void onEvent(const Event& e) {
    for (auto& fn : listeners) fn(e);
}
```