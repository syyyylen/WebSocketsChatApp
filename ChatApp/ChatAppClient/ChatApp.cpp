#include <iostream>
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>

inline void Log(const std::string& content)
{
    std::cout << content << '\n';
}

#define LOG(content) Log(content)

bool isRunning;
HWND hwnd;
ID3D11Device* device;
ID3D11DeviceContext* deviceContext;
IDXGISwapChain* dxgiSwapChain;
ID3D11RenderTargetView* backbufferRTV;

void ReloadBackBufferD3D11()
{
    ID3D11Texture2D* buffer = NULL;
    HRESULT hr;

    hr = dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buffer);
    if(FAILED(hr))
    {
        LOG("Failed Backbuffer creation");
        std::string errorMsg = std::system_category().message(hr);
        LOG(errorMsg);
    }

    hr = device->CreateRenderTargetView(buffer, nullptr, &backbufferRTV);
    if(FAILED(hr))
    {
        LOG("Failed Render Target creation");
        std::string errorMsg = std::system_category().message(hr);
        LOG(errorMsg);
    }
}

void ResizeBackBufferD3D11(unsigned width, unsigned height)
{
    backbufferRTV->Release();
    dxgiSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    ReloadBackBufferD3D11();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch(msg)
    {
    case WM_SIZE:
        {
            if(isRunning)
            {
                int width = LOWORD(lparam);
                int height = HIWORD(lparam);
                ResizeBackBufferD3D11(width, height);
            }
            break;
        }

    case WM_SETFOCUS:
        {
            break;
        }

    case WM_KILLFOCUS:
        {
            break;
        }

    case WM_DESTROY:
        {
            isRunning = false;
            ::PostQuitMessage(0);
            break;
        }

    default:
        return ::DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return NULL;
}

int main()
{
    std::cout << "Hello Client !" << std::endl;

    WNDCLASSEXW wc;
    wc.cbClsExtra = NULL;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = NULL;
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = NULL;
    wc.lpszClassName = L"MinecraftCloneWindowClass";
    wc.lpszMenuName = L"";
    wc.style = NULL;
    wc.lpfnWndProc = &WndProc;

    if(!::RegisterClassExW(&wc))
        return 0;

    int width = 1024;
    int height = 720;
    hwnd = ::CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, L"MinecraftCloneWindowClass", L"MinecraftClone", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, NULL, nullptr);

    ::ShowWindow(hwnd, SW_SHOW);
    ::UpdateWindow(hwnd);

    D3D_FEATURE_LEVEL featureLevels[]=
    {
        D3D_FEATURE_LEVEL_11_0
    };

    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
    desc.BufferCount = 2;
    desc.BufferDesc.Width = width;
    desc.BufferDesc.Height = height;
    desc.BufferDesc.RefreshRate.Numerator = 60;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_UNORDERED_ACCESS | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SampleDesc.Count = 1;      
    desc.SampleDesc.Quality = 0;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    desc.OutputWindow = hwnd;
    desc.Windowed = TRUE;

    D3D_FEATURE_LEVEL retFeatureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                   
        D3D_DRIVER_TYPE_HARDWARE,
        0,                          
        deviceFlags,               
        featureLevels,                     
        ARRAYSIZE(featureLevels),          
        D3D11_SDK_VERSION,          
        &desc,
        &dxgiSwapChain,
        &device,                  // Returns the Direct3D device created.
        &retFeatureLevel,          // Returns feature level of device created.
        &deviceContext            // Returns the device immediate context.
    );

    if(FAILED(hr))
    {
        LOG("Failed Device & SwapChain creation");
        std::string errorMsg = std::system_category().message(hr);
        LOG(errorMsg);
        throw std::exception("Failed SwapChain creation");
    }

    ReloadBackBufferD3D11();

    isRunning = true;

    while(isRunning)
    {
        MSG msg;
        while(::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) > 0)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }

        D3D11_VIEWPORT vp = {};
        vp.Width = width;
        vp.Height = height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        deviceContext->RSSetViewports(1, &vp);

        FLOAT clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        deviceContext->ClearRenderTargetView(backbufferRTV, clearColor);

        deviceContext->OMSetRenderTargets(1, &backbufferRTV, nullptr);
        dxgiSwapChain->Present(true, NULL);
    }
    
    DestroyWindow(hwnd);

    std::cin.get();
}
