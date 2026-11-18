---
name: singleton-base-class
description: >
  要求所有单例类必须派生自 Common 提供的 Singleton<Derived> CRTP 基类，
  禁止手写 instance() 静态方法、全局指针或裸 new 实现单例。当需要实现
  单例模式、全局管理器、唯一服务实例，或涉及生命周期管理、依赖注入时
  使用此技能。
---

# 单例基类使用规范

## 核心原则

项目中所有单例类**必须**派生自 `Common::Singleton<Derived>` 基类，
通过 CRTP 获得统一的 `instance()` 访问入口。

**禁止**以下实现方式：
- 在类内手写 `static T& instance()` 返回局部静态变量
- 使用全局指针 + `new` 构造单例
- 使用 `std::call_once`、`std::atomic` 手写双重检查锁
- 通过宏（如 `DECLARE_SINGLETON`）生成单例代码

## 基类定义

```cpp
namespace Common
{

template <class Derived>
class Singleton
{
public:
    static Derived& instance()
    {
        static Derived instance;
        return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

protected:
    Singleton() = default;
    ~Singleton() = default;
};
}