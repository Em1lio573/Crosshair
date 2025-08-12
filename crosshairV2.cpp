// main.cpp
//
// Para compilar este código usando g++ (MinGW), usa el siguiente comando:
// g++ main.cpp -o Crosshair.exe -lgdi32 -lcomdlg32 -lshlwapi -mwindows
//
// Para compilarlo en Visual Studio, simplemente crea un nuevo proyecto de "Aplicación de Windows"
// y pega este código.

#pragma execution_character_set("utf-8") // Asegura que los caracteres especiales se muestren correctamente

#include <windows.h>
#include <commctrl.h> // Para los controles comunes (como los sliders)
#include <commdlg.h>  // Para el diálogo de selección de color
#include <wchar.h>    // Para _wtoi
#include <ShlObj.h>   // Para SHGetFolderPathW
#include <shlwapi.h>  // Para PathAppendW
#include <cstdio>     // Para funciones de archivo como fwrite, fread, etc.

#pragma comment(lib, "shlwapi.lib") // Enlaza shlwapi.lib para Visual Studio

// --- Constantes y Enums ---
#define MAX_PRESETS 5

enum CrosshairShape {
    SHAPE_CROSS,
    SHAPE_CIRCLE,
    SHAPE_DOT
};

// --- Estructuras de Datos ---
struct CrosshairSettings {
    CrosshairShape shape;
    int size;
    int thickness;
    bool centerDot;
    COLORREF color;
    COLORREF customColors[16];
};

struct AllSettings {
    int activePresetIndex;
    CrosshairSettings presets[MAX_PRESETS];
};

// --- Variables Globales ---
AllSettings g_settings;
HWND g_hOverlayWnd = NULL;
HFONT g_hFont = NULL;

// --- IDs de Controles ---
#define MAIN_ICON 101
#define IDC_SLIDER_SIZE 102
#define IDC_SLIDER_THICKNESS 103
#define IDC_BUTTON_COLOR 104
#define IDC_CHECKBOX_DOT 105
#define IDC_COMBO_SHAPE 106
#define IDC_EDIT_SIZE 107
#define IDC_EDIT_THICKNESS 108
#define IDC_BUTTON_INFO 109
#define IDC_COMBO_PRESETS 110
#define IDC_BUTTON_SAVE_PRESET 111
#define IDC_BUTTON_RESET_PRESET 112

// --- Prototipos de funciones ---
LRESULT CALLBACK OverlayWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SettingsWndProc(HWND, UINT, WPARAM, LPARAM);
void SaveAllSettings();
void LoadAllSettings();
void UpdateAllControls(HWND);
void GetDefaultPreset(CrosshairSettings* preset);

// --- Funciones de Guardado y Carga ---
void GetConfigPath(WCHAR* pathBuffer, DWORD bufferSize) {
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, pathBuffer))) {
        PathAppendW(pathBuffer, L"\\CrosshairOverlay");
        CreateDirectoryW(pathBuffer, NULL);
        PathAppendW(pathBuffer, L"\\settings.dat");
    }
}

void SaveAllSettings() {
    WCHAR path[MAX_PATH];
    GetConfigPath(path, MAX_PATH);
    FILE* file;
    _wfopen_s(&file, path, L"wb");
    if (file != NULL) {
        fwrite(&g_settings, sizeof(AllSettings), 1, file);
        fclose(file);
    }
}

void LoadAllSettings() {
    WCHAR path[MAX_PATH];
    GetConfigPath(path, MAX_PATH);
    FILE* file;
    _wfopen_s(&file, path, L"rb");
    if (file != NULL && fread(&g_settings, sizeof(AllSettings), 1, file) == 1) {
        fclose(file);
    } else {
        g_settings.activePresetIndex = 0;
        for (int i = 0; i < MAX_PRESETS; ++i) {
            GetDefaultPreset(&g_settings.presets[i]);
        }
    }
}

// --- Otras Funciones de Ayuda ---
void GetDefaultPreset(CrosshairSettings* preset) {
    preset->shape = SHAPE_CROSS;
    preset->size = 1;
    preset->thickness = 3;
    preset->centerDot = false;
    preset->color = RGB(0, 255, 0);
    ZeroMemory(preset->customColors, sizeof(preset->customColors));
}

void OpenColorDialog(HWND hWnd) {
    CHOOSECOLORW cc;
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hWnd;
    cc.lpCustColors = g_settings.presets[g_settings.activePresetIndex].customColors;
    cc.rgbResult = g_settings.presets[g_settings.activePresetIndex].color;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColorW(&cc) == TRUE) {
        g_settings.presets[g_settings.activePresetIndex].color = cc.rgbResult;
    }
}

