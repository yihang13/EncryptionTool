#include <windows.h>
#include <string>

#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif

using namespace std;

// ==================== 加密算法核心 ====================

string xor_calc(string s, const string& key) {
    for (int i = 0; i < (int)s.size(); i++)
        s[i] ^= key[i % key.size()];
    return s;
}

string to_mi(string s) {
    string res;
    for (int i = 0; i < (int)s.size(); i++) {
        res += 'A' + ((unsigned char)s[i] / 16);
        res += 'A' + ((unsigned char)s[i] % 16);
    }
    return res;
}

string from_mi(string s) {
    string res;
    for (int i = 0; i < (int)s.size(); i += 2)
        res += (char)((s[i] - 'A') * 16 + (s[i + 1] - 'A'));
    return res;
}

// ==================== GUI ====================

#define IDC_KEY    101
#define IDC_INPUT  102
#define IDC_OUTPUT 103
#define IDC_ENCRYPT 104
#define IDC_DECRYPT 105
#define IDC_COPY   106

HWND hKey, hInput, hOutput, hBtnEnc, hBtnDec, hBtnCopy;
HFONT hFont;

void SetControlFont(HWND hwnd) {
    if (!hFont) {
        hFont = CreateFontW(
            18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI");
    }
    SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void DoEncrypt() {
    wchar_t keyBuf[256] = {0}, inBuf[4096] = {0};
    GetWindowTextW(hKey, keyBuf, 256);
    GetWindowTextW(hInput, inBuf, 4096);

    int keyLen = WideCharToMultiByte(CP_UTF8, 0, keyBuf, -1, nullptr, 0, nullptr, nullptr);
    int inLen = WideCharToMultiByte(CP_UTF8, 0, inBuf, -1, nullptr, 0, nullptr, nullptr);
    string key(keyLen - 1, 0), input(inLen - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, keyBuf, -1, &key[0], keyLen, nullptr, nullptr);
    WideCharToMultiByte(CP_UTF8, 0, inBuf, -1, &input[0], inLen, nullptr, nullptr);

    if (key.empty() || input.empty()) {
        MessageBoxW(nullptr, L"密钥和内容不能为空", L"提示", MB_ICONINFORMATION);
        return;
    }

    string result = to_mi(xor_calc(input, key));

    int resLen = MultiByteToWideChar(CP_UTF8, 0, result.c_str(), -1, nullptr, 0);
    wstring wres(resLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, result.c_str(), -1, &wres[0], resLen);
    SetWindowTextW(hOutput, wres.c_str());
}

void DoDecrypt() {
    wchar_t keyBuf[256] = {0}, inBuf[4096] = {0};
    GetWindowTextW(hKey, keyBuf, 256);
    GetWindowTextW(hInput, inBuf, 4096);

    int keyLen = WideCharToMultiByte(CP_UTF8, 0, keyBuf, -1, nullptr, 0, nullptr, nullptr);
    int inLen = WideCharToMultiByte(CP_UTF8, 0, inBuf, -1, nullptr, 0, nullptr, nullptr);
    string key(keyLen - 1, 0), input(inLen - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, keyBuf, -1, &key[0], keyLen, nullptr, nullptr);
    WideCharToMultiByte(CP_UTF8, 0, inBuf, -1, &input[0], inLen, nullptr, nullptr);

    if (key.empty() || input.empty()) {
        MessageBoxW(nullptr, L"密钥和内容不能为空", L"提示", MB_ICONINFORMATION);
        return;
    }

    string result = xor_calc(from_mi(input), key);

    int resLen = MultiByteToWideChar(CP_UTF8, 0, result.c_str(), -1, nullptr, 0);
    wstring wres(resLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, result.c_str(), -1, &wres[0], resLen);
    SetWindowTextW(hOutput, wres.c_str());
}

void DoCopy() {
    wchar_t buf[4096] = {0};
    GetWindowTextW(hOutput, buf, 4096);
    if (wcslen(buf) == 0) return;

    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        int len = wcslen(buf);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(wchar_t));
        if (hMem) {
            wchar_t* p = (wchar_t*)GlobalLock(hMem);
            wcscpy_s(p, len + 1, buf);
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        }
        CloseClipboard();
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        // 背景色
        HBRUSH bg = CreateSolidBrush(RGB(245, 247, 250));

        // 密钥行
        CreateWindowW(L"STATIC", L"密钥", WS_VISIBLE | WS_CHILD,
            30, 30, 60, 28, hwnd, nullptr, nullptr, nullptr);
        hKey = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            95, 28, 520, 30, hwnd, (HMENU)IDC_KEY, nullptr, nullptr);
        SendMessage(hKey, EM_SETCUEBANNER, TRUE, (LPARAM)L"输入加密/解密密钥");

        // 输入区
        CreateWindowW(L"STATIC", L"内容", WS_VISIBLE | WS_CHILD,
            30, 80, 60, 28, hwnd, nullptr, nullptr, nullptr);
        hInput = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            30, 108, 585, 120, hwnd, (HMENU)IDC_INPUT, nullptr, nullptr);

        // 按钮行
        hBtnEnc = CreateWindowW(L"BUTTON", L"加密", WS_VISIBLE | WS_CHILD | BS_FLAT,
            30, 248, 140, 40, hwnd, (HMENU)IDC_ENCRYPT, nullptr, nullptr);
        hBtnDec = CreateWindowW(L"BUTTON", L"解密", WS_VISIBLE | WS_CHILD | BS_FLAT,
            185, 248, 140, 40, hwnd, (HMENU)IDC_DECRYPT, nullptr, nullptr);

        // 输出区
        CreateWindowW(L"STATIC", L"结果", WS_VISIBLE | WS_CHILD,
            30, 310, 60, 28, hwnd, nullptr, nullptr, nullptr);
        hOutput = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            30, 338, 500, 120, hwnd, (HMENU)IDC_OUTPUT, nullptr, nullptr);

        hBtnCopy = CreateWindowW(L"BUTTON", L"复制结果", WS_VISIBLE | WS_CHILD | BS_FLAT,
            540, 338, 75, 40, hwnd, (HMENU)IDC_COPY, nullptr, nullptr);

        // 统一字体
        SetControlFont(hKey);
        SetControlFont(hInput);
        SetControlFont(hOutput);
        SetControlFont(hBtnEnc);
        SetControlFont(hBtnDec);
        SetControlFont(hBtnCopy);
        // STATIC 也设字体
        HWND child = GetWindow(hwnd, GW_CHILD);
        while (child) {
            wchar_t cls[32];
            GetClassNameW(child, cls, 32);
            if (wcscmp(cls, L"Static") == 0)
                SendMessage(child, WM_SETFONT, (WPARAM)hFont, TRUE);
            child = GetWindow(child, GW_HWNDNEXT);
        }

        return 0;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wp;
        SetBkColor(hdc, RGB(245, 247, 250));
        SetTextColor(hdc, RGB(60, 60, 60));
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wp;
        SetBkColor(hdc, RGB(255, 255, 255));
        return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
    case WM_COMMAND:
        if (LOWORD(wp) == IDC_ENCRYPT)
            DoEncrypt();
        else if (LOWORD(wp) == IDC_DECRYPT)
            DoDecrypt();
        else if (LOWORD(wp) == IDC_COPY)
            DoCopy();
        break;
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wp;
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH bg = CreateSolidBrush(RGB(245, 247, 250));
        FillRect(hdc, &rc, bg);
        DeleteObject(bg);
        return 1;
    }
    case WM_DESTROY:
        DeleteObject(hFont);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(245, 247, 250));
    wc.lpszClassName = L"EncryptTool";
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(L"EncryptTool", L"加密解密工具",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 660, 500,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
