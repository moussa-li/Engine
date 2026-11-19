'use strict';

// 渲染进程与主进程隔离的预加载脚本。当前仅暴露只读平台信息，后续可在此
// 通过 contextBridge 暴露受控 IPC（如打开 EnginePlatform 可执行文件等）。
const { contextBridge } = require('electron');

contextBridge.exposeInMainWorld('eglab', {
  platform: process.platform,
  agentPort: parseInt(process.env.EGLAB_AGENT_PORT || '8765', 10),
});
