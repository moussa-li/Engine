# AI ↔ EnginePlatform 完整对接链路 — 架构设计蓝图

- 文档编号：`AI_IPC_Blueprint`
- 版本：v1.0
- 读者：Foundation-Engineer / Mesh-Expert / Agent-Engineer / UI-Engineer
- 状态：待评审
- 依据事实（不得臆造）：
  - `EnginePlatform/include/Command/CommandManager.hpp`（`Common::Singleton<CommandManager>`，`call(EventId, EventPacket)` / `call(processId, EventId, PacketPOD)`）
  - `EnginePlatform/include/Command/CommandIds.hpp`（现有 EventId 枚举）
  - `EnginePlatform/src/Command/CommandCatalog.cpp`（`CommandDesc` 元数据）
  - `EnginePlatform/include/Command/CommandParam.hpp`（参数 POD）
  - `EnginePlatform/src/Command/CommandTransport.cpp`（`Common::MemoryQueue + Publisher` 跨进程）
  - `EnginePlatform/src/Command/CommandManager.cpp`（Impl 工作线程 `std::thread` 为存量代码）
  - `Common/include/Common/{MemoryQueue,Publisher,Subscriber,MessageCenter,Message,Log,Thread,Protocol}.hpp`
  - `AiClient/{mcp_tools.json,command_schemas.json,deepseek_command_client.py}`（阶段一产物，可复用）

---

## 0. 铁律（本蓝图所有设计的最高约束）

1. 禁止 `std::vector` / `std::string` / `std::thread` / `std::mutex` 等 STL 底层组件进入新增业务代码；必须使用 `Common::DynamicArray` / `Common::String` / `Common::Thread` / `Common::Mutex` 等自研组件。
2. 禁止 Boost / TBB / ZeroMQ / websocketpp 等第三方框架；传输层只用操作系统原生 API（Windows Named Pipe / WinSock2）。
3. 所有单例派生自 `Common::Singleton<Derived>`。
4. 所有引擎能力经 `CommandManager` 注册，并附带 JSON Schema（Command as Tool）。
5. AI 模块为独立客户端/进程，通过标准 IPC 通信，不侵入 C++ 引擎核心。
6. 面向对象领域建模，禁止 `namespace + 自由函数` 平铺。
7. 存量 `CommandManager::Impl` 与 `CommandBus` 中的 `std::thread` 可沿用，但**不新增**任何裸 `std::thread/std::mutex`。

---

## 1. 整体分层与模块划分

### 1.1 三层拓扑

```
┌────────────────────────────────────────────────────────────┐
│ AI Dialog UI（独立窗口，客户端）                              │
│  - 流式对话渲染（text_delta）                                │
│  - 任务进度（task_progress）                                 │
│  - 工具调用可视化（tool_call / tool_result）                  │
│  - HTTP Client + SSE Consumer（仅 stdlib）                  │
└───────────────▲─────────────────────────────────────────────┘
                │ HTTP+SSE（localhost，Agent 为服务端）
┌───────────────┴─────────────────────────────────────────────┐
│ EgLab-Agent（独立 Python 进程，仅 Python 标准库）             │
│  - Agent 角色：Dev-Commander / Architect / Mesh-Expert       │
│  - LLM：DeepSeek V4 Pro（规划）/ Flash（执行/流式）          │
│  - SessionManager：会话/上下文窗口/工具调用历史               │
│  - MCP Client：Named Pipe + NDJSON + JSON-RPC 2.0          │
│  - Agent HttpServer：HTTP+SSE 事件出口                       │
└───────────────▲─────────────────────────────────────────────┘
                │ Named Pipe（\\.\pipe\eglab_mcp）+ NDJSON
┌───────────────┴─────────────────────────────────────────────┐
│ EnginePlatform（C++ 主进程，内嵌 MCP Server）                 │
│  - McpServer / McpSession / McpToolDispatcher               │
│  - IMcpTool：CreateMesh / SetCamera / GetLog                │
│  - CommandManager + CommandCatalog（Command as Tool）       │
│  - Common：MemoryQueue / MessageCenter / LogRingBuffer      │
│  - RenderWork / MeshEngine / Context                        │
└──────────────────────────────────────────────────────────────┘
```

### 1.2 模块职责表

