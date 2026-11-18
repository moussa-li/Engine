---
name: object-oriented-design
description: >
  禁止以「namespace + 自由函数」的平铺方式新增功能接口，要求先做领域建模，
  将相关概念抽象为类与对象，并运用合适的设计模式组织代码。当新增功能、
  设计对外接口、扩展模块能力、重构既有过程式代码时使用此技能。
---

# 面向对象的领域建模规范

## 核心原则

新增功能**不得**以 `namespace { void func() {...} }` 的自由函数形式平铺实现。
必须先进行领域分析，识别核心概念与职责边界，将其抽象为**类与对象**，
再通过合适的设计模式组织协作关系，保证后续可维护、可扩展、可测试。

自由函数只允许作为**类内部实现细节**或**无状态纯工具函数**存在，
不能作为功能对外的组织形式。

## 禁止的写法

```cpp
//  平铺式：功能与状态割裂，无法扩展、无法测试、无法复用
namespace mesh {
    void generate(const std::string& model);
    void optimize(const std::string& model);
    void exportMesh(const std::string& model);
}
```