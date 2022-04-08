#include "Renderer/Renderer.h"

namespace library {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Renderer

	  Summary:  Constructor

	  Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
				  m_immediateContext, m_immediateContext1, m_swapChain,
				  m_swapChain1, m_renderTargetView, m_vertexShader,
				  m_pixelShader, m_vertexLayout, m_vertexBuffer].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	Renderer::Renderer() :
		m_driverType(D3D_DRIVER_TYPE_HARDWARE),
		m_featureLevel(D3D_FEATURE_LEVEL_11_1),
		m_d3dDevice(),
		m_d3dDevice1(),
		m_immediateContext(),
		m_immediateContext1(),
		m_swapChain(),
		m_swapChain1(),
		m_renderTargetView(),
		m_depthStencil(),
		m_depthStencilView(),
		m_view(),
		m_projection(),
		m_renderables(),
		m_vertexShaders(),
		m_pixelShaders()
	{}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Initialize

	  Summary:  Creates Direct3D device and swap chain

	  Args:     HWND hWnd
				  Handle to the window

	  Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
				  m_d3dDevice1, m_immediateContext1, m_swapChain1,
				  m_swapChain, m_renderTargetView, m_vertexShader,
				  m_vertexLayout, m_pixelShader, m_vertexBuffer].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::Initialize(_In_ HWND hWnd)
	{
		HRESULT hr;

#pragma region GetInterfaceForDeviceAndContext
		// This flag adds support for surfaces with a color-channel ordering different
		// from the API default. It is required for compatibility with Direct2D.
		UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		const D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

#if defined(DEBUG) || defined(_DEBUG)
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		hr = D3D11CreateDevice(
			nullptr,								// Specify nullptr to use the default adapter.
			m_driverType,							// Create a device using the hardware graphics driver.
			nullptr,								// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			deviceFlags,							// Set debug and Direct2D compatibility flags.
			featureLevels,							// List of feature levels this app can support.
			ARRAYSIZE(featureLevels),				// Size of the list above.
			D3D11_SDK_VERSION,						// Always set this to D3D11_SDK_VERSION for Windows Store apps.
			&m_d3dDevice,							// Returns the Direct3D device created.
			&m_featureLevel,						// Returns feature level of device created.
			&m_immediateContext						// Returns the device immediate context.
		);
		if (FAILED(hr)) return hr;

		m_d3dDevice.As(&m_d3dDevice1);
		m_immediateContext.As(&m_immediateContext1);
#pragma endregion

#pragma region CreateSwapChain
		DXGI_SWAP_CHAIN_DESC desc = {
			.BufferDesc = {
				.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			},
			.SampleDesc = {
				.Count = 1,
				.Quality = 0
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.OutputWindow = hWnd,
			.Windowed = TRUE,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
		};

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
			&m_swapChain
		);
		if (FAILED(hr)) return hr;

		m_swapChain.As(&m_swapChain1);
#pragma endregion

#pragma region CreateBuffersAndViews
		ComPtr<ID3D11Texture2D> pBackBuffer;
		D3D11_TEXTURE2D_DESC bbDesc;

		hr = m_swapChain->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			&pBackBuffer);
		if (FAILED(hr)) return hr;

		hr = m_d3dDevice->CreateRenderTargetView(
			pBackBuffer.Get(),
			nullptr,
			&m_renderTargetView
		);
		if (FAILED(hr)) return hr;

		m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

		pBackBuffer->GetDesc(&bbDesc);

		// Create depth stencil and the depth stencil view

		// Depth Stencil
		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			bbDesc.Width,
			bbDesc.Height,
			1, // Number of textures.
			1, // Single mipmap level.
			D3D11_BIND_DEPTH_STENCIL
		);

		hr = m_d3dDevice->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			&m_depthStencil
		);
		if (FAILED(hr)) return hr;

		// Depth Stencil View
		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(
			D3D11_DSV_DIMENSION_TEXTURE2D,
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			0
		);

		hr = m_d3dDevice->CreateDepthStencilView(
			m_depthStencil.Get(),
			&depthStencilViewDesc,
			&m_depthStencilView
		);
		if (FAILED(hr)) return hr;
#pragma endregion

#pragma region CreateAndSetViewport
		// Create a viewport
		D3D11_VIEWPORT viewport = {
			.Width = (float)bbDesc.Width,
			.Height = (float)bbDesc.Height,
			.MinDepth = 0,
			.MaxDepth = 1
		};

		m_immediateContext->RSSetViewports(
			1,
			&viewport
		);
#pragma endregion