| 层 | 模块 | 职责 | 关键边界 |
|---|---|---|---|
| C++ | `McpServer` | 命名管道监听、会话生命周期、IO 循环编排 | 只做协议与调度，不含业务 |
| C++ | `McpSession` | 单连接字节流读写、NDJSON 组帧/拆帧、写缓冲 | 一个连接一个对象 |
| C++ | `JsonRpcCodec` | JSON-RPC 2.0 编解码、错误对象构造 | 无状态 |
| C++ | `McpToolDispatcher` | 工具注册表、`tools/list`、`tools/call` 分发、JSON→EventPacket 转换 | 工具唯一入口 |
| C++ | `IMcpTool` 及实现 | 单个工具的 schema 与执行语义 | 不直接碰网络 |
| C++ | `CommandManager` | 引擎命令总线（已有） | 不感知 MCP |
| C++ | `MessageCenter` + `LogRingBuffer` | 日志产生与环形缓存（GetLog 数据源） | Common 层 |
| Python | `McpClient` | Named Pipe 客户端、JSON-RPC 请求/通知、超时与重连 | 仅 stdlib |
| Python | `AgentOrchestrator` | 角色路由、LLM 调用编排、工具循环 | 不感知 UI |
| Python | `SessionManager` | 会话/上下文维护 | 持久化可选 |
| Python | `AgentHttpServer` | HTTP+SSE 出口 | 仅 stdlib `http.server` |
| UI | `ChatView / ProgressView / ToolCallView` | 展示层 | 只消费 SSE |

---

## 2. 传输层选型（零第三方依赖）

### 2.1 方案对比

| 维度 | Named Pipe（Windows） | 自研 TCP（WinSock2） | 自研 WebSocket（TCP + 自研帧） |
|---|---|---|---|
| 依赖 | OS 原生 `CreateNamedPipe/ConnectNamedPipe/ReadFile/WriteFile` | OS 原生 `socket/bind/listen/accept` | 需再自研握手/掩码/分片/ping-pong |
| Python 端 | `open(r'\\.\pipe\eglab_mcp','r+b',buffering=0)`，stdlib 直接可用 | stdlib `socket` 可用 | 需自写 WebSocket 帧处理器 |
| 端口/防火墙 | 无端口、无防火墙弹窗 | 需选端口、防火墙可能拦截 | 同 TCP |
| 适用场景 | 单机、单/少量本地客户端 | 单机或未来远程 | 浏览器直连 |
| 实现成本 | 低（封装 1 个类） | 中（Acceptor+IO 循环） | 高 |
| 可移植性 | Windows-only（当前项目即 Windows_NT，可接受） | 好 | 好 |

### 2.2 推荐方案

**推荐：C++ ↔ Python 采用 Windows Named Pipe + NDJSON。**

理由：
1. 目标拓扑是「同机、单个 Agent 进程」连接「内嵌 MCP Server」，Named Pipe 的连接模型（1 对 1/少量实例）完全匹配。
2. 零端口配置、零防火墙干扰，内嵌 Server 可随引擎进程启停自动消失，不残留监听端口。
3. Python 标准库用文件方式即可打开命名管道，Agent 侧依然零第三方依赖。
4. 避免自研 WebSocket 帧处理（掩码、分片、ping/pong）的复杂度——本链路没有浏览器直连 C++ 的需求。

为保留未来扩展性：抽象 `Common::IpcChannel`（字节流接口），本期只实现 `NamedPipeChannel`；若未来需跨平台，可平行插入 `TcpSocketChannel` 而不改上层协议。

### 2.3 Common 库需 Foundation-Engineer 新增的组件清单（含接口草案）

> 均为设计签名，不含实现体；命名遵循 Common 现有风格（类大驼峰、函数小驼峰、成员 `_xxx`）。

