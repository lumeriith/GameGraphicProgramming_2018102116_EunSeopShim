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
	Renderer::Renderer()
		: m_driverType(D3D_DRIVER_TYPE_NULL)
		, m_featureLevel(D3D_FEATURE_LEVEL_11_0)
		, m_d3dDevice()
		, m_d3dDevice1()
		, m_immediateContext()
		, m_immediateContext1()
		, m_swapChain()
		, m_swapChain1()
		, m_renderTargetView()
		, m_depthStencil()
		, m_depthStencilView()
		, m_cbChangeOnResize()
		, m_pszMainSceneName(nullptr)
		, m_padding{ '\0' }
		, m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
		, m_projection()
		, m_scenes()
		, m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))
	{
	}

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
		HRESULT hr = S_OK;

		RECT rc;
		GetClientRect(hWnd, &rc);
		UINT uWidth = static_cast<UINT>(rc.right - rc.left);
		UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

		UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
		uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			m_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

			if (hr == E_INVALIDARG)
			{
				// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
				hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
					D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
			}

			if (SUCCEEDED(hr))
			{
				break;
			}
		}
		if (FAILED(hr))
		{
			return hr;
		}

		// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
		ComPtr<IDXGIFactory1> dxgiFactory;
		{
			ComPtr<IDXGIDevice> dxgiDevice;
			hr = m_d3dDevice.As(&dxgiDevice);
			if (SUCCEEDED(hr))
			{
				ComPtr<IDXGIAdapter> adapter;
				hr = dxgiDevice->GetAdapter(&adapter);
				if (SUCCEEDED(hr))
				{
					hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
				}
			}
		}
		if (FAILED(hr))
		{
			return hr;
		}

		// Create swap chain
		ComPtr<IDXGIFactory2> dxgiFactory2;
		hr = dxgiFactory.As(&dxgiFactory2);
		if (SUCCEEDED(hr))
		{
			// DirectX 11.1 or later
			hr = m_d3dDevice.As(&m_d3dDevice1);
			if (SUCCEEDED(hr))
			{
				m_immediateContext.As(&m_immediateContext1);
			}

			DXGI_SWAP_CHAIN_DESC1 sd =
			{
				.Width = uWidth,
				.Height = uHeight,
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.SampleDesc = {.Count = 1u, .Quality = 0u },
				.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
				.BufferCount = 1u
			};

			hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
			if (SUCCEEDED(hr))
			{
				hr = m_swapChain1.As(&m_swapChain);
			}
		}
		else
		{
			// DirectX 11.0 systems
			DXGI_SWAP_CHAIN_DESC sd =
			{
				.BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
				.SampleDesc = {.Count = 1, .Quality = 0 },
				.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
				.BufferCount = 1u,
				.OutputWindow = hWnd,
				.Windowed = TRUE
			};

			hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
		}

		// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
		dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		if (FAILED(hr))
		{
			return hr;
		}

		// Create a render target view
		ComPtr<ID3D11Texture2D> pBackBuffer;
		hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		if (FAILED(hr))
		{
			return hr;
		}

		hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}

		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth =
		{
			.Width = uWidth,
			.Height = uHeight,
			.MipLevels = 1u,
			.ArraySize = 1u,
			.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
			.SampleDesc = {.Count = 1u, .Quality = 0u },
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_DEPTH_STENCIL,
			.CPUAccessFlags = 0u,
			.MiscFlags = 0u
		};
		hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
		{
			.Format = descDepth.Format,
			.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
			.Texture2D = {.MipSlice = 0 }
		};
		hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}

		m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

		// Setup the viewport
		D3D11_VIEWPORT vp =
		{
			.TopLeftX = 0.0f,
			.TopLeftY = 0.0f,
			.Width = static_cast<FLOAT>(uWidth),
			.Height = static_cast<FLOAT>(uHeight),
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f,
		};
		m_immediateContext->RSSetViewports(1, &vp);

		// Set primitive topology
		m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Create the constant buffers
		D3D11_BUFFER_DESC bd =
		{
			.ByteWidth = sizeof(CBChangeOnResize),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = 0
		};
		hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}

		// Initialize the projection matrix
		m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

		CBChangeOnResize cbChangesOnResize =
		{
			.Projection = XMMatrixTranspose(m_projection)
		};
		m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

		bd.ByteWidth = sizeof(CBLights);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0u;

		hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}

		m_camera.Initialize(m_d3dDevice.Get());

		if (!m_scenes.contains(m_pszMainSceneName))
		{
			return E_FAIL;
		}

		hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
		if (FAILED(hr))
		{
			return hr;
		}

		hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
		if (FAILED(hr))
		{
			return hr;
		}

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddRenderable

	  Summary:  Add a renderable object

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable object
				const std::shared_ptr<Renderable>& renderable
				  Shared pointer to the renderable object

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
	{
		if (m_renderables.contains(pszRenderableName)) return E_FAIL;
		m_renderables.insert({ pszRenderableName, renderable });

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddModel

	  Summary:  Add a model object

	  Args:     PCWSTR pszModelName
				  Key of the model object
				const std::shared_ptr<Model>& pModel
				  Shared pointer to the model object

	  Modifies: [m_models].

	  Returns:  HRESULT
				  Status code.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddModel(_In_ PCWSTR pszModelName, _In_ const std::shared_ptr<Model>& pModel)
	{
		if (m_models.contains(pszModelName)) return E_FAIL;
		m_models.insert({ pszModelName, pModel });

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddPointLight

	  Summary:  Add a point light

	  Args:     size_t index
				  Index of the point light
				const std::shared_ptr<PointLight>& pointLight
				  Shared pointer to the point light object

	  Modifies: [m_aPointLights].

	  Returns:  HRESULT
				  Status code.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
	{
		if (index >= NUM_LIGHTS || !pPointLight)
		{
			return E_FAIL;
		}

		m_aPointLights[index] = pPointLight;
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
		if (m_vertexShaders.contains(pszVertexShaderName)) return E_FAIL;
		m_vertexShaders.insert({ pszVertexShaderName, vertexShader });

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
		if (m_pixelShaders.contains(pszPixelShaderName)) return E_FAIL;
		m_pixelShaders.insert({ pszPixelShaderName, pixelShader });

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddScene

	  Summary:  Add scene to renderer

	  Args:     PCWSTR pszSceneName
				  The name of the scene
				const std::shared_ptr<Scene>&
				  The shared pointer to Scene

	  Modifies: [m_scenes].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
	{
		if (m_scenes.contains(pszSceneName))
		{
			return E_FAIL;
		}

		m_scenes[pszSceneName] = scene;

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::GetSceneOrNull

	  Summary:  Return scene with the given name or null

	  Args:     PCWSTR pszSceneName
				  The name of the scene

	  Returns:  std::shared_ptr<Scene>
				  The shared pointer to Scene
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
	{
		if (m_scenes.contains(pszSceneName))
		{
			return m_scenes[pszSceneName];
		}

		return nullptr;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetMainScene

	  Summary:  Set the main scene

	  Args:     PCWSTR pszSceneName
				  The name of the scene

	  Modifies: [m_pszMainSceneName].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
	{
		if (!m_scenes.contains(pszSceneName))
		{
			return E_FAIL;
		}

		m_pszMainSceneName = pszSceneName;

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::HandleInput

	  Summary:  Handle user mouse input

	  Args:     DirectionsInput& directions
				MouseRelativeMovement& mouseRelativeMovement
				FLOAT deltaTime
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
	{
		m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Update

	  Summary:  Update the renderables and models each frame

	  Args:     FLOAT deltaTime
				  Time difference of a frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::Update(_In_ FLOAT deltaTime)
	{
		for (const auto& renderable : m_renderables)
		{
			renderable.second->Update(deltaTime);
		}

		for (const auto& model : m_models)
		{
			model.second->Update(deltaTime);
		}

		for (const auto& light : m_aPointLights)
		{
			light->Update(deltaTime);
		}

		m_scenes[m_pszMainSceneName]->Update(deltaTime);

		m_camera.Update(deltaTime);
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Render

	  Summary:  Render the frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::Render()
	{
		// Clear the backbuffer
		constexpr float clearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; // RGBA
		m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);

		// Clear the depth buffer to 1.0 (maximum depth)
		m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Create camera constant buffer and update
		XMFLOAT4 camPos;
		XMStoreFloat4(&camPos, m_camera.GetEye());
		CBChangeOnCameraMovement cbCamera = {
			.View = XMMatrixTranspose(m_camera.GetView()),
			.CameraPosition = camPos,
		};

		m_immediateContext->UpdateSubresource(
			m_camera.GetConstantBuffer().Get(),
			0u,
			nullptr,
			&cbCamera,
			0u,
			0u
		);
		m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
		m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());

		// Create light constant buffer and update
		CBLights cbLights = { };

		for (int i = 0; i < NUM_LIGHTS; i++)
		{
			if (!m_aPointLights[i]) continue;
			cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
			cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
		}

		m_immediateContext->UpdateSubresource(
			m_cbLights.Get(),
			0u,
			nullptr,
			&cbLights,
			0u,
			0u
		);

		m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

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
			m_immediateContext->IASetIndexBuffer(renderable->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

			// Set the input layout
			m_immediateContext->IASetInputLayout(renderable->GetVertexLayout().Get());

			// Create and update renderable constant buffer
			CBChangesEveryFrame cbRenderable = {
				.World = XMMatrixTranspose(renderable->GetWorldMatrix()),
				.OutputColor = renderable->GetOutputColor(),
				.HasNormalMap = renderable->HasNormalMap()
			};

			m_immediateContext->UpdateSubresource(
				renderable->GetConstantBuffer().Get(),
				0u,
				nullptr,
				&cbRenderable,
				0u,
				0u
			);

			// Set shaders
			m_immediateContext->VSSetShader(renderable->GetVertexShader().Get(), nullptr, 0);
			m_immediateContext->PSSetShader(renderable->GetPixelShader().Get(), nullptr, 0);

			// Set renderable constant buffer
			m_immediateContext->VSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());
			m_immediateContext->PSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());

			const UINT numOfMesh = renderable->GetNumMeshes();
			for (UINT i = 0; i < numOfMesh; i++)
			{
				const auto& mesh = renderable->GetMesh(i);

				if (renderable->HasTexture())
				{
					const auto& material = renderable->GetMaterial(mesh.uMaterialIndex);
					const auto& diffuseView = material->pDiffuse->GetTextureResourceView();
					const auto& diffuseSampler = material->pDiffuse->GetSamplerState();

					m_immediateContext->PSSetShaderResources(0, 1, diffuseView.GetAddressOf());
					m_immediateContext->PSSetSamplers(0, 1, diffuseSampler.GetAddressOf());
				}

				m_immediateContext->DrawIndexed(mesh.uNumIndices, mesh.uBaseIndex, static_cast<INT>(mesh.uBaseVertex));
			}
		}

		auto& voxels = m_scenes[m_pszMainSceneName]->GetVoxels();
		for (auto& vox : voxels)
		{
			// Set the vertex buffer
			UINT vtxStride = sizeof(SimpleVertex);
			UINT vtxOffset = 0;

			m_immediateContext->IASetVertexBuffers(
				0,										// the first input slot
				1,										// the number of buffers
				vox->GetVertexBuffer().GetAddressOf(),
				&vtxStride,								// array of stride values, one for each buffer
				&vtxOffset
			);

			// Set the normal buffer
			UINT norStride = sizeof(NormalData);
			UINT norOffset = 0;

			m_immediateContext->IASetVertexBuffers(
				1, // second slot
				1,
				vox->GetNormalBuffer().GetAddressOf(),
				&norStride,
				&norOffset
			);

			// Set the instance buffer
			UINT insStride = sizeof(InstanceData);
			UINT insOffset = 0;

			m_immediateContext->IASetVertexBuffers(
				2, // third slot
				1,
				vox->GetInstanceBuffer().GetAddressOf(),
				&insStride,
				&insOffset
			);

			// Set the index buffer
			m_immediateContext->IASetIndexBuffer(vox->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

			// Set the input layout
			m_immediateContext->IASetInputLayout(vox->GetVertexLayout().Get());

			// Create and update voxel constant buffer
			CBChangesEveryFrame cbVoxel = {
				.World = XMMatrixTranspose(vox->GetWorldMatrix()),
				.OutputColor = vox->GetOutputColor(),
				.HasNormalMap = vox->HasNormalMap()
			};

			m_immediateContext->UpdateSubresource(
				vox->GetConstantBuffer().Get(),
				0u,
				nullptr,
				&cbVoxel,
				0u,
				0u
			);

			// Set shaders
			m_immediateContext->VSSetShader(vox->GetVertexShader().Get(), nullptr, 0);
			m_immediateContext->PSSetShader(vox->GetPixelShader().Get(), nullptr, 0);

			// Set constant buffer
			m_immediateContext->VSSetConstantBuffers(2, 1, vox->GetConstantBuffer().GetAddressOf());
			m_immediateContext->PSSetConstantBuffers(2, 1, vox->GetConstantBuffer().GetAddressOf());

			m_immediateContext->DrawIndexedInstanced(
				vox->GetNumIndices(),
				vox->GetNumInstances(),
				0,
				0,
				0
			);
		}

		// For each models
		for (auto& pair : m_models)
		{
			auto& model = pair.second;

			// Set the vertex buffer

			// First slot
			UINT vtxStride = sizeof(SimpleVertex);
			UINT vtxOffset = 0;
			m_immediateContext->IASetVertexBuffers(
				0,
				1,
				model->GetVertexBuffer().GetAddressOf(),
				&vtxStride,
				&vtxOffset
			);

			// Second slot
			UINT norStride = sizeof(NormalData);
			UINT norOffset = 0;
			m_immediateContext->IASetVertexBuffers(
				1,
				1,
				model->GetNormalBuffer().GetAddressOf(),
				&norStride,
				&norOffset
			);

			// Third slot
			UINT animStride = sizeof(AnimationData);
			UINT animOffset = 0;
			m_immediateContext->IASetVertexBuffers(
				2,
				1,
				model->GetAnimationBuffer().GetAddressOf(),
				&animStride,
				&animOffset
			);

			// Set the index buffer
			m_immediateContext->IASetIndexBuffer(model->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

			// Set the input layout
			m_immediateContext->IASetInputLayout(model->GetVertexLayout().Get());

			// Create and update renderable constant buffer
			CBChangesEveryFrame cbRenderable = {
				.World = XMMatrixTranspose(model->GetWorldMatrix()),
				.OutputColor = model->GetOutputColor(),
				.HasNormalMap = model->HasNormalMap()
			};

			m_immediateContext->UpdateSubresource(
				model->GetConstantBuffer().Get(),
				0u,
				nullptr,
				&cbRenderable,
				0u,
				0u
			);

			// Create and update skinning constant buffer
			CBSkinning cbSkinning = {};
			auto& transforms = model->GetBoneTransforms();
			for (UINT i = 0u; i < transforms.size(); i++)
			{
				cbSkinning.BoneTransforms[i] = transforms[i];
			}

			m_immediateContext->UpdateSubresource(
				model->GetSkinningConstantBuffer().Get(),
				0u,
				nullptr,
				&cbSkinning,
				0u,
				0u
			);

			// Set shaders
			m_immediateContext->VSSetShader(model->GetVertexShader().Get(), nullptr, 0);
			m_immediateContext->PSSetShader(model->GetPixelShader().Get(), nullptr, 0);

			// Set renderable constant buffer
			m_immediateContext->VSSetConstantBuffers(2, 1, model->GetConstantBuffer().GetAddressOf());
			m_immediateContext->VSSetConstantBuffers(4, 1, model->GetSkinningConstantBuffer().GetAddressOf());
			m_immediateContext->PSSetConstantBuffers(2, 1, model->GetConstantBuffer().GetAddressOf());


			const UINT numOfMesh = model->GetNumMeshes();
			for (UINT i = 0; i < numOfMesh; i++)
			{
				const auto& mesh = model->GetMesh(i);

				if (model->HasTexture())
				{
					const auto& material = model->GetMaterial(mesh.uMaterialIndex);
					const auto& diffuseView = material->pDiffuse->GetTextureResourceView();
					const auto& diffuseSampler = material->pDiffuse->GetSamplerState();

					m_immediateContext->PSSetShaderResources(0, 1, diffuseView.GetAddressOf());
					m_immediateContext->PSSetSamplers(0, 1, diffuseSampler.GetAddressOf());
				}

				m_immediateContext->DrawIndexed(mesh.uNumIndices, mesh.uBaseIndex, static_cast<INT>(mesh.uBaseVertex));
			}
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
		if (!m_renderables.contains(pszRenderableName) || !m_vertexShaders.contains(pszVertexShaderName))
		{
			return E_INVALIDARG;
		}
		const auto& vs = m_vertexShaders[pszVertexShaderName];
		m_renderables[pszRenderableName]->SetVertexShader(vs);
		return S_OK;
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
		if (!m_renderables.contains(pszRenderableName) || !m_pixelShaders.contains(pszPixelShaderName))
		{
			return E_INVALIDARG;
		}
		const auto& ps = m_pixelShaders[pszPixelShaderName];
		m_renderables[pszRenderableName]->SetPixelShader(ps);
		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetVertexShaderOfModel

	  Summary:  Sets the pixel shader for a model

	  Args:     PCWSTR pszModelName
				  Key of the model
				PCWSTR pszVertexShaderName
				  Key of the vertex shader

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::SetVertexShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszVertexShaderName)
	{
		if (!m_models.contains(pszModelName) || !m_vertexShaders.contains(pszVertexShaderName))
		{
			return E_INVALIDARG;
		}
		const auto& vs = m_vertexShaders[pszVertexShaderName];
		m_models[pszModelName]->SetVertexShader(vs);
		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetPixelShaderOfModel

	  Summary:  Sets the pixel shader for a model

	  Args:     PCWSTR pszModelName
				  Key of the model
				PCWSTR pszPixelShaderName
				  Key of the pixel shader

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::SetPixelShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszPixelShaderName)
	{
		if (!m_models.contains(pszModelName) || !m_pixelShaders.contains(pszPixelShaderName))
		{
			return E_INVALIDARG;
		}
		const auto& ps = m_pixelShaders[pszPixelShaderName];
		m_models[pszModelName]->SetPixelShader(ps);
		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetVertexShaderOfScene

	  Summary:  Sets the vertex shader for the voxels in a scene

	  Args:     PCWSTR pszSceneName
				  Key of the scene
				PCWSTR pszVertexShaderName
				  Key of the vertex shader

	  Modifies: [m_scenes].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName)
	{
		if (!m_scenes.contains(pszSceneName) || !m_vertexShaders.contains(pszVertexShaderName))
		{
			return E_INVALIDARG;
		}
		const auto& vs = m_vertexShaders[pszVertexShaderName];
		const auto& voxels = m_scenes[pszSceneName]->GetVoxels();

		for (const auto& vox : voxels)
		{
			vox->SetVertexShader(vs);
		}

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetPixelShaderOfScene

	  Summary:  Sets the pixel shader for the voxels in a scene

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable
				PCWSTR pszPixelShaderName
				  Key of the pixel shader

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName)
	{
		if (!m_scenes.contains(pszSceneName) || !m_pixelShaders.contains(pszPixelShaderName))
		{
			return E_INVALIDARG;
		}
		const auto& ps = m_pixelShaders[pszPixelShaderName];
		const auto& voxels = m_scenes[pszSceneName]->GetVoxels();

		for (const auto& vox : voxels)
		{
			vox->SetPixelShader(ps);
		}

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::GetDriverType

	  Summary:  Returns the Direct3D driver type

	  Returns:  D3D_DRIVER_TYPE
				  The Direct3D driver type used
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	D3D_DRIVER_TYPE Renderer::GetDriverType() const
	{
		return m_driverType;
	}
}







