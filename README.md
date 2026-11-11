# brightness · 便携屏亮度

面向 Windows x64 的极简 C 托盘工具。针对 **Z1 / DZX1560** 便携屏，通过 DDC/CI 调节真实背光，不影响笔记本内置屏幕。

**[下载最新版本](https://github.com/Ktclat/brightness/releases/latest)** · [MIT 协议](LICENSE)

## 使用

完整解压 Release 中的 ZIP，双击 `Brightness.exe`。无需安装 .NET、WinUI 或其他第三方运行库。程序没有主窗口，只显示托盘图标；图标可能位于任务栏的隐藏图标区。

点击或右键托盘图标打开菜单：

- **开机自启**：勾选或取消，状态同步保存到 INI 和当前用户 Windows 启动项。
- **默认亮度：xx%**：显示保存的目标亮度，展开可选择预设百分比并立即应用。
- **打开文件路径**：打开软件文件夹。
- **退出**：移除托盘图标并结束后台进程。

图标使用统一的单色亮度条，托盘适配明暗任务栏。后台等待显示器重连、唤醒事件，不持续轮询；重复运行不会产生多个实例。

> 本项目目前仅匹配 `DISPLAY#DZX1560#`，不保证适用于其他显示器。这里的“默认亮度”是用户保存的目标值，不是屏幕的出厂设置。

## 便携配置

所有配置位于 EXE 旁边的 `brightness.ini`：

```ini
[Settings]
Brightness=100
Resident=1
AutoStart=0
```

| 设置 | 含义 |
| --- | --- |
| `Brightness` | 0–100 的整数；超出范围回退到 100。 |
| `Resident` | 1：显示托盘并等待重连/唤醒；0：设置一次亮度后退出。 |
| `AutoStart` | 1：登录时启动；0：关闭，可从托盘菜单操作。 |

普通运行不生成日志，不向 AppData 写数据。请把软件放在可写文件夹。手动修改配置后重新启动，或等待下一次重连/唤醒重新读取亮度。

只有启用开机自启时需要当前用户注册表项：`HKCU\Software\Microsoft\Windows\CurrentVersion\Run\Z1SilentBrightness`。移动软件后请从新位置运行一次以更新路径；卸载前取消自启并退出，再删除文件夹。

## 命令行

```powershell
.\Brightness.exe --quit   # 退出驻留实例
.\Brightness.exe --once   # 应用一次 INI 亮度后退出，不显示托盘
.\Brightness.exe --check  # 当前亮度通过退出码返回
$LASTEXITCODE
```

`--check` 返回 0–100 的亮度；200 表示未发现目标屏幕，201 表示控制/确认失败，204 表示程序路径异常，205 表示初始化失败。普通设置操作成功返回 0。

## 从源码构建

需要 Windows x64、PowerShell，以及 MinGW-w64 的 `gcc`、`windres`（在 PATH 中）。推荐 MSYS2 UCRT64 的 `mingw-w64-ucrt-x86_64-gcc`。

```powershell
.\build.ps1
.\package.ps1
```

`build.ps1` 编译资源和 C 源码；使用 `-nostdlib` 和自定义入口，不依赖 C 运行库。输出 `Brightness.exe`。`package.ps1` 生成 `dist` 中的便携 ZIP、独立 EXE 和 SHA-256 校验文件。

`assets` 中包含已生成的图标。可在 Windows PowerShell 中运行 `generate-icons.ps1` 重新生成，图标同样采用 MIT 协议。

GitHub Actions 会在提交后构建并校验系统 DLL 依赖。发布工作流创建 `v1.2.0` Release，已有同名 Release 时不覆盖。

## 验证范围

本机已验证：Z1 的 75% 写入回读、恢复 100%、驻留和退出、单实例、开机自启写入/移除。EXE 约 35 KB；文件大小随编译器版本略有变化。

CI 验证构建、资源版本与 DLL 依赖，不在无显示器环境中宣称通过硬件测试。完整鼠标菜单交互、真实拔插、睡眠和重启恢复尚未全流程验证。
