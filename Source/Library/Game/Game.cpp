#include "Game/Game.h"

namespace library
{
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	const wchar_t CLASS_NAME[] = L"Game Window Class";
	const D3D_FEATURE_LEVEL FEATURE_LEVELS[] = {
		D3D_FEATURE_LEVEL_9_1,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1
	};

	HINSTANCE hInstance;
	HWND hWindow;

	ComPtr<ID3D11Device>			pCurrentDevice;
	ComPtr<ID3D11DeviceContext>		pCurrentContext;
	ComPtr<IDXGISwapChain>			pCurrentSwapChain;
	ComPtr<ID3D11RenderTargetView>	pRenderTargetView;

	/*--------------------------------------------------------------------
	  Forward declarations
	--------------------------------------------------------------------*/

	HRESULT GetInterfaceForDeviceAndContext();
	HRESULT CreateSwapChain();
	HRESULT CreateRenderTarget();

	/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	  Function: WindowProc

	  Summary:  Defines the behavior of the window—its appearance, how
				it interacts with the user, and so forth

	  Args:     HWND hWnd
				  Handle to the window
				UINT uMsg
				  Message code
				WPARAM wParam
				  Additional data that pertains to the message
				LPARAM lParam
				  Additional data that pertains to the message

	  Returns:  LRESULT
				  Integer value that your program returns to Windows
	-----------------------------------------------------------------F-F*/
	LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
		switch (uMsg)
		{
		case WM_CLOSE:
		{
			HMENU hMenu;
			hMenu = GetMenu(hWnd);
			if (hMenu != nullptr)
			{
				DestroyMenu(hMenu);
			}
			DestroyWindow(hWnd);
			UnregisterClass(
				CLASS_NAME,
				hInstance
			);
			return 0;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow) {
		library::hInstance = hInstance;
		// Registers the window class
		WNDCLASS wc = {};

		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = CLASS_NAME;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

		RegisterClass(&wc);

		// Creates a window
		hWindow = CreateWindowEx(
			0,                              // Optional window styles.
			CLASS_NAME,                     // Window class
			L"Game Graphics Programming Lab 01: Direct3D 11 Basics",    // Window text
			WS_OVERLAPPEDWINDOW,            // Window style

			// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

			nullptr,		// Parent window    
			nullptr,		// Menu
			hInstance,		// Instance handle
			nullptr			// Additional application data
		);

		if (hWindow == nullptr)
		{
			DWORD dwError = GetLastError();
			return HRESULT_FROM_WIN32(dwError);
		}

		// Shows the window
		ShowWindow(hWindow, nCmdShow);

		// Returns a result code of HRESULT type
		return S_OK;
	}

	HRESULT InitDevice() {
		HRESULT hr;

		hr = GetInterfaceForDeviceAndContext();
		if (FAILED(hr)) return hr;

		hr = CreateSwapChain();
		if (FAILED(hr)) return hr;

		hr = CreateRenderTarget();
		if (FAILED(hr)) return hr;

		return S_OK;
	}

	HRESULT GetInterfaceForDeviceAndContext() {
		// This flag adds support for surfaces with a color-channel ordering different
		// from the API default. It is required for compatibility with Direct2D.
		UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG) || defined(_DEBUG)
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL featureLevel;

		HRESULT hr;
		hr = D3D11CreateDevice(
			nullptr,                    // Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
			nullptr,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			deviceFlags,                // Set debug and Direct2D compatibility flags.
			FEATURE_LEVELS,                     // List of feature levels this app can support.
			ARRAYSIZE(FEATURE_LEVELS),          // Size of the list above.
			D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
			pCurrentDevice.GetAddressOf(),                    // Returns the Direct3D device created.
			&featureLevel,            // Returns feature level of device created.
			pCurrentContext.GetAddressOf()                    // Returns the device immediate context.
		);
		if (FAILED(hr)) return hr;

		return S_OK;
	}

	HRESULT CreateSwapChain() {
		// -------------- Create the swap chain -------------------
		DXGI_SWAP_CHAIN_DESC desc = {};

		desc.Windowed = TRUE; // Sets the initial state of full-screen mode.
		desc.BufferCount = 2;
		desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SampleDesc.Count = 1;      //multisampling setting
		desc.SampleDesc.Quality = 0;    //vendor-specific flag
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		desc.OutputWindow = hWindow;

		// Create the DXGI device object to use in other factories, such as Direct2D.
		ComPtr<IDXGIDevice3> pDxgiDevice;
		pCurrentDevice.As(&pDxgiDevice);

		// Create swap chain.
		ComPtr<IDXGIAdapter> pAdapter;
		ComPtr<IDXGIFactory> pFactory;

		HRESULT hr;
		hr = pDxgiDevice->GetAdapter(&pAdapter);
		if (FAILED(hr)) return hr;

		hr = pAdapter->GetParent(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr)) return hr;

		hr = pFactory->CreateSwapChain(
			pCurrentDevice.Get(),
			&desc,
			&pCurrentSwapChain
		);
		if (FAILED(hr)) return hr;

		return S_OK;
	}

	HRESULT CreateRenderTarget() {
		HRESULT hr;

		// Create a render target view
		ComPtr<ID3D11Texture2D> pBackBuffer;
		D3D11_TEXTURE2D_DESC bbDesc;

		hr = pCurrentSwapChain->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			(void**)pBackBuffer.GetAddressOf());
		if (FAILED(hr)) return hr;

		hr = pCurrentDevice->CreateRenderTargetView(
			pBackBuffer.Get(),
			nullptr,
			pRenderTargetView.GetAddressOf()
		);
		if (FAILED(hr)) return hr;

		pBackBuffer->GetDesc(&bbDesc);

		// Create a depth-stencil buffer
		ComPtr<ID3D11Texture2D> pDepthStencil;
		ComPtr<ID3D11DepthStencilView> pDepthStencilView;

		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			bbDesc.Width,
			bbDesc.Height,
			1, // This depth stencil view has only one texture.
			1, // Use a single mipmap level.
			D3D11_BIND_DEPTH_STENCIL
		);

		hr = pCurrentDevice->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			pDepthStencil.GetAddressOf()
		);
		if (FAILED(hr)) return hr;

		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

		hr = pCurrentDevice->CreateDepthStencilView(
			pDepthStencil.Get(),
			&depthStencilViewDesc,
			pDepthStencilView.GetAddressOf()
		);
		if (FAILED(hr)) return hr;

		// Create a viewport
		D3D11_VIEWPORT viewport = {};
		viewport.Height = (float)bbDesc.Height;
		viewport.Width = (float)bbDesc.Width;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;

		pCurrentContext->RSSetViewports(
			1,
			&viewport
		);

		return S_OK;
	}

	void CleanupDevice() {
		// Explicitly release ComPtrs
		pCurrentDevice = nullptr;
		pCurrentContext = nullptr;
		pCurrentSwapChain = nullptr;
		pRenderTargetView = nullptr;
	}

	void Render() {
		// Clear the backbuffer
		const float ClearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; // RGBA
		pCurrentContext->ClearRenderTargetView(pRenderTargetView.Get(), ClearColor);

		// Present
		pCurrentSwapChain->Present(0, 0);
	}
}