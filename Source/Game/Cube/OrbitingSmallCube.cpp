#include "OrbitingSmallCube.h"

OrbitingSmallCube::OrbitingSmallCube() : m_elapsedTime(0) {}

void OrbitingSmallCube::Update(_In_ FLOAT deltaTime)
{
	m_elapsedTime += deltaTime;

	m_world = XMMatrixIdentity();

	RotateZ(-1.0f * m_elapsedTime);

	Scale(0.3f, 0.3f, 0.3f);

	XMFLOAT3 trVector(4.0f, 0.0f, 0.0f);
	Translate(XMLoadFloat3(&trVector));

	RotateY(2.0f * -1.0f * m_elapsedTime);
}