```cpp
// 1. JSON（零依赖 DOM，AI 链路的核心基础件）
namespace EgLab::Common {
class JsonValue {
public:
    enum class Type { Null, Bool, Int64, Double, String, Array, Object };
    Type type() const;
    bool asBool(bool def = false) const;
    int64_t asInt64(int64_t def = 0) const;
    double asDouble(double def = 0.0) const;
    const String& asString() const;
    const DynamicArray<JsonValue>& asArray() const;
    const HashMap<String, JsonValue>& asObject() const;
    JsonValue& operator[](size_t index);
    JsonValue& operator[](const String& key);
    // 构造器：JsonValue()/JsonValue(bool)/JsonValue(int64_t)/...
};
class JsonParser {
public:
    static bool parse(const String& text, JsonValue& out, String& error);
};
class JsonWriter {
public:
    static bool write(const JsonValue& value, String& out);
};
}

// 2. NDJSON 行组帧器
namespace EgLab::Common {
class LineFramer {
public:
    void push(const uint8_t* data, size_t size);
    bool next(DynamicArray<uint8_t>& line); // 取出一条以 '\n' 结尾的行
    void reset();
private:
    DynamicArray<uint8_t> _buffer;
};
}

// 3. 字节流抽象 + Named Pipe 封装（Windows 原生 API）
namespace EgLab::Common {
class IpcChannel {
public:
    virtual ~IpcChannel() = default;
    virtual bool isOpen() const = 0;
    virtual size_t read(uint8_t* buf, size_t capacity) = 0;  // 0 = 无数据或已关闭
    virtual size_t write(const uint8_t* buf, size_t size) = 0;
    virtual void close() = 0;
    virtual uintptr_t nativeHandle() const = 0;              // 供 IoPoller 使用
};

class NamedPipeServer {
public:
    NamedPipeServer(const String& pipeName, uint32_t maxInstances = 4);
    bool listen();                                            // CreateNamedPipe + 等待连接
    UniquePtr<IpcChannel> accept(uint32_t timeoutMs);
    void close();
private:
    String _pipeName;
    // 原生句柄 + 状态，不暴露给上层
};

class NamedPipeClient {  // 预留：C++ 侧客户端；Python 侧用 open() 直接访问
public:
    explicit NamedPipeClient(const String& pipeName);
    bool connect(uint32_t timeoutMs);
};
}

// 4. IO 事件循环（select 封装，非阻塞句柄轮询）
namespace EgLab::Common {
enum class IoEventKind { Readable, Writable, Error };
struct IoEvent {
    uintptr_t handle;
    void* context;
    IoEventKind kind;
};
class IoPoller {
public:
    bool add(uintptr_t handle, void* context, uint32_t events);
    bool remove(uintptr_t handle);
    int32_t wait(DynamicArray<IoEvent>& out, uint32_t timeoutMs);
};
}

// 5. 线程池（内部用 Common::Thread 实现，禁止裸 std::thread）
namespace EgLab::Common {
class IRunnable {
public:
    virtual ~IRunnable() = default;
    virtual void run() = 0;
};
class ThreadPool {
public:
    explicit ThreadPool(size_t workerCount);
    bool post(SharedPtr<IRunnable> task);
    void stop(bool drainPending);
private:
    // Common::Thread 数组 + 任务队列（建议复用 MemoryQueue 或新 TaskQueue）
};
}

// 6. 互斥与条件变量（补足并发原语，供会话写缓冲/环形日志使用）
namespace EgLab::Common {
class Mutex {
public:
    void lock();
    void unlock();
};
class ConditionVariable {
public:
    void wait(Mutex& mutex);
    void notifyOne();
    void notifyAll();
};
}

// 7. 环形日志缓存（GetLog 数据源）
namespace EgLab::Common {
struct LogEntry {
    int32_t level;      // MessageLevel 底层值
    uint64_t sequence;
    String text;
    uint64_t timestamp;
};
class LogRingBuffer {
public:
    explicit LogRingBuffer(size_t capacity = 2048);
    void append(int32_t level, const String& text, uint64_t sequence);
    size_t query(int32_t minLevel, const String& filter, size_t count,
                 DynamicArray<LogEntry>& out); // 新→旧
private:
    Mutex _mutex;
    DynamicArray<LogEntry> _entries;
    uint64_t _cursor;
};
}
```

> 说明：`Common::Thread` 已存在（封装 `std::thread`），新增并发组件必须基于它；`std::atomic` 在存量 `Protocol.hpp` 已使用，新代码如需原子量优先封装为 `Common::Atomic<T>`（可选 P1 追加）。

---

## 3. MCP 协议细节（JSON-RPC 2.0 over NDJSON）

### 3.1 分帧方式

- **NDJSON**：每个 JSON 对象独占一行，以 `\n` 结尾；JSON 字符串内的换行必须被 `\n` 转义，保证「物理一行 = 一个消息」。
- 编码 UTF-8；单行上限 `MAX_MCP_LINE_BYTES = 1 MiB`（超限断开连接）。
- 发送端必须保证一次 `write` 不拆半行；接收端用 `LineFramer` 组帧。

### 3.2 消息格式

```jsonc
// Request（Client → Server）
{ "jsonrpc": "2.0", "id": 1, "method": "tools/call",
  "params": { "name": "CreateMesh", "arguments": { "geometryType": "box" } } }

// Success Response（Server → Client）
{ "jsonrpc": "2.0", "id": 1,
  "result": { "content": [ { "type": "text", "text": "{\"nodeId\":42}" } ], "isError": false } }

// Error Response
{ "jsonrpc": "2.0", "id": 1,
  "error": { "code": -32602, "message": "Invalid params", "data": { "detail": "missing geometryType" } } }

// Notification（任一方，无 id）
{ "jsonrpc": "2.0", "method": "notifications/toolResult",
  "params": { "requestId": 7, "toolName": "CreateMesh", "ok": true, "nodeId": 42 } }
```

### 3.3 方法列表

| 方法 | 方向 | 说明 |
|---|---|---|
| `initialize` | C→S | 极简握手：协议版本、客户端信息；返回 `serverInfo` |
| `notifications/initialized` | C→S | 握手完成通知（可选） |
| `tools/list` | C→S | 返回全部工具名 + `inputSchema` |
| `tools/call` | C→S | 执行工具，返回 result 或 error |
| `notifications/toolResult` | S→C | 变更类工具的**异步执行结果**（见 §4.4） |

