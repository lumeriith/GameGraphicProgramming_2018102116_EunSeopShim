#include "DrillingCube.h"

HRESULT DrillingCube::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
{
	HRESULT hr = BaseCube::Initialize(pDevice, pImmediateContext);
	if (FAILED(hr)) return hr;

	Scale(0.3f, 8.0f, 0.3f);

	return S_OK;
}

void DrillingCube::Update(_In_ FLOAT deltaTime)
{
	// 2,000RPM Yuna Kim Cube.
	RotateY(deltaTime * 16.0f);
	Translate(XMVectorSet(0.0f, -1.0f * deltaTime, 0.0f, 0.0f));
}