#pragma region CreateViewAndProjectionMatrices
		// Initialize view matrix and the projection matrix
		FXMVECTOR vEye(0.0f, 1.0f, -5.0f, 0.0f);
		FXMVECTOR vAt(0.0f, 1.0f, 0.0f, 0.0f);
		FXMVECTOR vUp(0.0f, 1.0f, 0.0f, 0.0f);

		m_view = XMMatrixLookAtLH(vEye, vAt, vUp);

		float fovAngleY = 3.14159265358979323846f;
		float nearZ = 0.01f;
		float farZ = 100.0f;


		m_projection = XMMatrixPerspectiveFovLH(fovAngleY, bbDesc.Width / bbDesc.Height, nearZ, farZ);
#pragma endregion

#pragma region InitializeRenderablesAndShaders
		for (auto renderable : m_renderables)
		{
			renderable.second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
		}

		for (auto vs : m_vertexShaders)
		{
			vs.second->Initialize(m_d3dDevice.Get());
		}

		for (auto ps : m_pixelShaders)
		{
			ps.second->Initialize(m_d3dDevice.Get());
		}
#pragma endregion

		// Initialize the shaders, then the renderables
		// - Shaders and renderables are stored in Hash maps
		// - Strings are used as the key, and the shader/renderable objects are the value
		// - When iterating, use iterators

		m_immediateContext->IASetInputLayout(m_vertexLayout.Get());

#pragma region CreateSetVertexBuffer
		SimpleVertex vertices[] = {
			{ XMFLOAT3(0.0f, 0.5f, 0.5f) },
			{ XMFLOAT3(0.5f, -0.5f, 0.5f) },
			{ XMFLOAT3(-0.5f, -0.5f, 0.5f) },
		};

		D3D11_BUFFER_DESC bufferDesc = {
			.ByteWidth = sizeof(vertices),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0
		};

		D3D11_SUBRESOURCE_DATA initData = {
			.pSysMem = vertices,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		hr = m_d3dDevice->CreateBuffer(&bufferDesc, &initData, &m_vertexBuffer);
		if (FAILED(hr)) return hr;

		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;

		m_immediateContext->IASetVertexBuffers(
			0,                // the first input slot for binding
			1,                // the number of buffers in the array
			m_vertexBuffer.GetAddressOf(), // the array of vertex buffers
			&stride,          // array of stride values, one for each buffer
			&offset);
#pragma endregion

		// Set primitive topology
		m_immediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddRenderable

	  Summary:  Add a renderable object and initialize the object

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable object
				const std::shared_ptr<Renderable>& renderable
				  Unique pointer to the renderable object

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
	{
		return E_NOTIMPL;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddVertexShader

	  Summary:  Add the vertex shader into the renderer

	  Args:     PCWSTR pszVertexShaderName
				  Key of the vertex shader
				const std::shared_ptr<VertexShader>&
				  Vertex shader to add

	  Modifies: [m_vertexShaders].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
	{
		return E_NOTIMPL;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddPixelShader

	  Summary:  Add the pixel shader into the renderer

	  Args:     PCWSTR pszPixelShaderName
				  Key of the pixel shader
				const std::shared_ptr<PixelShader>&
				  Pixel shader to add

	  Modifies: [m_pixelShaders].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
	{
		return E_NOTIMPL;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Update

	  Summary:  Update the renderables each frame

	  Args:     FLOAT deltaTime
				  Time difference of a frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::Update(_In_ FLOAT deltaTime)
	{
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

		// Set shaders
		m_immediateContext->VSSetShader(m_vertexShader.Get(), nullptr, 0u);
		m_immediateContext->PSSetShader(m_pixelShader.Get(), nullptr, 0u);

		// Draw
		m_immediateContext->Draw(3, 0);

		// Present
		m_swapChain->Present(0, 0);

		// Set Render Target View again (Present call for DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL unbinds backbuffer 0)
		m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetVertexShaderOfRenderable

	  Summary:  Sets the vertex shader for a renderable

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable
				PCWSTR pszVertexShaderName
				  Key of the vertex shader

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
	{
		return E_NOTIMPL;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetPixelShaderOfRenderable

	  Summary:  Sets the pixel shader for a renderable

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable
				PCWSTR pszPixelShaderName
				  Key of the pixel shader

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
	{
		return E_NOTIMPL;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::GetDriverType

	  Summary:  Returns the Direct3D driver type

	  Returns:  D3D_DRIVER_TYPE
				  The Direct3D driver type used
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	D3D_DRIVER_TYPE Renderer::GetDriverType() const
	{
		return D3D_DRIVER_TYPE();
	}
}







