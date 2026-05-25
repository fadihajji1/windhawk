# MicSwap
Instantly toggle between two audio input devices (microphones) from your system tray — no diving into Sound settings.

---

## How to Use

1. **Choose Icons** — Open the **Settings** tab and pick an icon for each of your two devices.
2. **Select Your Devices** — Right-click the tray icon. Use **Set as Input 1** and **Set as Input 2** to assign your inputs from the live device list.
3. **Toggle** — Left-click the tray icon at any time to swap between the two devices instantly.

> The tray tooltip always shows the active input. On first run it will read *"Right-click to configure"* until both devices are assigned.

---

## Changelog

### v1.1.0
- Fix: Separate tray icon from Windhawk process grouping
  - Add unique GUID (GUID_MicSwitcherTrayIcon) to tray icon to prevent Windows from grouping it with Windhawk's process
  - Include NIF_GUID flag in Shell_NotifyIcon calls for both ADD/MODIFY and DELETE operations
  - Set SetCurrentProcessExplicitAppUserModelID early in WhTool_ModInit() before window creation
  - Use unique AppUserModelID ("BlackPaw.MicSwitcher.ToolMod") for tool process to ensure proper taskbar grouping
  - Add #include <initguid.h> for GUID macro support
  - Ensure window property store also uses tool-specific AppUserModelID
  - Resolves issue where dragging the mic icon to the taskbar would attach it to Windhawk's icon instead of creating a separate taskbar entry

### v1.0.0
- Initial release. Mirrors AudioSwap but targets audio input/capture devices.
- Right-click context menu auto-detects all active audio inputs and lets you assign Input 1 and Input 2 from a live list.
- Device selections persist across restarts.
- Toggle matches devices by their unique system ID for reliable switching regardless of device naming.