// --- Punto de Entrada Principal ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    LoadAllSettings();

    g_hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    // Registrar clases de ventana
    const WCHAR OVERLAY_CLASS_NAME[] = L"CrosshairOverlayClass";
    WNDCLASSW wcOverlay = {};
    wcOverlay.lpfnWndProc = OverlayWndProc;
    wcOverlay.hInstance = hInstance;
    wcOverlay.lpszClassName = OVERLAY_CLASS_NAME;
    wcOverlay.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wcOverlay.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClassW(&wcOverlay);

    const WCHAR SETTINGS_CLASS_NAME[] = L"CrosshairSettingsClass";
    WNDCLASSW wcSettings = {};
    wcSettings.lpfnWndProc = SettingsWndProc;
    wcSettings.hInstance = hInstance;
    wcSettings.lpszClassName = SETTINGS_CLASS_NAME;
    wcSettings.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcSettings.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcSettings.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(MAIN_ICON));
    RegisterClassW(&wcSettings);

    // Crear ventanas
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    g_hOverlayWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST, OVERLAY_CLASS_NAME, L"Mira Overlay", WS_POPUP, 0, 0, screenWidth, screenHeight, NULL, NULL, hInstance, NULL);
    SetLayeredWindowAttributes(g_hOverlayWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(g_hOverlayWnd, nCmdShow);
    UpdateWindow(g_hOverlayWnd);

    HWND hSettingsWnd = CreateWindowExW(WS_EX_TOPMOST, SETTINGS_CLASS_NAME, L"Panel de Ajustes", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 400, 320, NULL, NULL, hInstance, NULL);
    ShowWindow(hSettingsWnd, nCmdShow);
    UpdateWindow(hSettingsWnd);

    // Bucle de Mensajes
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(g_hFont);
    return 0;
}

// --- Procedimiento de la Ventana de la MIRA (Overlay) ---
LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            CrosshairSettings& currentPreset = g_settings.presets[g_settings.activePresetIndex];
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            int centerX = rect.right / 2;
            int centerY = rect.bottom / 2;

            HPEN hPen = CreatePen(PS_SOLID, currentPreset.thickness, currentPreset.color);
            HBRUSH hBrush = CreateSolidBrush(currentPreset.color);
            HGDIOBJ hOldPen = SelectObject(hdc, hPen);
            HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

            switch (currentPreset.shape) {
                case SHAPE_CROSS:
                    MoveToEx(hdc, centerX - currentPreset.size, centerY, NULL);
                    LineTo(hdc, centerX + currentPreset.size, centerY);
                    MoveToEx(hdc, centerX, centerY - currentPreset.size, NULL);
                    LineTo(hdc, centerX, centerY + currentPreset.size);
                    if (currentPreset.centerDot) {
                        SelectObject(hdc, hBrush);
                        Ellipse(hdc, centerX - currentPreset.thickness, centerY - currentPreset.thickness, centerX + currentPreset.thickness + 1, centerY + currentPreset.thickness + 1);
                    }
                    break;
                case SHAPE_CIRCLE:
                    Ellipse(hdc, centerX - currentPreset.size, centerY - currentPreset.size, centerX + currentPreset.size, centerY + currentPreset.size);
                    if (currentPreset.centerDot) {
                        SelectObject(hdc, hBrush);
                        Ellipse(hdc, centerX - currentPreset.thickness, centerY - currentPreset.thickness, centerX + currentPreset.thickness + 1, centerY + currentPreset.thickness + 1);
                    }
                    break;
                case SHAPE_DOT:
                    SelectObject(hdc, hBrush);
                    Ellipse(hdc, centerX - currentPreset.size, centerY - currentPreset.size, centerX + currentPreset.size, centerY + currentPreset.size);
                    break;
            }

            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hPen);
            DeleteObject(hBrush);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

