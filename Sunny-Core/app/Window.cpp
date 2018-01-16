//
// Created by adunstudio on 2018-01-08.
//

#include "Window.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace sunny
{
    // 메시지를 처리하는 함수
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HINSTANCE hInstance;   // 윈도우 핸들 인스턴스 식별자
    HDC hDc;               // 윈도우 핸들 디바이스 컨텍스트
    HWND hWnd;             // 윈도우 핸들 (번호)

    std::map<void*, Window*> Window::s_handles;

    void Window::RegisterWindowClass(void* handle, Window* window)
    {
        s_handles[handle] = window;
    }

    Window* Window::GetWindowClass(void* handle)
    {
        if(handle == nullptr)
            return s_handles.begin()->second;

        return s_handles[handle];
    }

    Window::Window(const std::string title, const WindowProperties& properties)
            : m_title(title), m_properties(properties), m_handle(nullptr), m_closed(false)
    {
        if(!Init())
        {
            // TODO: Debug System
            return;
        }
    }

    Window::~Window()
    {

    }

    bool Window::Init()
    {
        hInstance = (HINSTANCE)&__ImageBase;

        WNDCLASS winClass      = {};                                      // 생성하려는 윈도우의 속성 값을 저장해 등록하는 구조체
        winClass.hInstance     = hInstance;                               //
        winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // 윈도우가 출력되는 형태
        winClass.lpfnWndProc   = (WNDPROC)WndProc;                        // 메시지 처리에 사용될 함수
        winClass.lpszClassName = "Sunny Win32 Window";                 // 윈도우 클래스의 이름 (윈도우를 만들 때 이 이름을 이용한다.)
        winClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);      // 기본 커서
        winClass.hIcon         = LoadIcon(nullptr, IDI_WINLOGO);      // 기본 아이콘

        /*
        winClass.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);          // 윈도우의 배경 색
        winClass.cbClsExtra     = 0;                                 // 클래스를 위한 여분의 메모리 크기
        winClass.cbWndExtra     = 0;                                 // 윈도우를 위한 여분의 메모리 크기
        */

        // 윈도우 클래스를 커널에 등록한다.
        if(!RegisterClassA(&winClass))
        {
            // TODO: Debug System
            return false;
        }

        RECT size   = { 0, 0, m_properties.width, m_properties.height };

        DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
        // WS_OVERLAPPEDWINDOW : 타이틀 바에 최소화, 최대화, 닫기 버튼이 나타나고 오른쪽 버튼을 눌렀을 때 시스템 메뉴가 나타난다.
        // WS_OVERLAPPED       : 기본적인 윈도우로 아이콘이 없고 최소화, 최대화, 닫기 버튼도 없다, 또한 시스템 메뉴가 나타나지 않는다.
        // ... 검색

        AdjustWindowRect(&size, style, false);

        hWnd = CreateWindowExA(
                WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,                             // 태스크바에 나타낸다.
                winClass.lpszClassName,                                         // 윈도우 클래스의 이름
                _T(m_title.c_str()),                                                // 만들어질 윈도우의 타이틀 바에 나타내는 문자열
                WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,        // 윈도우 스타일 조합
                GetSystemMetrics(SM_CXSCREEN) / 2 - m_properties.width / 2,     // 윈도우 생성 위치 x
                GetSystemMetrics(SM_CYSCREEN) / 2 - m_properties.height / 2,    // 윈도우 생성 위치 y
                // TODO: This requires some... attention
                size.right + (-size.left), size.bottom + (-size.top),           // 생성되는 윈도우의 폭과 높이
                NULL,
                NULL,
                hInstance,
                NULL
        );


        if(!hWnd)
        {
            // TODO: Debug System
            return false;
        }

        RegisterWindowClass(hWnd, this);

        ShowWindow(hWnd, SW_SHOW);
        SetFocus(hWnd);

        SetTitle(m_title);

        return true;
    }

    void Window::Update()
    {
        MSG message;

        while(PeekMessage(&message, nullptr, 0, 0, PM_REMOVE) > 0)
        {
            if (message.message == WM_QUIT)
            {
                m_closed = true;
                return;
            }

            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    void Window::Clear() const
    {
        // 화면을 지워주는 작업
    }

    bool Window::Closed() const
    {
        return m_closed;
    }

    void Window::SetTitle(const std::string title)
    {
        m_title = title;
        SetWindowText(hWnd, _T(m_title.c_str()));
    }

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        LRESULT result;

        Window* window = Window::GetWindowClass(hWnd);

        if(window == nullptr)
            return DefWindowProc(hWnd, message, wParam, lParam);

        switch(message)
        {
            case WM_SETFOCUS:
                FocusCallback(window, true);
                break;
            case WM_KILLFOCUS:
                FocusCallback(window, false);
                break;
            case WM_CLOSE:
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
                // KeyBoard
                break;
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
                // Mouse
                break;
            case WM_SIZE:
                ResizeCallback(window, LOWORD(lParam), HIWORD(lParam));
                break;
            default:
                result = DefWindowProc(hWnd, message, wParam, lParam);
        }

        return result;
    }

    void ResizeCallback(Window* window, int width, int height)
    {
        window->m_properties.width  = width;
        window->m_properties.height = height;
    }

    void FocusCallback(Window* window, bool focused)
    {
        // 포커싱 또는 포커싱을 잃었을때 작업하는 함수
    }
}