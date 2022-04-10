#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class OrbitingSmallCube : public BaseCube
{
public:
	OrbitingSmallCube();
	OrbitingSmallCube(const OrbitingSmallCube& other) = delete;
	OrbitingSmallCube(OrbitingSmallCube&& other) = delete;
	OrbitingSmallCube& operator=(const OrbitingSmallCube& other) = delete;
	OrbitingSmallCube& operator=(OrbitingSmallCube&& other) = delete;
	~OrbitingSmallCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;
private:
	float m_elapsedTime;
	BYTE m_padding[12];
};