#include "Renderer/Renderer.h"

namespace library {
	const D3D_FEATURE_LEVEL FEATURE_LEVELS[] = {
	D3D_FEATURE_LEVEL_9_1,
	D3D_FEATURE_LEVEL_9_2,
	D3D_FEATURE_LEVEL_9_3,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_11_1
	};

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Renderer

	  Summary:  Constructor

	  Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
				  m_immediateContext, m_immediateContext1, m_swapChain,
				  m_swapChain1, m_renderTargetView].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	Renderer::Renderer()
	{
		m_driverType = D3D_DRIVER_TYPE_HARDWARE;
		m_featureLevel = D3D_FEATURE_LEVEL_11_1;

		m_d3dDevice = nullptr;
		m_d3dDevice1 = nullptr;
		m_immediateContext = nullptr;
		m_immediateContext1 = nullptr;
		m_swapChain = nullptr;
		m_swapChain1 = nullptr;
		m_renderTargetView = nullptr;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Initialize

	  Summary:  Creates Direct3D device and swap chain

	  Args:     HWND hWnd
				  Handle to the window

	  Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
				  m_d3dDevice1, m_immediateContext1, m_swapChain1,
				  m_swapChain, m_renderTargetView].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::Initialize(_In_ HWND hWnd)
	{
		HRESULT hr;

		// ---------------------------------------------------------------------------------------------
		// GetInterfaceForDeviceAndContext 
		// ---------------------------------------------------------------------------------------------

		// This flag adds support for surfaces with a color-channel ordering different
		// from the API default. It is required for compatibility with Direct2D.
		UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG) || defined(_DEBUG)
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		hr = D3D11CreateDevice(
			nullptr,								// Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE,				// Create a device using the hardware graphics driver.
			nullptr,								// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			deviceFlags,							// Set debug and Direct2D compatibility flags.
			FEATURE_LEVELS,							// List of feature levels this app can support.
			ARRAYSIZE(FEATURE_LEVELS),				// Size of the list above.
			D3D11_SDK_VERSION,						// Always set this to D3D11_SDK_VERSION for Windows Store apps.
			m_d3dDevice.GetAddressOf(),		// Returns the Direct3D device created.
			&m_featureLevel,					// Returns feature level of device created.
			m_immediateContext.GetAddressOf() // Returns the device immediate context.
		);
		if (FAILED(hr)) return hr;

		m_d3dDevice.As(&m_d3dDevice1);
		m_immediateContext.As(&m_immediateContext1);

		// ---------------------------------------------------------------------------------------------
		// CreateSwapChain 
		// ---------------------------------------------------------------------------------------------

		DXGI_SWAP_CHAIN_DESC desc = {};

		desc.Windowed = TRUE; // Sets the initial state of full-screen mode.
		desc.BufferCount = 2;
		desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SampleDesc.Count = 1;      //multisampling setting
		desc.SampleDesc.Quality = 0;    //vendor-specific flag
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		desc.OutputWindow = hWnd;

		// Create the DXGI device object to use in other factories, such as Direct2D.
		ComPtr<IDXGIDevice3> pDxgiDevice;
		m_d3dDevice.As(&pDxgiDevice);

		// Create swap chain.
		ComPtr<IDXGIAdapter> pAdapter;
		ComPtr<IDXGIFactory> pFactory;

		hr = pDxgiDevice->GetAdapter(&pAdapter);
		if (FAILED(hr)) return hr;

		hr = pAdapter->GetParent(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr)) return hr;

		hr = pFactory->CreateSwapChain(
			m_d3dDevice.Get(),
			&desc,
			m_swapChain.GetAddressOf()
		);
		if (FAILED(hr)) return hr;

		m_swapChain.As(&m_swapChain1);

		// ---------------------------------------------------------------------------------------------
		// CreateRenderTarget 
		// ---------------------------------------------------------------------------------------------

		ComPtr<ID3D11Texture2D> pBackBuffer;
		D3D11_TEXTURE2D_DESC bbDesc;

		hr = m_swapChain->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			(void**)pBackBuffer.GetAddressOf());
		if (FAILED(hr)) return hr;

		hr = m_d3dDevice->CreateRenderTargetView(
			pBackBuffer.Get(),
			nullptr,
			m_renderTargetView.GetAddressOf()
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

		hr = m_d3dDevice->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			pDepthStencil.GetAddressOf()
		);
		if (FAILED(hr)) return hr;

		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

		hr = m_d3dDevice->CreateDepthStencilView(
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

		m_immediateContext->RSSetViewports(
			1,
			&viewport
		);

		return S_OK;
	}


	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Render

	  Summary:  Render the frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::Render()
	{
		// Clear the backbuffer
		const float ClearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; // RGBA
		m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), ClearColor);

		// Present
		m_swapChain->Present(0, 0);
	}
}