### 3.4 tools/list 结果

```jsonc
{ "jsonrpc": "2.0", "id": 2,
  "result": { "tools": [
    { "name": "CreateMesh", "description": "...", "inputSchema": { /* JSON Schema */ } },
    { "name": "SetCamera",  "description": "...", "inputSchema": { /* JSON Schema */ } },
    { "name": "GetLog",     "description": "...", "inputSchema": { /* JSON Schema */ } }
  ] } }
```

### 3.5 错误码表

| code | 常量 | 含义 |
|---|---|---|
| -32700 | ParseError | NDJSON 行不是合法 JSON |
| -32600 | InvalidRequest | 缺 `jsonrpc`/`method`/`id` 等 |
| -32601 | MethodNotFound | 方法未注册 |
| -32602 | InvalidParams | `arguments` 不符合工具 schema |
| -32603 | InternalError | 服务端未预期异常 |
| -32000 | ToolNotFound | `tools/call` 的 `name` 未注册 |
| -32001 | ToolExecutionFailed | 命令执行失败（CommandBus 返回 Failed） |
| -32002 | ToolTimeout | 引擎侧执行超时 |
| -32003 | ArgumentValidationFailed | 参数校验失败（携带 `data.detail`） |

---

## 4. 三个工具的 Command 映射与参数 JSON Schema

### 4.1 总映射表

| MCP 工具 | 引擎 Command | 动作 | 结果通道 |
|---|---|---|---|
| `CreateMesh` | **新增** `EventId::CreateMesh = 200009` | 创建参数化几何体 | 异步 `notifications/toolResult`（携带 nodeId） |
| `SetCamera` | `mode=="fit"` → **复用** `FitView(100002)`；其余 → **新增** `EventId::SetCamera = 300005` | 设置相机 | 异步 `notifications/toolResult` |
| `GetLog` | **不走 CommandManager**（只读查询） | 读 `Common::LogRingBuffer` | 同步返回日志列表 |

### 4.2 CreateMesh

- 新增 EventId：`CreateMesh = 200009`（Mesh 段 `2xxxxx`，`200008` 之后空闲）。
- 新增 `CreateMeshParam`（POD，trivially copyable）：

```cpp
struct CreateMeshParam {
    uint64_t requestId;   // 协程/会话关联 id，用于异步结果回传
    uint32_t geometryType; // 0=box 1=sphere 2=cylinder 3=cone 4=plane 5=torus
    float size;
    uint32_t subdivisions;
    char name[64];
};
```

- `CommandCatalog` 新增 `CommandDesc{ EventId::CreateMesh, "CreateMesh", "Create Mesh", "Mesh", true, sizeof(CreateMeshParam), ... }`；字段：`name(String,64)`、`geometryType(String, 枚举提示)`、`size(Float32, 需扩展 ParamFieldType)`、`subdivisions(UInt32)`。
- 新增 `CreateMeshCmd : CommandBase<CreateMeshParam>`，`execImpl()` 委托给 MeshEngine 几何生成服务（Mesh-Expert 需在 `RenderWork/MeshEngine` 侧提供 `createPrimitive(...)` 能力），完成后把 `nodeId` 写入完成通知。
- `CommandManager::Impl::registerCommand()` 新增 `_creator[CreateMesh]` 与 `_packetCreator[CreateMesh]`（键名与 JSON 字段一致：`name/geometryType/size/subdivisions/requestId`）。

JSON Schema（与 `AiClient/mcp_tools.json` 对齐）：

```jsonc
{ "type": "object",
  "properties": {
    "geometryType": { "type": "string", "enum": ["box","sphere","cylinder","cone","plane","torus"] },
    "name":         { "type": "string", "maxLength": 64 },
    "size":         { "type": "number", "minimum": 0.0001, "maximum": 10000 },
    "subdivisions": { "type": "integer", "minimum": 1, "maximum": 256 }
  },
  "required": ["geometryType"], "additionalProperties": false }
```

### 4.3 SetCamera

- 复用：`mode=="fit"` → 直接 `CommandManager::call(EventId::FitView, EventPacket{})`。
- 新增 EventId：`SetCamera = 300005`（Camera 段 `3xxxxx`，`300004` 之后空闲）。
- 新增 `SetCameraParam`：

```cpp
struct SetCameraParam {
    uint64_t requestId;
    uint32_t mode;       // 0=orbit 1=fps
    float position[3];
    float target[3];
    float fov;           // 垂直视场角（度）
};
```

