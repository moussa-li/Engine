'use strict';

// 下载并解压 Python 嵌入式运行时（python.org embeddable zip），
// 使打包后的 EgLabAIDialog.exe 无需用户预装 Python 即可直接运行。
//
// 环境变量可覆盖：
//   EGLAB_PY_VERSION   Python 版本（默认 3.12.10）
//   EGLAB_PYTHON_URL   完整下载地址（默认 npmmirror 镜像）

const https = require('https');
const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');

const PY_VERSION = process.env.EGLAB_PY_VERSION || '3.12.10';
const DEFAULT_URL =
  `https://registry.npmmirror.com/-/binary/python/${PY_VERSION}/python-${PY_VERSION}-embed-amd64.zip`;
const URL = process.env.EGLAB_PYTHON_URL || DEFAULT_URL;
const ROOT = path.join(__dirname, '..');
const DEST = path.join(ROOT, 'python');
const ZIP = path.join(ROOT, 'python-embed.zip');

function download(url, file, redirects) {
  redirects = redirects || 0;
  return new Promise((resolve, reject) => {
    const req = https.get(url, { headers: { 'User-Agent': 'eglab-build' } }, (res) => {
      if (res.statusCode >= 300 && res.statusCode < 400 && res.headers.location) {
        res.resume();
        if (redirects >= 8) {
          return reject(new Error('重定向次数过多'));
        }
        return resolve(download(res.headers.location, file, redirects + 1));
      }
      if (res.statusCode !== 200) {
        res.resume();
        return reject(new Error(`HTTP ${res.statusCode} for ${url}`));
      }
      const out = fs.createWriteStream(file);
      res.pipe(out);
      out.on('finish', () => out.close(resolve));
      out.on('error', reject);
    });
    req.on('error', reject);
    req.setTimeout(600000, () => req.destroy(new Error('下载超时')));
  });
}

function main() {
  if (fs.existsSync(path.join(DEST, 'python.exe'))) {
    console.log('[fetch-python] 已存在，跳过: ' + DEST);
    return;
  }
  console.log('[fetch-python] 下载: ' + URL);
  download(URL, ZIP)
    .then(() => {
      console.log('[fetch-python] 解压到: ' + DEST);
      fs.mkdirSync(DEST, { recursive: true });
      const ps = `Expand-Archive -Force -LiteralPath '${ZIP.replace(/'/g, "''")}' -DestinationPath '${DEST.replace(/'/g, "''")}'`;
      execFileSync('powershell', ['-NoProfile', '-Command', ps], { stdio: 'inherit' });
      fs.unlinkSync(ZIP);
      console.log('[fetch-python] 完成');
    })
    .catch((err) => {
      console.error('[fetch-python] 失败: ' + err.message);
      process.exit(1);
    });
}

main();
