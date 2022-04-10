#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class DrillingCube : public BaseCube
{
public:
	DrillingCube() = default;
	DrillingCube(const DrillingCube& other) = delete;
	DrillingCube(DrillingCube&& other) = delete;
	DrillingCube& operator=(const DrillingCube& other) = delete;
	DrillingCube& operator=(DrillingCube&& other) = delete;
	~DrillingCube() = default;

	virtual HRESULT Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext) override;
	virtual void Update(_In_ FLOAT deltaTime) override;
};