- 新增 `SetCameraCmd : CommandBase<SetCameraParam>`，`execImpl()` 通过 `RenderWork` 在渲染线程设置相机（位置/目标/fov）。
- 现有 `CameraPan/CameraRotate/CameraRotateLocal/CameraZoom` **不参与**：它们是 `holdInput/mouseOnly` 连续手势命令，不适合离散一次性调用（`command_schemas.json` 已明确排除）。
- `CommandCatalog` 新增 `SetCamera` 描述；字段需扩展 `ParamFieldType::Float32` 与数组字段（`ParamFieldDesc` 增加 `arrayLength`）以表达 `position[3]/target[3]`。

JSON Schema：

```jsonc
{ "type": "object",
  "properties": {
    "mode":     { "type": "string", "enum": ["fit","orbit","fps"] },
    "position": { "type": "array", "items": { "type": "number" }, "minItems": 3, "maxItems": 3 },
    "target":   { "type": "array", "items": { "type": "number" }, "minItems": 3, "maxItems": 3 },
    "fov":      { "type": "number", "minimum": 10, "maximum": 170 }
  },
  "required": ["mode"], "additionalProperties": false }
```

### 4.4 GetLog（只读查询，绕过 fire-and-forget 命令总线）

- `CommandManager::call` 是**单向**（`void`），无法回传结果，故 GetLog **不作为 Command** 注册；由 `GetLogTool` 直接同步查询 `Common::LogRingBuffer`。
- `LogRingBuffer` 数据来源：`MessageCenter::emit(...)` 与 `Common::Log`（LOG/Logger）统一汇入（建议 MessageCenter 作为唯一 funnel）。
- JSON Schema：

```jsonc
{ "type": "object",
  "properties": {
    "count":  { "type": "integer", "minimum": 1, "maximum": 500 },
    "level":  { "type": "string", "enum": ["all","debug","info","warning","error"] },
    "filter": { "type": "string", "maxLength": 128 }
  },
  "additionalProperties": false }
```

- 同步返回：`result.content[0].text` 为日志条目的 JSON 数组。
- **铁律兼容说明**：若评审要求「所有能力必须经 CommandManager」，可在 P1.2 追加 `CommandManager::query(QueryId, QueryPacket, QueryResult&)` 只读查询通道；本蓝图默认采用直接查询服务（更克制、不阻塞命令总线）。

### 4.5 异步结果通道（变更类工具）

1. `tools/call` 立即返回 `{"status":"accepted","requestId":<id>}`，不阻塞 IO 线程。
2. `CreateMeshParam/SetCameraParam` 携带 `requestId`。
3. `CommandBase` 执行结束处（已有 MessageCenter emit 点）追加/改为发送结构化 `CommandFinishedMessage{requestId, returnCode, nodeId}`（Common 新消息通道，trivially copyable）。
4. `McpServer` 订阅该通道，翻译为 `notifications/toolResult` 回写 Python。

---

## 5. C++ 侧类设计（继承关系 / Singleton / 接口签名）

### 5.1 类图（协作关系）

```
Common::Singleton<McpServer>
        │ owns
        ├── Common::UniquePtr<NamedPipeServer>
        ├── Common::UniquePtr<McpToolDispatcher>
        ├── Common::DynamicArray<Common::SharedPtr<McpSession>>
        └── Common::Thread _ioThread

McpSession (1:1 连接，非单例)
        ├── Common::UniquePtr<IpcChannel> _channel
        ├── LineFramer _inFramer
        └── DynamicArray<uint8_t> _outBuffer

McpToolDispatcher (被 McpServer 拥有)
        ├── Common::HashMap<Common::String, SharedPtr<IMcpTool>> _tools
        └── CommandCatalog（只读，生成 Command as Tool 透传列表）

IMcpTool（接口）
        ├── CreateMeshTool
        ├── SetCameraTool
        └── GetLogTool

JsonRpcCodec（协议编解码，无状态，组合使用）
```

### 5.2 关键接口签名（设计草案）

