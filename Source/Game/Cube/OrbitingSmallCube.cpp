#include "OrbitingSmallCube.h"

HRESULT OrbitingSmallCube::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
{
	HRESULT hr = BaseCube::Initialize(pDevice, pImmediateContext);
	if (FAILED(hr)) return hr;

	Scale(0.7f, 0.7f, 0.7f);

	XMFLOAT3 trVector(4.0f, 0.0f, 0.0f);
	Translate(XMLoadFloat3(&trVector));
}

void OrbitingSmallCube::Update(_In_ FLOAT deltaTime)
{
	RotateY(2.0f * -1.0f * deltaTime);
	RotateZ(-1.0f * deltaTime);
}
