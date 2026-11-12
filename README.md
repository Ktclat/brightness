# Brightness

A minimal native C tray utility for Windows x64, controlling the real backlight of a **Z1 / DZX1560** portable monitor through DDC/CI.

**[Download latest release](https://github.com/Ktclat/brightness/releases/latest)** · [MIT License](LICENSE)

## Usage

Extract the ZIP and run `Brightness.exe`. No .NET, WinUI, or third-party runtime is needed. There is no main window; the tray icon may be in the taskbar's hidden icons area.

- **Start with Windows**: enable or disable login startup.
- **Default brightness: xx%**: select, save, and apply a target brightness.
- **Open file location**: open the application folder.
- **Remove startup entries**: disable startup and clean current/legacy startup records, including Task Manager approval records.
- **Exit**: remove the icon and stop the app.

The executable file description and product name are **Brightness**, with company metadata **Ktclat**. All menu labels and notifications are English. The EXE remains unsigned: company metadata is not a verified digital publisher.

Only monitors matching `DISPLAY#DZX1560#` are supported. Default brightness is the saved user target, not the monitor's factory setting. The app waits for reconnect/resume events instead of continuously polling, and prevents duplicate resident instances.

## Portable settings

`brightness.ini` stays beside the EXE. Normal operation writes no logs and stores no settings in AppData.

```ini
[Settings]
Brightness=100
Resident=1
AutoStart=0
```

- `Brightness`: integer 0–100; invalid ranges fall back to 100.
- `Resident`: 1 enables the tray and background restoration; 0 applies once and exits.
- `AutoStart`: 1 enables login startup; 0 disables it. Also controlled by the menu.

Use a writable folder. Restart after manual configuration changes. After moving the folder, enable startup again if desired to update the path.

## Startup cleanup

The new startup value is `Brightness`. Cleanup removes only the exact value names `Brightness`, `Z1SilentBrightness`, and `PortableBrightnessZ1` from these **current-user** paths:

```text
HKCU\Software\Microsoft\Windows\CurrentVersion\Run
HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run
HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run32
```

Unrelated values and entire registry keys are not deleted. Cleanup saves `AutoStart=0` and leaves the app running. Use **Exit** to stop it. Close and reopen Task Manager to refresh cached names. Old versions retain old file descriptions until their EXE is replaced.

Before uninstalling, remove startup entries, exit, then delete the folder.

## Commands

```powershell
.\Brightness.exe --remove-startup # Remove startup records and save AutoStart=0
.\Brightness.exe --quit           # Stop the resident instance, including the old C version
.\Brightness.exe --once           # Apply brightness once without a tray icon
.\Brightness.exe --check          # Brightness is returned in the exit code
$LASTEXITCODE
```

`--check`: 0–100 = brightness. Errors: 200 = monitor not found, 201 = hardware control/verification failed, 204 = invalid executable path, 205 = initialization failed, 206 = startup cleanup or INI update failed. Normal successful operations return 0.

## Build

Windows x64, PowerShell, MinGW-w64 `gcc` and `windres` in PATH are required. MSYS2 UCRT64's `mingw-w64-ucrt-x86_64-gcc` is recommended.

```powershell
.\build.ps1
.\package.ps1
```

Uses `-nostdlib` and a custom entry point, importing only Windows system DLLs. Output: `Brightness.exe`; packaging creates ZIP, EXE and SHA-256 checksums in `dist`. Icons and `generate-icons.ps1` are included under MIT.

GitHub Actions builds and verifies system-only imports and English version metadata, then publishes `v1.3.0` without replacing existing releases.

## Validation

Tested on the target machine: 75% write/readback, restore 100%, resident startup/exit, single instance and startup registration/removal. This release also checks English metadata and cleanup of current/legacy Run and StartupApproved records. Full mouse menu, physical reconnect, sleep and reboot scenarios have not been tested end to end.
