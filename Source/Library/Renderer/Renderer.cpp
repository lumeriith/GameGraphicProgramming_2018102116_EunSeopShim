#include "Renderer/Renderer.h"

namespace library
{

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Renderer

	  Summary:  Constructor

	  Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
				  m_immediateContext, m_immediateContext1, m_swapChain,
				  m_swapChain1, m_renderTargetView, m_depthStencil,
				  m_depthStencilView, m_cbChangeOnResize, m_cbShadowMatrix,
				  m_pszMainSceneName, m_camera, m_projection, m_scenes
				  m_invalidTexture, m_shadowMapTexture, m_shadowVertexShader,
				  m_shadowPixelShader].
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
		, m_shadowMapTexture()
		, m_shadowVertexShader()
		, m_shadowPixelShader()
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
				  m_vertexLayout, m_pixelShader, m_vertexBuffer
				  m_cbShadowMatrix].

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
				.Windowed = TRUE,
				.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
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

		// Projection Constant Buffer
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

		m_projection = XMMatrixPerspectiveFovLH(
			XM_PIDIV4,
			static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight),
			0.01f,
			1000.0f
		);

		CBChangeOnResize cbChangesOnResize =
		{
			.Projection = XMMatrixTranspose(m_projection)
		};
		m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);
		m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());

		// Light Constant Buffer
		bd.ByteWidth = sizeof(CBLights);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0u;

		hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}

		m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
		m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

		// Shadow Constant Buffer
		bd.ByteWidth = sizeof(CBShadowMatrix);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0u;

		hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbShadowMatrix.GetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}

		// Create Shadow Map Texture
		m_shadowMapTexture = std::make_shared<RenderTexture>(uWidth, uHeight);

		// Initialize
		hr = m_shadowMapTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
		if (FAILED(hr)) return hr;

		hr = m_camera.Initialize(m_d3dDevice.Get());
		if (FAILED(hr)) return hr;

		if (!m_scenes.contains(m_pszMainSceneName))
		{
			return E_FAIL;
		}

		const auto& mainScene = m_scenes[m_pszMainSceneName];

		hr = mainScene->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
		if (FAILED(hr))
		{
			return hr;
		}

		for (UINT i = 0u; i < NUM_LIGHTS; i++)
		{
			mainScene->GetPointLight(i)->Initialize(uWidth, uHeight);
		}

		hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
		if (FAILED(hr))
		{
			return hr;
		}

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
	  Method:   Renderer::SetShadowMapShaders

	  Summary:  Set shaders for the shadow mapping

	  Args:     std::shared_ptr<ShadowVertexShader>
				  vertex shader
				std::shared_ptr<PixelShader>
				  pixel shader

	  Modifies: [m_shadowVertexShader, m_shadowPixelShader].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::SetShadowMapShaders(_In_ std::shared_ptr<ShadowVertexShader> vertexShader, _In_ std::shared_ptr<PixelShader> pixelShader)
	{
		m_shadowVertexShader = move(vertexShader);
		m_shadowPixelShader = move(pixelShader);
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

	  Summary:  Update the renderables each frame

	  Args:     FLOAT deltaTime
				  Time difference of a frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::Update(_In_ FLOAT deltaTime)
	{
		m_scenes[m_pszMainSceneName]->Update(deltaTime);

		m_camera.Update(deltaTime);
	}


	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Render

	  Summary:  Render the frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::Render()
	{
		RenderSceneToTexture();

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

		const auto& mainScene = m_scenes[m_pszMainSceneName];

		// Create light constant buffer and update
		CBLights cbLights = { };

		for (UINT i = 0u; i < NUM_LIGHTS; i++)
		{
			const auto& light = mainScene->GetPointLight(i);
			const float attDist = light->GetAttenuationDistance();
			const float sqrAttDist = attDist * attDist;
			auto& data = cbLights.PointLights[i];
			if (!light) continue;

			data.Position = light->GetPosition();
			data.Color = light->GetColor();
			data.View = XMMatrixTranspose(light->GetViewMatrix());
			data.Projection = XMMatrixTranspose(light->GetProjectionMatrix());
			data.AttenuationDistance = XMFLOAT4(
				attDist,
				attDist,
				sqrAttDist,
				sqrAttDist
			);
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

		// Shadow texture and sampler state
		m_immediateContext->PSSetShaderResources(2, 1, m_shadowMapTexture->GetShaderResourceView().GetAddressOf());
		m_immediateContext->PSSetSamplers(2, 1, m_shadowMapTexture->GetSamplerState().GetAddressOf());

		// For each renderables
		for (const auto& pair : mainScene->GetRenderables())
		{
			const auto& renderable = pair.second;

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

			// Set the normal buffer
			UINT norStride = sizeof(NormalData);
			UINT norOffset = 0;

			m_immediateContext->IASetVertexBuffers(
				1, // second slot
				1,
				renderable->GetNormalBuffer().GetAddressOf(),
				&norStride,
				&norOffset
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
					const auto& diffuseSampler = Texture::s_samplers[static_cast<size_t>(material->pDiffuse->GetSamplerType())];

					m_immediateContext->PSSetShaderResources(0, 1, diffuseView.GetAddressOf());
					m_immediateContext->PSSetSamplers(0, 1, diffuseSampler.GetAddressOf());

					if (renderable->HasNormalMap())
					{
						const auto& normalView = material->pNormal->GetTextureResourceView();
						const auto& normalSampler = Texture::s_samplers[static_cast<size_t>(material->pNormal->GetSamplerType())];

						m_immediateContext->PSSetShaderResources(1, 1, normalView.GetAddressOf());
						m_immediateContext->PSSetSamplers(1, 1, normalSampler.GetAddressOf());
					}
				}

				m_immediateContext->DrawIndexed(mesh.uNumIndices, mesh.uBaseIndex, static_cast<INT>(mesh.uBaseVertex));
			}
		}

		for (auto& vox : mainScene->GetVoxels())
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


			const UINT numOfMesh = vox->GetNumMeshes();
			for (UINT i = 0; i < numOfMesh; i++)
			{
				const auto& mesh = vox->GetMesh(i);

				if (vox->HasTexture())
				{
					const auto& material = vox->GetMaterial(mesh.uMaterialIndex);

					const auto& diffuseView = material->pDiffuse->GetTextureResourceView();
					const auto& diffuseSampler = Texture::s_samplers[static_cast<size_t>(material->pDiffuse->GetSamplerType())];

					const auto& normalView = material->pNormal->GetTextureResourceView();
					const auto& normalSampler = Texture::s_samplers[static_cast<size_t>(material->pNormal->GetSamplerType())];

					m_immediateContext->PSSetShaderResources(0, 1, diffuseView.GetAddressOf());
					m_immediateContext->PSSetSamplers(0, 1, diffuseSampler.GetAddressOf());

					m_immediateContext->PSSetShaderResources(1, 1, normalView.GetAddressOf());
					m_immediateContext->PSSetSamplers(1, 1, normalSampler.GetAddressOf());
				}

				m_immediateContext->DrawIndexedInstanced(
					mesh.uNumIndices,
					vox->GetNumInstances(),
					mesh.uBaseIndex,
					static_cast<INT>(mesh.uBaseVertex),
					0
				);
			}
		}

		for (auto& pair : mainScene->GetModels())
		{
			auto& model = pair.second;

			// Set the vertex buffer

			// First slot
			UINT stride0 = sizeof(SimpleVertex);
			UINT offset0 = 0;
			m_immediateContext->IASetVertexBuffers(
				0,
				1,
				model->GetVertexBuffer().GetAddressOf(),
				&stride0,
				&offset0
			);

			// Second slot
			UINT stride1 = sizeof(NormalData);
			UINT offset1 = 0;
			m_immediateContext->IASetVertexBuffers(
				1,
				1,
				model->GetNormalBuffer().GetAddressOf(),
				&stride1,
				&offset1
			);

			// Third slot
			UINT stride2 = sizeof(AnimationData);
			UINT offset2 = 0;
			m_immediateContext->IASetVertexBuffers(
				2,
				1,
				model->GetAnimationBuffer().GetAddressOf(),
				&stride2,
				&offset2
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
					const auto& diffuseSampler = Texture::s_samplers[static_cast<size_t>(material->pDiffuse->GetSamplerType())];

					const auto& normalView = material->pNormal->GetTextureResourceView();
					const auto& normalSampler = Texture::s_samplers[static_cast<size_t>(material->pNormal->GetSamplerType())];

					m_immediateContext->PSSetShaderResources(0, 1, diffuseView.GetAddressOf());
					m_immediateContext->PSSetSamplers(0, 1, diffuseSampler.GetAddressOf());

					m_immediateContext->PSSetShaderResources(1, 1, normalView.GetAddressOf());
					m_immediateContext->PSSetSamplers(1, 1, normalSampler.GetAddressOf());
				}

				m_immediateContext->DrawIndexed(mesh.uNumIndices, mesh.uBaseIndex, static_cast<INT>(mesh.uBaseVertex));
			}
		}

		const auto& skyBox = mainScene->GetSkyBox();
		if (skyBox)
		{
			// Set the vertex buffer
			UINT stride = sizeof(SimpleVertex);
			UINT offset = 0;

			m_immediateContext->IASetVertexBuffers(
				0,												// the first input slot for binding
				1,												// the number of buffers in the array
				skyBox->GetVertexBuffer().GetAddressOf(),	// the array of vertex buffers
				&stride,										// array of stride values, one for each buffer
				&offset
			);

			// Set the index buffer
			m_immediateContext->IASetIndexBuffer(skyBox->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

			// Set the input layout
			m_immediateContext->IASetInputLayout(skyBox->GetVertexLayout().Get());

			// Create and update renderable constant buffer
			CBChangesEveryFrame cbRenderable = {
				.World = XMMatrixTranspose(skyBox->GetWorldMatrix()),
				.OutputColor = skyBox->GetOutputColor(),
				.HasNormalMap = skyBox->HasNormalMap()
			};

			m_immediateContext->UpdateSubresource(
				skyBox->GetConstantBuffer().Get(),
				0u,
				nullptr,
				&cbRenderable,
				0u,
				0u
			);

			// Set shaders
			m_immediateContext->VSSetShader(skyBox->GetVertexShader().Get(), nullptr, 0);
			m_immediateContext->PSSetShader(skyBox->GetPixelShader().Get(), nullptr, 0);

			// Set renderable constant buffer
			m_immediateContext->VSSetConstantBuffers(2, 1, skyBox->GetConstantBuffer().GetAddressOf());
			m_immediateContext->PSSetConstantBuffers(2, 1, skyBox->GetConstantBuffer().GetAddressOf());

			const UINT numOfMesh = skyBox->GetNumMeshes();
			for (UINT i = 0; i < numOfMesh; i++)
			{
				const auto& mesh = skyBox->GetMesh(i);

				if (skyBox->HasTexture())
				{
					const auto& material = skyBox->GetMaterial(mesh.uMaterialIndex);

					const auto& diffuseView = material->pDiffuse->GetTextureResourceView();
					const auto& diffuseSampler = Texture::s_samplers[static_cast<size_t>(material->pDiffuse->GetSamplerType())];

					m_immediateContext->PSSetShaderResources(0, 1, diffuseView.GetAddressOf());
					m_immediateContext->PSSetSamplers(0, 1, diffuseSampler.GetAddressOf());

					if (skyBox->HasNormalMap())
					{
						const auto& normalView = material->pNormal->GetTextureResourceView();
						const auto& normalSampler = Texture::s_samplers[static_cast<size_t>(material->pNormal->GetSamplerType())];

						m_immediateContext->PSSetShaderResources(1, 1, normalView.GetAddressOf());
						m_immediateContext->PSSetSamplers(1, 1, normalSampler.GetAddressOf());
					}
				}

				m_immediateContext->DrawIndexed(mesh.uNumIndices, mesh.uBaseIndex, static_cast<INT>(mesh.uBaseVertex));
			}
		}


		// Present
		m_swapChain->Present(0, 0);

		// Set Render Target View again (Present call for DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL unbinds backbuffer 0)
		m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

		// Unbind shadow texture so fake render can write to it
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		m_immediateContext->PSSetShaderResources(2, 1, nullSRV);


		// Unbind vertex slots so RenderSceneToTexture doesn't complain
		ID3D11Buffer* nullVB[3] = { nullptr, nullptr, nullptr };
		UINT zero = 0;
		m_immediateContext->IASetVertexBuffers(
			0,
			3,
			nullVB,
			&zero,
			&zero
		);
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::RenderSceneToTexture

	  Summary:  Render scene to the texture
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::RenderSceneToTexture()
	{
		m_immediateContext->OMSetRenderTargets(
			1,
			m_shadowMapTexture->GetRenderTargetView().GetAddressOf(),
			m_depthStencilView.Get()
		);

		m_immediateContext->ClearRenderTargetView(m_shadowMapTexture->GetRenderTargetView().Get(), Colors::White);
		m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		const auto& scene = m_scenes[m_pszMainSceneName];

		// Set shaders
		m_immediateContext->VSSetShader(m_shadowVertexShader->GetVertexShader().Get(), nullptr, 0);
		m_immediateContext->PSSetShader(m_shadowPixelShader->GetPixelShader().Get(), nullptr, 0);

		const auto& light = m_scenes[m_pszMainSceneName]->GetPointLight(0);

		for (const auto& pair : scene->GetRenderables())
		{
			const auto& renderable = pair.second;

			// Set the vertex buffer
			UINT stride = sizeof(SimpleVertex);
			UINT offset = 0;

			m_immediateContext->IASetVertexBuffers(
				0,
				1,
				renderable->GetVertexBuffer().GetAddressOf(),
				&stride,
				&offset
			);

			// Set the index buffer
			m_immediateContext->IASetIndexBuffer(renderable->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

			// Set the input layout
			m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());

			// Shadow constant buffer
			CBShadowMatrix cbShadow = {
				.World = XMMatrixTranspose(renderable->GetWorldMatrix()),
				.View = XMMatrixTranspose(light->GetViewMatrix()),
				.Projection = XMMatrixTranspose(light->GetProjectionMatrix()),
				.IsVoxel = false
			};

			m_immediateContext->UpdateSubresource(
				m_cbShadowMatrix.Get(),
				0u,
				nullptr,
				&cbShadow,
				0u,
				0u
			);

			m_immediateContext->VSSetConstantBuffers(0, 1, m_cbShadowMatrix.GetAddressOf());

			const UINT numOfMesh = renderable->GetNumMeshes();
			for (UINT i = 0; i < numOfMesh; i++)
			{
				const auto& mesh = renderable->GetMesh(i);
				m_immediateContext->DrawIndexed(mesh.uNumIndices, mesh.uBaseIndex, static_cast<INT>(mesh.uBaseVertex));
			}
		}

		for (auto& pair : scene->GetModels())
		{
			auto& model = pair.second;

			// Set the vertex buffer
			UINT stride0 = sizeof(SimpleVertex);
			UINT offset0 = 0;
			m_immediateContext->IASetVertexBuffers(
				0,
				1,
				model->GetVertexBuffer().GetAddressOf(),
				&stride0,
				&offset0
			);

			// Set the index buffer
			m_immediateContext->IASetIndexBuffer(model->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

			// Set the input layout
			m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());

			// Shadow constant buffer
			CBShadowMatrix cbShadow = {
				.World = XMMatrixTranspose(model->GetWorldMatrix()),
				.View = XMMatrixTranspose(light->GetViewMatrix()),
				.Projection = XMMatrixTranspose(light->GetProjectionMatrix()),
				.IsVoxel = false
			};

			m_immediateContext->UpdateSubresource(
				m_cbShadowMatrix.Get(),
				0u,
				nullptr,
				&cbShadow,
				0u,
				0u
			);

			m_immediateContext->VSSetConstantBuffers(0, 1, m_cbShadowMatrix.GetAddressOf());

			const UINT numOfMesh = model->GetNumMeshes();
			for (UINT i = 0; i < numOfMesh; i++)
			{
				const auto& mesh = model->GetMesh(i);
				m_immediateContext->DrawIndexed(mesh.uNumIndices, mesh.uBaseIndex, static_cast<INT>(mesh.uBaseVertex));
			}
		}

		// Reset RT back to original back buffer
		m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
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