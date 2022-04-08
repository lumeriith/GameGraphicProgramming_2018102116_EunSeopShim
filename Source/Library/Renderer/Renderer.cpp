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

#pragma region CreateBackDepthStencilBuffersAndViews
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

		pBackBuffer->GetDesc(&bbDesc);

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

		m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
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
		for (auto& vs : m_vertexShaders)
		{
			vs.second->Initialize(m_d3dDevice.Get());
		}

		for (auto& ps : m_pixelShaders)
		{
			ps.second->Initialize(m_d3dDevice.Get());
		}

		for (auto& renderable : m_renderables)
		{
			renderable.second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
		}
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
		if (m_renderables.count(pszRenderableName) > 0) return E_FAIL;

		std::shared_ptr<Renderable> newItem(renderable);
		m_renderables.insert({ pszRenderableName, newItem });
		// TODO Is this even right??

		return S_OK;
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
		if (m_vertexShaders.count(pszVertexShaderName) > 0) return E_FAIL;

		std::shared_ptr<VertexShader> newItem(vertexShader);
		m_vertexShaders.insert({ pszVertexShaderName, newItem });
		// TODO Is this even right??

		return S_OK;
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
		if (m_pixelShaders.count(pszPixelShaderName) > 0) return E_FAIL;

		std::shared_ptr<PixelShader> newItem(pixelShader);
		m_pixelShaders.insert({ pszPixelShaderName, newItem });
		// TODO Is this even right??

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Update

	  Summary:  Update the renderables each frame

	  Args:     FLOAT deltaTime
				  Time difference of a frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::Update(_In_ FLOAT deltaTime)
	{
		for (auto& renderable : m_renderables)
		{
			renderable.second->Update(deltaTime);
		}
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

		// Clear the depth buffer to 1.0 (maximum depth)
		m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// For each renderables
		for (auto& pair : m_renderables)
		{
			auto& renderable = pair.second;

			// Set the vertex buffer
			UINT stride = sizeof(SimpleVertex);
			UINT offset = 0;

			m_immediateContext->IASetVertexBuffers(
				0,												// the first input slot for binding
				1,												// the number of buffers in the array
				renderable->GetVertexBuffer().GetAddressOf(),	// the array of vertex buffers
				&stride,										// array of stride values, one for each buffer
				&offset
			);

			// Set the index buffer
			m_immediateContext->IASetIndexBuffer(renderable->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

			// Set the input layout
			m_immediateContext->IASetInputLayout(renderable->GetVertexLayout().Get());

			// Update constant buffer
			ConstantBuffer cb = {
				.World = renderable->GetWorldMatrix(),
				.View = m_view,
				.Projection = m_projection,
			};
			cb.World = XMMatrixTranspose(cb.World);
			cb.View = XMMatrixTranspose(cb.View);
			cb.Projection = XMMatrixTranspose(cb.Projection);

			m_immediateContext->UpdateSubresource(
				renderable->GetConstantBuffer().Get(),
				0u,
				nullptr,
				&cb,
				0u,
				0u
			);

			// Set constant buffer
			m_immediateContext->VSSetConstantBuffers(0, 1, renderable->GetConstantBuffer().GetAddressOf());

			// Render the triangles
			m_immediateContext->DrawIndexed(renderable->GetNumIndices(), 0, 0);
		}

		// Present
		m_swapChain->Present(0, 0);

		// Set Render Target View again (Present call for DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL unbinds backbuffer 0)
		m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
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