```cpp
namespace EgLab::Platform {

struct McpServerConfig {
    Common::String pipeName{ "eglab_mcp" };   // \\.\pipe\eglab_mcp
    uint32_t maxInstances{ 4 };
    uint32_t maxLineBytes{ 1024 * 1024 };
    uint32_t pollIntervalMs{ 10 };
};

// 内嵌 MCP Server 生命周期门面；唯一对外集成点。
class McpServer : public Common::Singleton<McpServer> {
public:
    bool start(const McpServerConfig& config);
    void stop();                       // 停止接受→关闭会话→join IO 线程
    bool isRunning() const;
    McpToolDispatcher& dispatcher();
private:
    McpServer();
    ~McpServer();
    friend class Common::Singleton<McpServer>;
    class Impl;
    Common::UniquePtr<Impl> _impl;
};

// 单连接会话；只负责字节/行/JSON-RPC 编解码与写缓冲，不含业务。
class McpSession {
public:
    explicit McpSession(Common::UniquePtr<Common::IpcChannel> channel,
                        McpToolDispatcher& dispatcher, uint32_t maxLineBytes);
    bool onReadable();                 // 读→组帧→逐行 dispatch
    bool onWritable();                 // flush 写缓冲
    bool sendJson(const Common::JsonValue& message);   // 入写缓冲
    void close();
private:
    Common::UniquePtr<Common::IpcChannel> _channel;
    Common::LineFramer _inFramer;
    Common::DynamicArray<uint8_t> _outBuffer;
    McpToolDispatcher& _dispatcher;
};

// 工具注册与分发；自动汇总工具 schema 供 tools/list。
class McpToolDispatcher {
public:
    bool registerTool(Common::SharedPtr<IMcpTool> tool);
    Common::SharedPtr<IMcpTool> findTool(const Common::String& name);
    Common::Return listTools(Common::JsonValue& out);
    Common::Return callTool(const Common::String& name,
                            const Common::JsonValue& arguments,
                            Common::JsonValue& result,
                            Common::JsonValue& error);
};

// 单工具抽象：schema 由实现类返回（Command as Tool）。
class IMcpTool {
public:
    virtual ~IMcpTool() = default;
    virtual Common::String name() const = 0;
    virtual Common::String description() const = 0;
    virtual Common::JsonValue inputSchema() const = 0;
    virtual Common::Return call(const Common::JsonValue& args,
                                Common::JsonValue& result,
                                Common::JsonValue& error) = 0;
};

// 工具实现：桥接 JSON 参数与 CommandManager::call(EventId, EventPacket)。
class CreateMeshTool : public IMcpTool { /* name()="CreateMesh"; call() 构造 EventPacket→CommandManager::call(EventId::CreateMesh, packet) */ };
class SetCameraTool  : public IMcpTool { /* mode=="fit"→FitView；否则 SetCamera 命令 */ };
class GetLogTool     : public IMcpTool { /* 直接读 Common::LogRingBuffer，同步返回 */ };

// JSON-RPC 编解码（无状态、可静态组合）。
class JsonRpcCodec {
public:
    static bool decode(const Common::String& line, Common::JsonValue& message, Common::String& error);
    static bool encode(const Common::JsonValue& message, Common::String& out);
    static Common::JsonValue makeError(int32_t code, const Common::String& message,
                                       const Common::JsonValue& data);
};

} // namespace EgLab::Platform
```

> 单例判定：仅 `McpServer` 需要 Singleton（进程内唯一服务门面）；`McpSession` 每连接一实例；`McpToolDispatcher` 由 `McpServer` 独占持有；工具类为注册到 dispatcher 的普通对象。

---

## 6. 线程模型（仅 Common 并发组件）

### 6.1 线程拓扑

- `McpServer::Impl` 启动 **1 个 IO 线程**（`Common::Thread`）运行 `IoPoller` 循环，负责：接受连接、读取字节、组帧、分发、flush 写缓冲。
- 可选：启动 **1 个 `Common::ThreadPool`（2 worker）** 承担 `tools/call` 中的参数校验与 JSON 序列化，避免重负载占用 IO 线程；P0 可先内联执行。
- 变更类工具不阻塞 IO 线程：其底层执行发生在**存量** `CommandBus` 工作线程（`std::thread`，不动）。

### 6.2 数据流

```
NamedPipeServer(accept) → IpcChannel → McpSession::onReadable()
  → LineFramer.next(line)
  → JsonRpcCodec.decode(message)
  → dispatcher.callTool(...)
        ├─ tools/list      → dispatcher 汇总 schema → sendJson(result)
        ├─ GetLog         → LogRingBuffer::query → sendJson(result)
        └─ CreateMesh/SetCamera
              → 构造 EventPacket
              → CommandManager::call(EventId, EventPacket)
              → （存量）MemoryQueue → CommandBus 工作线程执行
              → CommandFinishedMessage → McpServer 订阅 → sendJson(notification)
```

### 6.3 并发与缓冲

- 每会话写缓冲 `_outBuffer` 仅在 IO 线程访问 → 无需额外锁；若未来引入多线程写，用 `Common::Mutex` 保护。
- `LogRingBuffer` 由业务线程（CommandBus/LOG）追加、IO 线程查询 → 内部用 `Common::Mutex`（或 SPSC 环形缓冲）保护。
- 新增代码**不出现** `std::thread/std::mutex`；存量 `CommandManager::Impl::_worker` 与 `CommandBus` 的 `std::thread` 保持不动。

---

## 7. 与 Application / Context / CommandManager 的集成点与生命周期

### 7.1 集成点