// --- Procedimiento de la Ventana de AJUSTES ---
void UpdateAllControls(HWND hwnd) {
    CrosshairSettings& currentPreset = g_settings.presets[g_settings.activePresetIndex];
    WCHAR buffer[50];

    SendMessageW(GetDlgItem(hwnd, IDC_SLIDER_SIZE), TBM_SETPOS, TRUE, currentPreset.size);
    wsprintfW(buffer, L"%d", currentPreset.size);
    SetDlgItemTextW(hwnd, IDC_EDIT_SIZE, buffer);

    SendMessageW(GetDlgItem(hwnd, IDC_SLIDER_THICKNESS), TBM_SETPOS, TRUE, currentPreset.thickness);
    wsprintfW(buffer, L"%d", currentPreset.thickness);
    SetDlgItemTextW(hwnd, IDC_EDIT_THICKNESS, buffer);

    SendMessageW(GetDlgItem(hwnd, IDC_CHECKBOX_DOT), BM_SETCHECK, currentPreset.centerDot ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(GetDlgItem(hwnd, IDC_COMBO_SHAPE), CB_SETCURSEL, (WPARAM)currentPreset.shape, 0);
    SendMessageW(GetDlgItem(hwnd, IDC_COMBO_PRESETS), CB_SETCURSEL, (WPARAM)g_settings.activePresetIndex, 0);
}

LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // --- Controles de Preset ---
            HWND hLabelPresets = CreateWindowW(L"STATIC", L"Preset:", WS_CHILD | WS_VISIBLE, 20, 20, 80, 20, hwnd, NULL, NULL, NULL);
            HWND hComboPresets = CreateWindowW(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 100, 20, 150, 120, hwnd, (HMENU)IDC_COMBO_PRESETS, NULL, NULL);
            for (int i = 0; i < MAX_PRESETS; ++i) {
                WCHAR buffer[20];
                wsprintfW(buffer, L"Preset %d", i + 1);
                SendMessageW(hComboPresets, CB_ADDSTRING, 0, (LPARAM)buffer);
            }
            HWND hBtnSavePreset = CreateWindowW(L"BUTTON", L"Guardar Preset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 260, 20, 120, 25, hwnd, (HMENU)IDC_BUTTON_SAVE_PRESET, NULL, NULL);
            HWND hBtnResetPreset = CreateWindowW(L"BUTTON", L"Resetear Preset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 260, 50, 120, 25, hwnd, (HMENU)IDC_BUTTON_RESET_PRESET, NULL, NULL);

            // --- Controles de Ajustes ---
            HWND hLabelSize = CreateWindowW(L"STATIC", L"Tamaño:", WS_CHILD | WS_VISIBLE, 20, 90, 80, 20, hwnd, NULL, NULL, NULL);
            HWND hSize = CreateWindowW(L"msctls_trackbar32", L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS, 100, 90, 150, 30, hwnd, (HMENU)IDC_SLIDER_SIZE, NULL, NULL);
            SendMessageW(hSize, TBM_SETRANGE, TRUE, MAKELPARAM(1, 50));
            HWND hEditSize = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 260, 90, 50, 20, hwnd, (HMENU)IDC_EDIT_SIZE, NULL, NULL);

            HWND hLabelThick = CreateWindowW(L"STATIC", L"Grosor:", WS_CHILD | WS_VISIBLE, 20, 130, 80, 20, hwnd, NULL, NULL, NULL);
            HWND hThick = CreateWindowW(L"msctls_trackbar32", L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS, 100, 130, 150, 30, hwnd, (HMENU)IDC_SLIDER_THICKNESS, NULL, NULL);
            SendMessageW(hThick, TBM_SETRANGE, TRUE, MAKELPARAM(1, 10));
            HWND hEditThick = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 260, 130, 50, 20, hwnd, (HMENU)IDC_EDIT_THICKNESS, NULL, NULL);

            HWND hBtnColor = CreateWindowW(L"BUTTON", L"Cambiar Color", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 170, 120, 30, hwnd, (HMENU)IDC_BUTTON_COLOR, NULL, NULL);
            HWND hDot = CreateWindowW(L"BUTTON", L"Punto Central", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 150, 170, 120, 30, hwnd, (HMENU)IDC_CHECKBOX_DOT, NULL, NULL);

            HWND hLabelShape = CreateWindowW(L"STATIC", L"Forma:", WS_CHILD | WS_VISIBLE, 20, 210, 80, 20, hwnd, NULL, NULL, NULL);
            HWND hShape = CreateWindowW(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 100, 210, 150, 100, hwnd, (HMENU)IDC_COMBO_SHAPE, NULL, NULL);
            SendMessageW(hShape, CB_ADDSTRING, 0, (LPARAM)L"Cruz");
            SendMessageW(hShape, CB_ADDSTRING, 0, (LPARAM)L"Círculo");
            SendMessageW(hShape, CB_ADDSTRING, 0, (LPARAM)L"Punto");
            
            HWND hBtnInfo = CreateWindowW(L"BUTTON", L"?", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 350, 240, 25, 25, hwnd, (HMENU)IDC_BUTTON_INFO, NULL, NULL);

            // Aplicar fuentes
            SendMessage(hLabelPresets, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hComboPresets, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hBtnSavePreset, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hBtnResetPreset, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hLabelSize, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hEditSize, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hLabelThick, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hEditThick, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hBtnColor, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hDot, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hLabelShape, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hShape, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            SendMessage(hBtnInfo, WM_SETFONT, (WPARAM)g_hFont, TRUE);

            UpdateAllControls(hwnd);
            break;
        }
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE) {
                ShowWindow(hwnd, SW_MINIMIZE);
            }
            break;
        case WM_HSCROLL: {
            CrosshairSettings& currentPreset = g_settings.presets[g_settings.activePresetIndex];
            if (LOWORD(wParam) == TB_THUMBTRACK || LOWORD(wParam) == TB_ENDTRACK) {
                WCHAR buffer[10];
                if ((HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_SIZE)) {
                    currentPreset.size = SendMessageW((HWND)lParam, TBM_GETPOS, 0, 0);
                    wsprintfW(buffer, L"%d", currentPreset.size);
                    SetDlgItemTextW(hwnd, IDC_EDIT_SIZE, buffer);
                }
                if ((HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_THICKNESS)) {
                    currentPreset.thickness = SendMessageW((HWND)lParam, TBM_GETPOS, 0, 0);
                    wsprintfW(buffer, L"%d", currentPreset.thickness);
                    SetDlgItemTextW(hwnd, IDC_EDIT_THICKNESS, buffer);
                }
                InvalidateRect(g_hOverlayWnd, NULL, TRUE);
            }
            break;
        }
        case WM_COMMAND: {
            CrosshairSettings& currentPreset = g_settings.presets[g_settings.activePresetIndex];
            
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                int controlId = LOWORD(wParam);
                if (controlId == IDC_COMBO_PRESETS) {
                    g_settings.activePresetIndex = SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0);
                    UpdateAllControls(hwnd);
                    InvalidateRect(g_hOverlayWnd, NULL, TRUE);
                } else if (controlId == IDC_COMBO_SHAPE) {
                    currentPreset.shape = (CrosshairShape)SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0);
                    InvalidateRect(g_hOverlayWnd, NULL, TRUE);
                }
                return 0;
            }

            if (HIWORD(wParam) == EN_CHANGE) {
                int controlId = LOWORD(wParam);
                WCHAR buffer[10];
                GetWindowTextW((HWND)lParam, buffer, 10);
                int value = _wtoi(buffer);
                if (controlId == IDC_EDIT_SIZE && value >= 1 && value <= 50) {
                    currentPreset.size = value;
                    SendMessageW(GetDlgItem(hwnd, IDC_SLIDER_SIZE), TBM_SETPOS, TRUE, value);
                    InvalidateRect(g_hOverlayWnd, NULL, TRUE);
                } else if (controlId == IDC_EDIT_THICKNESS && value >= 1 && value <= 10) {
                    currentPreset.thickness = value;
                    SendMessageW(GetDlgItem(hwnd, IDC_SLIDER_THICKNESS), TBM_SETPOS, TRUE, value);
                    InvalidateRect(g_hOverlayWnd, NULL, TRUE);
                }
            }
            
            switch (LOWORD(wParam)) {
                case IDC_BUTTON_SAVE_PRESET:
                    SaveAllSettings();
                    MessageBoxW(hwnd, L"¡Configuración guardada!", L"Guardado", MB_OK | MB_ICONINFORMATION);
                    break;
                case IDC_BUTTON_RESET_PRESET:
                    if (MessageBoxW(hwnd, L"¿Seguro que quieres resetear este preset a sus valores por defecto?", L"Confirmar", MB_YESNO | MB_ICONWARNING) == IDYES) {
                        GetDefaultPreset(&currentPreset);
                        UpdateAllControls(hwnd);
                        InvalidateRect(g_hOverlayWnd, NULL, TRUE);
                    }
                    break;
                case IDC_BUTTON_COLOR:
                    OpenColorDialog(hwnd);
                    InvalidateRect(g_hOverlayWnd, NULL, TRUE);
                    break;
                case IDC_CHECKBOX_DOT:
                    currentPreset.centerDot = !currentPreset.centerDot;
                    InvalidateRect(g_hOverlayWnd, NULL, TRUE);
                    break;
                case IDC_BUTTON_INFO:
                    MessageBoxW(hwnd, L"Creado por DonMiñoquito", L"Información", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            SaveAllSettings();
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
