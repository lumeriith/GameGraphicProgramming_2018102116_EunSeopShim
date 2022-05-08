#include "Cube/BaseCube.h"

BaseCube::BaseCube(_In_ const XMFLOAT4& outputColor)
	: Renderable(outputColor)
{}

HRESULT BaseCube::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
{
	BasicMeshEntry entry;
	entry.uNumIndices = NUM_INDICES;
	m_aMeshes.push_back(entry);

	return initialize(pDevice, pImmediateContext);
}

UINT BaseCube::GetNumVertices() const
{
	return NUM_VERTICES;
}

UINT BaseCube::GetNumIndices() const
{
	return NUM_INDICES;
}

const library::SimpleVertex* BaseCube::getVertices() const
{
	return VERTICES;
}


const WORD* BaseCube::getIndices() const
{
	return INDICES;
}