| 集成点 | 位置 | 动作 |
|---|---|---|
| 启动 | `Application::Application()`，在 `CommandManager::instance()` 与 `IniManager::instance()` 初始化之后 | `McpServer::instance().start(McpServerConfig{...})`；`McpToolDispatcher` 注册 `CreateMeshTool/SetCameraTool/GetLogTool` |
| 工具分发 | `McpToolDispatcher::callTool` | 复用 `CommandManager::call(EventId, EventPacket)` 与 `CommandCatalog::commands(...)` |
| 日志采集 | `MessageCenter::emit` / `Common::Log` sink | 追加写入 `Common::LogRingBuffer` |
| Mesh 生成 | `CreateMeshCmd` → `RenderWork/MeshEngine` | Mesh-Expert 提供 `createPrimitive(geometryType,size,subdivisions,name)→nodeId` 服务 |
| 相机设置 | `SetCameraCmd` → `RenderWork` | 在渲染线程设置相机位置/目标/fov |
| 关闭 | `Application::~Application()` | **先** `McpServer::instance().stop()`（flush 通知、关闭连接）**再** `RenderWork::instance().stop()` / `UIWork::stop()` |

### 7.2 生命周期顺序

- 启动：`IniManager → CommandManager → McpServer.start() → Window/RenderWork/UIWork`
- 运行：主循环照常 `glfwWaitEventsTimeout`；MCP 独立 IO 线程并行运行，不触碰 GLFW。
- 关闭：`McpServer.stop() → RenderWork.stop() → UIWork.stop() → window terminate → IniManager.shutdown()`
- 异常：任一 McpSession 协议错误仅关闭该会话，不影响引擎主循环；`McpServer.stop()` 幂等。

---

## 8. Python Agent ↔ UI 接口契约（选型 + 事件）

### 8.1 选型：HTTP + SSE（推荐）

理由：
1. Agent 是本地单用户服务端，UI 是本地窗口；**流式**需求主要是 Agent→UI 单向（LLM 文本增量、工具状态）。
2. SSE 本质是「HTTP 响应 + text/event-stream + 换行分帧」，用 Python 标准库 `http.server` 即可实现，**零第三方依赖**；WebSocket 则需自研握手与帧处理器，复杂度高且无必要。
3. UI→Agent 用普通 `POST` 即可，不要求全双工高频推送。

### 8.2 端点

| 方法/路径 | 方向 | 用途 |
|---|---|---|
| `POST /v1/chat` | UI→Agent | 发送用户消息，返回 `{sessionId}` |
| `GET /v1/events?sessionId=` | UI←Agent | SSE 流 |
| `POST /v1/interrupt` | UI→Agent | 中断当前任务 |
| `GET /v1/health` | UI→Agent | 健康检查 |

### 8.3 SSE 事件类型（`event:` 字段 + JSON `data:`）

| event | 载荷（data） | 含义 |
|---|---|---|
| `status` | `{ "state": "connecting|ready|thinking|idle", "sessionId": "..." }` | 会话/Agent 状态 |
| `text_delta` | `{ "delta": "..." }` | LLM 流式文本增量 |
| `tool_call` | `{ "id": 7, "name": "CreateMesh", "arguments": {...} }` | 工具调用开始 |
| `tool_result` | `{ "id": 7, "ok": true, "result": {...} }` | 工具执行结果 |
| `task_progress` | `{ "current": 2, "total": 5, "label": "生成网格" }` | 任务进度 |
| `done` | `{ "sessionId": "..." }` | 本轮结束 |
| `error` | `{ "code": "...", "message": "..." }` | 错误终止 |

---

## 9. 分阶段实现里程碑与验收标准

### P0 — 协议 + Stub 联调（先打通链路骨架）
交付：
- Common 新增：`Json`/`JsonParser`/`JsonWriter`、`LineFramer`、`IpcChannel`+`NamedPipeServer/Client`、`IoPoller`（P0 可最小实现）。
- `McpServer`/`McpSession`/`JsonRpcCodec`/`McpToolDispatcher` 骨架，`tools/list` 静态返回 `mcp_tools.json` 的三工具 schema；`tools/call` 返回 stub 结果。
- Python `McpClient`（stdlib）：初始化、`tools/list`、`tools/call`、错误路径、超时重连。

验收：
1. Named Pipe 上 NDJSON 消息双向收发无半行/粘包问题；
2. `tools/list` 返回与 `AiClient/mcp_tools.json` 一致；
3. 错误码 -32700/-32601/-32000 可被 Python 端正确识别；
4. 新增代码 0 处 `std::` 底层组件违规。

### P1 — C++ MCP Server + 真实工具
交付：
- EventId/CommandParam/CommandCatalog 扩展（CreateMesh=200009、SetCamera=300005；`ParamFieldType::Float32` 与数组字段扩展）。
- `CreateMeshCmd`（MeshEngine 几何生成）、`SetCameraCmd`（RenderWork 相机设置）。
- `LogRingBuffer` + `MessageCenter/Log` 汇入 + `GetLogTool`。
- `CommandFinishedMessage` 通道与 `notifications/toolResult` 异步通知。

