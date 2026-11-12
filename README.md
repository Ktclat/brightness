# Brightness

极简 C 语言 Windows x64 托盘工具，调节 Z1 / DZX1560 便携屏的真实背光。

**软件名称、EXE 文件属性和启动项名称始终使用英文 Brightness；菜单、提示和使用说明使用简体中文。**

[下载最新版](https://github.com/Ktclat/brightness/releases/latest) · [MIT 协议](LICENSE)

## 使用

完整解压 ZIP 后运行 `Brightness.exe`。无需安装 .NET、WinUI 或第三方运行库，没有主窗口。托盘图标可能位于任务栏隐藏图标区。

点击或右键托盘图标：

- **开机自启**：勾选或取消登录启动。
- **默认亮度：xx%**：展开选择预设，保存并立即应用。
- **打开文件路径**：打开软件所在文件夹。
- **删除启动注册信息**：关闭自启，清除本软件当前及旧版的启动记录。
- **退出**：移除托盘图标并结束进程。

默认亮度是用户保存的目标值，不是屏幕出厂值。当前仅匹配 `DISPLAY#DZX1560#`，不调节笔记本内置屏幕。后台等待重连、唤醒事件，不持续轮询；同一时间只运行一个驻留实例。

## 便携配置

配置在 EXE 旁边的 `brightness.ini`，不写入 AppData：

```ini
[Settings]
Brightness=100
Resident=1
AutoStart=0
```

- `Brightness`：0–100 的整数；超出范围时回退到 100。
- `Resident`：1 为托盘驻留，0 为设置一次后退出。
- `AutoStart`：1 开启登录启动，0 关闭，也可从菜单切换。

请使用可写文件夹。手动编辑配置后重启程序；亮度也会在下一次重连/唤醒时重读。移动文件夹后，如需自启，请重新启用以更新路径。

## 清理启动项

当前启动项名称为英文 `Brightness`。清理功能仅删除 `Brightness`、`Z1SilentBrightness`、`PortableBrightnessZ1` 这三个名称在以下当前用户位置的值：

```text
HKCU\Software\Microsoft\Windows\CurrentVersion\Run
HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run
HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run32
```

不删除整个注册表键，不影响其他软件。清理会保存 `AutoStart=0`，但保留程序运行；需要停止时选择“退出”。任务管理器显示可能有缓存，关闭并重新打开后刷新。卸载前先清理启动项并退出，再删除软件文件夹。

文件描述和产品名为 **Brightness**，公司元数据为 **Ktclat**。程序未做数字签名，公司字段不代表经过验证的发布者。

## 命令行

```powershell
.\Brightness.exe --remove-startup # 删除本软件启动注册信息
.\Brightness.exe --quit           # 退出驻留实例（兼容旧 C 版）
.\Brightness.exe --once           # 设置一次亮度后退出，无托盘
.\Brightness.exe --check          # 当前亮度通过退出码返回
$LASTEXITCODE
```

`--check` 返回 0–100 为亮度。错误码：200 未找到目标屏幕，201 硬件控制/验证失败，204 路径异常，205 初始化失败，206 启动项清理或配置写入失败。普通操作成功返回 0。

## 构建

需要 Windows x64、PowerShell，以及 PATH 中的 MinGW-w64 `gcc` 和 `windres`。推荐 MSYS2 UCRT64 的 `mingw-w64-ucrt-x86_64-gcc`。

```powershell
.\build.ps1
.\package.ps1
```

使用自定义入口和 `-nostdlib`，仅依赖 Windows 系统 DLL。构建输出 `Brightness.exe`；打包结果包含 ZIP、独立 EXE 和 SHA-256 校验文件。图标和生成脚本同样使用 MIT 协议。

GitHub Actions 自动构建、检查依赖及英文软件名称元数据，并发布 `v1.3.1`，不覆盖已有 Release。

## 验证范围

本机已验证硬件亮度读写、恢复 100%、单实例、驻留退出、自启写入/删除及旧版启动项清理。完整鼠标菜单、实际拔插、睡眠和重启场景尚未全流程验证。
