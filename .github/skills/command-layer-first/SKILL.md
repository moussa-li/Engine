---
name: command-layer-first
description: >
  强制所有软件操作通过 Command 抽象与 CommandManager 统一调用，禁止在
  业务层直接调用底层算法、渲染或建模接口。当编写、修改功能模块，或涉及
  网格算法调试、MCP 对接、建模脚手架、操作命令、撤销重做、跨进程调用时
  使用此技能。
---

# Command 层优先规范

## 软件宗旨

本软件是一个**灵活的跨进程可视化工具**，核心场景：

1. 快速的网格算法可视化调试工具
2. 方便大模型通过 MCP 对接的可视化工具
3. 建模软件脚手架

为实现以上场景，**所有软件操作必须封装为 Command**，由 `CommandManager`
统一 `call` 调用。这是架构约束，不是风格偏好。

## 核心原则

- 任何「会改变软件状态或产生副作用」的操作，都必须先封装为 Command，
  再由 `CommandManager` 调度执行。
- 业务层、UI 层、MCP 适配层**不得**直接调用算法函数、渲染接口或建模 API。
- Command 必须是**自描述、可序列化、可跨进程传输**的数据对象，
  而非绑定了具体上下文的回调闭包。

## 分层与调用链

```text
调用方（UI / MCP / 脚本 / 守护进程）
        ↓ 只允许通过 CommandManager
CommandManager.call(command)
        ↓
具体 Command.execute()
        ↓
底层能力（网格算法 / 渲染 / 建模 / 文件 IO）
```