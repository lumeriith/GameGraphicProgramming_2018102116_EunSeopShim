#include "Renderer/Renderable.h"

namespace library
{
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::Renderable

	  Summary:  Constructor

	  Args:     const std::filesystem::path& textureFilePath
				  Path to the texture to use

	  Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
				 m_textureRV, m_samplerLinear, m_vertexShader,
				 m_pixelShader, m_textureFilePath, m_world].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	Renderable::Renderable(_In_ const std::filesystem::path& textureFilePath)
	{
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::initialize

	  Summary:  Initializes the buffers, texture, and the world matrix

	  Args:     ID3D11Device* pDevice
				  The Direct3D device to create the buffers
				ID3D11DeviceContext* pImmediateContext
				  The Direct3D context to set buffers

	  Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
				 m_textureRV, m_samplerLinear, m_world].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderable::initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
	{
		UNREFERENCED_PARAMETER(pImmediateContext);
		HRESULT hr;

		// Create the vertex buffer
		D3D11_BUFFER_DESC vBufferDesc = {
			.ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0
		};

		D3D11_SUBRESOURCE_DATA vData = {
			.pSysMem = getVertices(),
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		hr = pDevice->CreateBuffer(&vBufferDesc, &vData, &m_vertexBuffer);
		if (FAILED(hr)) return hr;

		// Create the index buffer
		D3D11_BUFFER_DESC iBufferDesc = {
			.ByteWidth = sizeof(WORD) * GetNumIndices(),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_INDEX_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0
		};

		D3D11_SUBRESOURCE_DATA iData = {
			.pSysMem = getIndices(),
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		hr = pDevice->CreateBuffer(&iBufferDesc, &iData, &m_indexBuffer);
		if (FAILED(hr)) return hr;

		// Create the constant buffer
		D3D11_BUFFER_DESC cBufferDesc = {
			.ByteWidth = sizeof(ConstantBuffer),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		ConstantBuffer cb = {};

		D3D11_SUBRESOURCE_DATA cData = {
			.pSysMem = &cb,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		hr = pDevice->CreateBuffer(&cBufferDesc, &cData, &m_constantBuffer);
		if (FAILED(hr)) return hr;

		// Initialize the world matrix (Use identity matrix)
		m_world = XMMatrixIdentity();

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::SetVertexShader

	  Summary:  Sets the vertex shader to be used for this renderable
				object

	  Args:     const std::shared_ptr<VertexShader>& vertexShader
				  Vertex shader to set to

	  Modifies: [m_vertexShader].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
	{
		m_vertexShader = vertexShader;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::SetPixelShader

	  Summary:  Sets the pixel shader to be used for this renderable
				object

	  Args:     const std::shared_ptr<PixelShader>& pixelShader
				  Pixel shader to set to

	  Modifies: [m_pixelShader].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
	{
		m_pixelShader = pixelShader;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetVertexShader

	  Summary:  Returns the vertex shader

	  Returns:  ComPtr<ID3D11VertexShader>&
				  Vertex shader. Could be a nullptr
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
	{
		return m_vertexShader->GetVertexShader();
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetPixelShader

	  Summary:  Returns the vertex shader

	  Returns:  ComPtr<ID3D11PixelShader>&
				  Pixel shader. Could be a nullptr
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
	{
		return m_pixelShader->GetPixelShader();
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetVertexLayout

	  Summary:  Returns the vertex input layout

	  Returns:  ComPtr<ID3D11InputLayout>&
				  Vertex input layout
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
	{
		return m_vertexShader->GetVertexLayout();
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetVertexBuffer

	  Summary:  Returns the vertex buffer

	  Returns:  ComPtr<ID3D11Buffer>&
				  Vertex buffer
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
	{
		return m_vertexBuffer;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetIndexBuffer

	  Summary:  Returns the index buffer

	  Returns:  ComPtr<ID3D11Buffer>&
				  Index buffer
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
	{
		return m_indexBuffer;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetConstantBuffer

	  Summary:  Returns the constant buffer

	  Returns:  ComPtr<ID3D11Buffer>&
				  Constant buffer
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
	{
		return m_constantBuffer;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
		  Method:   Renderable::GetWorldMatrix

		  Summary:  Returns the world matrix

		  Returns:  const XMMATRIX&
					  World matrix
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	const XMMATRIX& Renderable::GetWorldMatrix() const
	{
		return m_world;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetTextureResourceView

	  Summary:  Returns the texture resource view

	  Returns:  ComPtr<ID3D11ShaderResourceView>&
				  The texture resource view
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11ShaderResourceView>& Renderable::GetTextureResourceView()
	{
		return m_textureRV;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetSamplerState

	  Summary:  Returns the sampler state

	  Returns:  ComPtr<ID3D11SamplerState>&
				  The sampler state
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11SamplerState>& Renderable::GetSamplerState()
	{
		return m_samplerLinear;
	}
}
