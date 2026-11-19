'use strict';

const { app, BrowserWindow, dialog } = require('electron');
const { spawn } = require('child_process');
const http = require('http');
const path = require('path');

const HOST = '127.0.0.1';
const PORT = parseInt(process.env.EGLAB_AGENT_PORT || '8765', 10);
// 默认 native：直连运行中 EnginePlatform 的 CommandManager（真实命令执行）。
// stub=演示后端（不触达引擎），mcp=内嵌 MCP Server（P1 落地后使用）。
const BRIDGE = process.env.EGLAB_BRIDGE || 'native';
const MCP_PIPE = process.env.EGLAB_MCP_PIPE || 'eglab_mcp';

let agentProcess = null;
let mainWindow = null;

function agentScriptPath() {
  return app.isPackaged
    ? path.join(process.resourcesPath, 'AiClient', 'agent_server.py')
    : path.join(__dirname, '..', 'agent_server.py');
}

function pythonExecutable() {
  if (process.env.EGLAB_PYTHON) return process.env.EGLAB_PYTHON;
  // 打包后使用随应用捆绑的 Python 嵌入式运行时，双击 exe 即可直接启动。
  if (app.isPackaged) return path.join(process.resourcesPath, 'Python', 'python.exe');
  return 'python';
}

function healthCheck(cb) {
  const req = http.get(
    { host: HOST, port: PORT, path: '/api/health', timeout: 800 },
    (res) => { res.resume(); cb(true); }
  );
  req.on('error', () => cb(false));
  req.on('timeout', () => { req.destroy(); cb(false); });
}

function spawnAgent() {
  const args = [
    agentScriptPath(),
    '--host', HOST,
    '--port', String(PORT),
    '--bridge', BRIDGE,
    '--mcp-pipe', MCP_PIPE,
  ];
  const key = process.env.DEEPSEEK_API_KEY;
  if (key) args.push('--api-key', key);

  console.log('[main] 启动 Agent:', pythonExecutable(), args.join(' '));
  agentProcess = spawn(pythonExecutable(), args, {
    stdio: ['ignore', 'pipe', 'pipe'],
    windowsHide: true,
  });
  agentProcess.stdout.on('data', (d) => process.stdout.write('[agent] ' + d));
  agentProcess.stderr.on('data', (d) => process.stderr.write('[agent] ' + d));
  agentProcess.on('exit', (code) => {
    if (!app.isQuitting) console.error('[main] Agent 退出，code=', code);
    if (agentProcess) agentProcess = null;
  });
}

function ensureAgent(cb) {
  healthCheck((ok) => {
    if (ok) { cb(true); return; }
    if (!agentProcess) spawnAgent();
    let tries = 40;
    const poll = () => {
      healthCheck((ok2) => {
        if (ok2) { cb(true); return; }
        tries -= 1;
        if (tries <= 0) { cb(false); return; }
        setTimeout(poll, 250);
      });
    };
    setTimeout(poll, 250);
  });
}

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1180,
    height: 800,
    minWidth: 860,
    minHeight: 600,
    title: 'EgLab AI 对话',
    backgroundColor: '#0e1117',
    autoHideMenuBar: true,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
    },
  });
  mainWindow.loadURL('http://' + HOST + ':' + PORT + '/');
  mainWindow.on('closed', () => { mainWindow = null; });
}

app.whenReady().then(() => {
  ensureAgent((ok) => {
    if (!ok) {
      dialog.showErrorBox(
        'EgLab Agent 启动失败',
        '无法在 127.0.0.1:' + PORT + ' 启动本地 Agent。\n\n' +
        '请确认已安装 Python 3 并加入 PATH（或设置 EGLAB_PYTHON 指向 python.exe）。\n' +
        'DeepSeek API Key 可在应用内左下角“设置”中填写，无需环境变量。'
      );
      app.quit();
      return;
    }
    createWindow();
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});

app.on('before-quit', () => {
  app.isQuitting = true;
  if (agentProcess) {
    try { agentProcess.kill(); } catch (e) { /* ignore */ }
    agentProcess = null;
  }
});
