#include <windows.h>
#include <vector>
#include <cstdint>

// Структура для цвета
struct Color {
    uint8_t r, g, b;

    Color(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}

    uint32_t toInt() const {
        return (r << 16) | (g << 8) | b;
    }
};

// Класс для framebuffer
class Framebuffer {
public:
    int width, height;
    std::vector<uint32_t> buffer;

    Framebuffer(int w, int h) : width(w), height(h), buffer(w* h, 0xFFFFFF) {}

    void resize(int w, int h) {
        width = w;
        height = h;
        buffer.resize(w * h, 0xFFFFFF);
    }

    void clear(const Color& color) {
        std::fill(buffer.begin(), buffer.end(), color.toInt());
    }

    void setPixel(int x, int y, const Color& color) {
        if (x >= 0 && y >= 0 && x < width && y < height) {
            buffer[y * width + x] = color.toInt();
        }
    }

    void drawRectangle(int x, int y, int w, int h, const Color& color) {
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                setPixel(x + j, y + i, color);
            }
        }
    }

    void drawQuarterCircle(int centerX, int centerY, int radius, const Color& color, int quadrant) {
        for (int y = 0; y <= radius; ++y) {
            for (int x = 0; x <= radius; ++x) {
                if (x * x + y * y <= radius * radius) {
                    switch (quadrant) {
                    case 1: // Верхний левый
                        setPixel(centerX - x, centerY - y, color);
                        break;
                    case 2: // Верхний правый
                        setPixel(centerX + x, centerY - y, color);
                        break;
                    case 3: // Нижний левый
                        setPixel(centerX - x, centerY + y, color);
                        break;
                    case 4: // Нижний правый
                        setPixel(centerX + x, centerY + y, color);
                        break;
                    }
                }
            }
        }
    }

    void drawButton(int x, int y, int w, int h, int radius, const Color& color) {
        drawRectangle(x + radius, y, w - 2 * radius, h, color);
        drawRectangle(x, y + radius, w, h - 2 * radius, color);
        drawQuarterCircle(x + radius, y + radius, radius, color, 1);
        drawQuarterCircle(x + w - radius - 1, y + radius, radius, color, 2);
        drawQuarterCircle(x + radius, y + h - radius - 1, radius, color, 3);
        drawQuarterCircle(x + w - radius - 1, y + h - radius - 1, radius, color, 4);
    }

    void drawBorder(int x, int y, int w, int h, int radius, int thickness, const Color& color) {
        for (int i = 0; i < thickness; ++i) {
            // Horizontal borders
            drawRectangle(x + radius, y + i, w - 2 * radius, 1, color); // Top border
            drawRectangle(x + radius, y + h - i - 1, w - 2 * radius, 1, color); // Bottom border

            // Vertical borders
            drawRectangle(x + i, y + radius, 1, h - 2 * radius, color); // Left border
            drawRectangle(x + w - i - 1, y + radius, 1, h - 2 * radius, color); // Right border

            // Corner borders
            drawQuarterCircle(x + radius, y + radius, radius - i, color, 1); // Top-left corner
            drawQuarterCircle(x + w - radius - 1, y + radius, radius - i, color, 2); // Top-right corner
            drawQuarterCircle(x + radius, y + h - radius - 1, radius - i, color, 3); // Bottom-left corner
            drawQuarterCircle(x + w - radius - 1, y + h - radius - 1, radius - i, color, 4); // Bottom-right corner
        }
    }
};

Framebuffer fb(800, 600);
Color orange(255, 165, 0);
Color green(11, 132, 0);
Color lightGreen(17, 207, 0);
Color lightOrange(255, 200, 100);
Color white(255, 255, 255);
Color black(0, 0, 0);

BITMAPINFO bitmapInfo;
bool needsRedraw = true;
bool isHovered = false;

struct Button {
    int x, y, width, height, radius;
    const wchar_t* text;
    int borderThickness;
    Color borderColor;

    Button(int x, int y, int w, int h, int r, const wchar_t* t)
        : x(x), y(y), width(w), height(h), radius(r), text(t), borderThickness(0), borderColor(black) {
    }

    bool isMouseOver(int mouseX, int mouseY) const {
        return mouseX >= x && mouseX <= x + width &&
            mouseY >= y && mouseY <= y + height;
    }

    void setBorder(int thickness, const Color& color) {
        borderThickness = thickness;
        borderColor = color;
    }
};

Button button(100, 100, 430, 180, 7, L"Simple Button!");

void updateFramebuffer() {
    fb.clear(white);
    fb.drawButton(button.x, button.y, button.width, button.height, button.radius, isHovered ? lightGreen : green);
    if (button.borderThickness > 0) {
        fb.drawBorder(button.x, button.y, button.width, button.height, button.radius, button.borderThickness, button.borderColor);
    }
    needsRedraw = true;
}

void setupBitmapInfo(int width, int height) {
    ZeroMemory(&bitmapInfo, sizeof(BITMAPINFO));
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = -height;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SIZE: {
        int newWidth = LOWORD(lParam);
        int newHeight = HIWORD(lParam);

        fb.resize(newWidth, newHeight);
        setupBitmapInfo(newWidth, newHeight);
        updateFramebuffer();

        return 0;
    }
    case WM_MOUSEMOVE: {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        bool hoverState = button.isMouseOver(mouseX, mouseY);
        if (hoverState != isHovered) {
            isHovered = hoverState;
            updateFramebuffer();

            SetCursor(isHovered ? LoadCursor(nullptr, IDC_HAND) : LoadCursor(nullptr, IDC_ARROW));

            InvalidateRect(hwnd, nullptr, FALSE);
        }

        return 0;
    }
    case WM_PAINT: {
        if (needsRedraw) {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            StretchDIBits(
                hdc,
                0, 0, fb.width, fb.height,
                0, 0, fb.width, fb.height,
                fb.buffer.data(),
                &bitmapInfo,
                DIB_RGB_COLORS,
                SRCCOPY
            );

            HFONT hFont = CreateFont(19, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_SWISS, L"Arial");
            HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            RECT buttonRect = { button.x, button.y, button.x + button.width, button.y + button.height };
            DrawText(hdc, button.text, -1, &buttonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(hdc, oldFont);
            DeleteObject(hFont);

            EndPaint(hwnd, &ps);
            needsRedraw = false;
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"BufferedWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Buffered Framebuffer Example",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        fb.width, fb.height,
        nullptr, nullptr,
        hInstance, nullptr
    );

    if (hwnd == nullptr) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    setupBitmapInfo(fb.width, fb.height);
    updateFramebuffer();

    button.setBorder(1, black);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
