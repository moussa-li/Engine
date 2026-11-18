---
name: common-containers-only
description: >
  强制使用项目 Common 模块自研实现（DynamicArray、HashTable、String、
  UniquePtr、SharedPtr 及 move/forward 工具），禁止引入底层 STL 容器与
  智能指针。当编写、修改、重构或评审任何 C++ 代码，或涉及容器选型、
  数据结构、字符串处理、内存管理、所有权转移时使用此技能。
---

# Common 自研实现优先规范

## 核心原则

所有新增与修改的 C++ 代码，容器、字符串、哈希表、智能指针、移动语义工具
必须使用 `Common` 命名空间下的自研实现，**不得**使用 `std::vector`、
`std::unordered_map`、`std::map`、`std::string`、`std::unique_ptr`、
`std::shared_ptr` 等底层 STL 组件。

## 类型映射表

| 场景 | 禁止使用 | 必须使用 |
|---|---|---|
| 连续存储 / 动态数组 | `std::vector`、`std::array`、裸 `new T[]` | `Common::DynamicArray<T>` |
| 键值映射 / 快速查找 | `std::unordered_map`、`std::map` | `Common::HashTable<K, V>` |
| 字符串 | `std::string`、`std::string_view` | `Common::String` |
| 集合去重 | `std::unordered_set`、`std::set` | `Common::HashTable<K, Empty>` 或 Common 提供的 Set |
| 独占所有权 | `std::unique_ptr`、裸 `new`/`delete` | `Common::UniquePtr<T>` |
| 共享所有权 | `std::shared_ptr`、`std::weak_ptr` | `Common::SharedPtr<T>` |
| 移动 / 完美转发 | `std::move`、`std::forward` | `Common::move`、`Common::forward` |
| 内存分配 | `malloc` / `new` / `delete` 裸操作 | Common 提供的分配器 / 内存池 |

> 若 Common 中尚无对应实现，**不要自行回退到 STL**，应先向用户说明缺口并确认方案。

## 执行步骤

1. 生成代码前，先确认 Common 中相关组件的头文件路径与真实接口签名
   （通过读取 `Common/` 目录下的头文件确认，不要凭记忆假设 API）。
2. 容器与智能指针选型严格按上表映射，不做「这里用 STL 更方便」的例外判断。
3. 涉及网格数据（节点、单元、邻接表、面信息）时，优先 `Common::DynamicArray`
   配合索引访问，避免指针容器。
4. 需要排序、查找等算法时，优先使用 Common 提供的算法；缺失时先询问用户，
   不擅自引入 `<algorithm>` 对 STL 容器的依赖。
5. 所有权设计：优先栈对象与引用；确需堆分配时用 `Common::UniquePtr`，
   仅当存在多所有权语义时才使用 `Common::SharedPtr`。
6. 输出代码后自检：全文搜索 `std::`，确认无禁用容器、智能指针与
   move/forward 残留。

## 允许保留的例外

以下不属于「底层 STL 容器与智能指针」，可以正常使用：

- `<cstdint>`、`<cstddef>`、`<limits>`、`<type_traits>`、`<utility>` 等基础头
- `std::hash`（作为哈希策略传入 Common::HashTable）
- `std::cout` / `std::cerr`（仅限测试代码与 `main` 入口）

## 示例

**错误写法**

```cpp
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>

std::vector<int> nodes;
std::unordered_map<std::string, int> index;
auto ptr = std::unique_ptr<Mesh>(new Mesh());
transfer(std::move(ptr));
```