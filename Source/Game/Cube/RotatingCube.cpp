#include "Cube/RotatingCube.h"

RotatingCube::RotatingCube(const XMFLOAT4& outputColor)
	: BaseCube(outputColor)
{
}

void RotatingCube::Update(_In_ FLOAT deltaTime)
{
	XMMATRIX mOrbit = XMMatrixRotationY(-2.0f * deltaTime);

	m_world = m_world * mOrbit;
}