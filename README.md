# claude-code-termux 🚀

在 Termux 上原生运行 **Claude Code**，无需 proot、无需 glibc-runner，基于 Bun 运行时原生执行。

Native **Claude Code** on Termux — no proot, no glibc-runner, powered by the Bun runtime.

## ⚠️ 警告 / Warning

> **由于本修改版脱离了官方二进制，部分风控机制会被跳过。请避免在本修改版中登录官方账号，否则可能导致账号被封禁或触发风控策略，后果自负。**
>
> *This patched version bypasses the official binary and some security controls may be skipped. **Do not log in with your official account** — doing so may result in account suspension or triggering risk controls. Use at your own risk.*

---

## ✨ 特性 / Features

- **⚡ 原生性能** — 直接在 Termux 中运行，无需 chroot/proot 开销
- **🧩 原生 N-API 插件** — 内置预编译的 glibc→bionic 垫片库，Claude Code内置的`image-processor.node`、`audio-capture.node` 开箱即用
- **📦 离线安装** — 下载 tgz 后直接 `npm i -g`，无需联网

---

## 📋 要求 / Requirements

| 依赖 | 说明 |
|------|------|
| **Termux** | 最新版本，aarch64 架构 |
| **Bun** | 运行时必需（见下方安装指引） |
| **ripgrep**（推荐） | `pkg install ripgrep` |
| **alsa-lib**（可选） | `pkg install alsa-lib`，音频功能需要 |

---

## 📥 安装 / Installation

### 1. 安装 Bun

Termux pkg 没有官方的 Bun 包，需要手动安装 Android aarch64 版本：

```bash
cd "${TMPDIR:-/tmp}"
curl -L -o bun.zip \
  https://github.com/oven-sh/bun/releases/latest/download/bun-linux-aarch64-android.zip
unzip -j bun.zip '*/bun' -d $PREFIX/bin && chmod +x $PREFIX/bin/bun
```

验证安装：

```bash
bun --version
```

### 2. 安装 Claude Code

从 [Releases](https://github.com/naiyQAQ/claude-code-termux/releases) 页面下载最新的 `anthropic-ai-claude-code-<version>-termux.tgz`，然后在本地执行：

```bash
npm i -g /path/to/anthropic-ai-claude-code-<version>-termux.tgz
```

安装包内已预编译好所有垫片库，无需反复编译。

### 3. 运行

```bash
claude
```

---

## 🔄 同步更新 / Keeping Updated

本仓库会跟随 [@anthropic-ai/claude-code](https://www.npmjs.com/package/@anthropic-ai/claude-code) 官方版本同步更新，在 [Releases](https://github.com/naiyQAQ/claude-code-termux/releases) 中发布已打补丁的安装包。

> 需要某个特定版本而仓库尚未同步？欢迎提交 Issue。

---

## 📄 许可 / License

本仓库中的兼容层源码采用 **MIT License** 授权。Claude Code 本身由 [Anthropic](https://www.anthropic.com/) 版权所有，请遵守其原始许可条款。

---

## 🙏 致谢 / Credits

- [Anthropic](https://www.anthropic.com/) — Claude Code
- [Bun](https://bun.sh/) — 跨平台 JavaScript 运行时
- [Termux](https://termux.dev/) — Android 终端模拟器