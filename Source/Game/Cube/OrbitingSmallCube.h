#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class OrbitingSmallCube : public BaseCube
{
public:
	OrbitingSmallCube();
	void Update(_In_ FLOAT deltaTime);
private:
	float m_elapsedTime;
	BYTE m_padding[12];
};