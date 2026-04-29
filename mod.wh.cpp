// ==WindhawkMod==
// @id              explorer-disk-bar-style
// @name            Explorer Disk Usage Bar Style
// @description     Change color and border radius of disk usage bars
// @version         1.2
// @author          fad
// @compilerOptions -lgdi32  
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- name: barColor
  type: string
  label: Bar Color (HEX like #FF0000)
  default: "#00C8FF"

- name: cornerRadius
  type: int
  label: Corner Radius (px)
  default: 8
  min: 0
  max: 30
*/
// ==WindhawkModSettings==

#include <windows.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <wingdi.h>
#include <vssym32.h>   // فيه PP_FILL و PP_CHUNK

#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")

// ==========================
// Global variables (settings)
// ==========================
COLORREF g_barColor = RGB(0, 200, 255); // default color
int g_cornerRadius = 8;                 // default radius

// ==========================
// Convert HEX "#RRGGBB" → COLORREF
// ==========================
COLORREF HexToColor(const wchar_t* hex) {
    // Validate format
    if (!hex || wcslen(hex) != 7 || hex[0] != L'#')
        return RGB(0, 200, 255); // fallback

    // Convert hex string to integer
    int value = wcstol(hex + 1, nullptr, 16);

    int r = (value >> 16) & 0xFF;
    int g = (value >> 8) & 0xFF;
    int b = value & 0xFF;

    return RGB(r, g, b);
}

// ==========================
// Function pointer typedef
// (original Windows function)
// ==========================
typedef HRESULT (WINAPI *DrawThemeBackground_t)(
    HTHEME hTheme,
    HDC hdc,
    int iPartId,
    int iStateId,
    const RECT *pRect,
    const RECT *pClipRect
);

// Pointer to original function
DrawThemeBackground_t OriginalDrawThemeBackground = nullptr;

// ==========================
// Hooked function
// This intercepts Windows drawing
// ==========================
HRESULT WINAPI HookedDrawThemeBackground(
    HTHEME hTheme,
    HDC hdc,
    int iPartId,
    int iStateId,
    const RECT *pRect,
    const RECT *pClipRect
) {
    // Safety check (avoid crashes)
    if (!pRect || !hdc) {
        return OriginalDrawThemeBackground(
            hTheme, hdc, iPartId, iStateId, pRect, pClipRect
        );
    }

    // 🎯 Target progress bar fill parts
    // These are used for disk usage bars in Explorer
    if (iPartId == PP_FILL || iPartId == PP_CHUNK) {

        // Create a solid brush with chosen color
        HBRUSH brush = CreateSolidBrush(g_barColor);
        if (!brush) {
            return OriginalDrawThemeBackground(
                hTheme, hdc, iPartId, iStateId, pRect, pClipRect
            );
        }

        // Create a "null pen" → removes border
        HPEN pen = CreatePen(PS_NULL, 0, 0);
        if (!pen) {
            DeleteObject(brush);
            return OriginalDrawThemeBackground(
                hTheme, hdc, iPartId, iStateId, pRect, pClipRect
            );
        }

        // Select our custom brush + pen into device context
        HGDIOBJ oldPen = ::SelectObject(hdc, pen);
        HGDIOBJ oldBrush = ::SelectObject(hdc, brush);

        // Draw rounded rectangle (this replaces default bar)
        ::RoundRect(
            hdc,
            pRect->left,
            pRect->top,
            pRect->right,
            pRect->bottom,
            g_cornerRadius,
            g_cornerRadius
        );

        // Restore previous objects (VERY IMPORTANT)
        ::SelectObject(hdc, oldBrush);
        ::SelectObject(hdc, oldPen);

        // Cleanup GDI objects (avoid memory leaks)
        ::DeleteObject(brush);
        ::DeleteObject(pen);

        return S_OK; // tell Windows we handled drawing
    }

    // Default behavior for everything else
    return OriginalDrawThemeBackground(
        hTheme, hdc, iPartId, iStateId, pRect, pClipRect
    );
}

// ==========================
// Load settings from Windhawk UI
// ==========================
void LoadSettings() {
    // Get HEX color string from UI
    PCWSTR colorStr = Wh_GetStringSetting(L"barColor");
    g_barColor = HexToColor(colorStr);

    // Get radius from UI
    g_cornerRadius = Wh_GetIntSetting(L"cornerRadius");
}

// ==========================
// Called when mod starts
// ==========================
BOOL Wh_ModInit() {
    Wh_Log(L"[DiskBarStyle] Initializing");

    // Load user settings
    LoadSettings();

    // Load uxtheme.dll (contains DrawThemeBackground)
    HMODULE hUxTheme = LoadLibraryW(L"uxtheme.dll");
    if (!hUxTheme) {
        Wh_Log(L"Failed to load uxtheme.dll");
        return FALSE;
    }

    // Get original function address
    OriginalDrawThemeBackground = (DrawThemeBackground_t)
        GetProcAddress(hUxTheme, "DrawThemeBackground");

    if (!OriginalDrawThemeBackground) {
        Wh_Log(L"Failed to find DrawThemeBackground");
        return FALSE;
    }

    // Install hook
    Wh_SetFunctionHook(
        (void*)OriginalDrawThemeBackground,
        (void*)HookedDrawThemeBackground,
        (void**)&OriginalDrawThemeBackground
    );

    return TRUE;
}

// ==========================
// Called when settings change in UI
// ==========================
void Wh_ModSettingsChanged() {
    Wh_Log(L"[DiskBarStyle] Settings updated");

    // Reload settings without restarting Explorer
    LoadSettings();
}

// ==========================
// Cleanup when mod unloads
// ==========================
void Wh_ModUninit() {
    Wh_Log(L"[DiskBarStyle] Unloaded");
}