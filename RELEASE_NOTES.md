# Brightness v1.3.0

- English executable metadata, tray labels, notifications and menu items.
- Formal file description and product name: **Brightness**. Company metadata: **Ktclat**; the EXE remains unsigned.
- New login startup value: `Brightness`; enabling startup removes legacy Run records.
- Added **Remove startup entries** and `--remove-startup`: save `AutoStart=0` and remove current/legacy records from HKCU Run and StartupApproved Run/Run32.
- Only this app's exact startup value names are removed; unrelated values and keys are preserved.
- Retains native C implementation, portable INI settings and MIT license.

Upgrade: exit the old version and replace its EXE, keeping `brightness.ini`. Select **Remove startup entries** to disable startup and clear stale records, then close and reopen Task Manager to refresh its display.
