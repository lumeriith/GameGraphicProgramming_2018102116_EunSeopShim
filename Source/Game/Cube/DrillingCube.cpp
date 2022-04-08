#include "DrillingCube.h"

HRESULT DrillingCube::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
{
	HRESULT hr = BaseCube::Initialize(pDevice, pImmediateContext);
	if (FAILED(hr)) return hr;

	Scale(0.3f, 2.0f, 0.3f);

	XMFLOAT3 trVector(0.0f, 2.0f, 0.0f);
	Translate(XMLoadFloat3(&trVector));
}

void DrillingCube::Update(_In_ FLOAT deltaTime)
{
	// 2,000RPM Yuna Kim Cube
	RotateY(deltaTime * 16.0f);
}