验收：
1. 「创建一个立方体」→ 场景出现网格且收到含 nodeId 的通知；
2. 「相机 fit / 相机 orbit(位置/目标/fov)」→ 视口正确变化；
3. `GetLog` 返回最近 INFO/ERROR 日志且过滤/条数生效；
4. DeepSeek 客户端经 MCP 完成自然语言→工具调用闭环。

### P2 — Python Agent + UI 打磨
交付：
- Agent 角色（Dev-Commander/Architect/Mesh-Expert）、DeepSeek V4 Pro/Flash 流式调用、SessionManager 上下文维护。
- `AgentHttpServer`（HTTP+SSE）与 UI 事件流。
- UI：流式对话、任务进度、工具调用可视化。

验收：
1. 多轮任务（如「建网格→改相机→查日志」）端到端跑通；
2. UI 实时展示 `text_delta/tool_call/tool_result/status`；
3. 中断/超时/断线恢复可用；
4. AI 模块全程独立进程，卸载后引擎不受影响。

---

## 10. 关键风险与对策

| 风险 | 对策 |
|---|---|
| Named Pipe Windows 专属 | 抽象 `IpcChannel`，保留 TCP 实现位；当前项目为 Windows_NT |
| `CommandManager::call` 单向无返回值 | 变更类工具「accepted + 异步通知」；只读类走查询服务 |
| `CommandCatalog` 元数据无法表达 float/数组 | P1 扩展 `ParamFieldType::Float32` 与 `ParamFieldDesc::arrayLength`；AI 工具 schema 以 MCP 注册表为准 |
| 协议单行 1 MiB 限制 | 工具参数均为小对象；GetLog 结果超限时截断 + 分页 `cursor` |
| 阻塞引擎主循环 | MCP 全部在独立 IO 线程；引擎执行仍在存量 CommandBus |

---

### 核心决策总结（≤10 条）

1. **三层解耦**：EnginePlatform（内嵌 C++ MCP Server）↔ EgLab-Agent（Python 进程）↔ AI Dialog UI（独立窗口），AI 侧完全独立进程、不侵入引擎核心。
2. **传输层选型**：C++↔Python 用 **Windows Named Pipe + NDJSON**（同机、零端口、零第三方、Python stdlib 可直接打开），抽象 `Common::IpcChannel` 预留 TCP 扩展位；不采用需自研帧处理的 WebSocket。
3. **协议**：JSON-RPC 2.0 over NDJSON（一行一条、`\n` 分帧、1 MiB 行上限），方法 `initialize / tools/list / tools/call` + 服务端通知 `notifications/toolResult`，错误码 -32700/-32600/-32601/-32602/-32603/-32000/-32001/-32002/-32003。
4. **CreateMesh → 新增 `EventId::CreateMesh=200009`**：新增 `CreateMeshParam/CreateMeshCmd`，注册进 CommandManager 与 CommandCatalog，委托 MeshEngine 生成几何体，nodeId 经异步通知回传。
5. **SetCamera → 混合映射**：`mode=="fit"` 复用现有 `FitView(100002)`；`orbit/fps` 新增 `EventId::SetCamera=300005` 与 `SetCameraParam/SetCameraCmd`（位置/目标/fov）；现有连续手势相机命令不参与。
6. **GetLog → 不走 fire-and-forget 命令总线**：由 `GetLogTool` 同步读新增 `Common::LogRingBuffer`（`MessageCenter::emit` 与 `Common::Log` 统一汇入），这是对「Command as Tool」的克制补充。
7. **C++ 类设计**：`McpServer : Common::Singleton<McpServer>` 为唯一服务门面；`McpSession`（每连接）、`McpToolDispatcher`、`IMcpTool` 及其三个工具实现类、`JsonRpcCodec` 组合协作；工具复用 `CommandManager::call(EventId, EventPacket)`。
8. **线程模型**：`McpServer` 以 `Common::Thread` 跑 IO 循环 + `Common::ThreadPool`（可选）做分发，新增代码零 `std::thread/std::mutex`；引擎执行仍在存量 CommandBus，IO 线程不阻塞。
9. **集成与生命周期**：`Application` 构造时 `McpServer::start()`、析构时先 `McpServer::stop()` 再停 RenderWork/UIWork；`Context` 不需改动，Mesh 生成服务由 MeshEngine 提供。
10. **Common 新增组件清单**（Foundation-Engineer）：`JsonValue/JsonParser/JsonWriter`、`LineFramer`、`IpcChannel/NamedPipeServer/NamedPipeClient`、`IoPoller`、`ThreadPool`、`Mutex/ConditionVariable`、`LogRingBuffer`；Python↔UI 用 **HTTP+SSE**（事件 `text_delta/tool_call/tool_result/status/task_progress/done/error`）。