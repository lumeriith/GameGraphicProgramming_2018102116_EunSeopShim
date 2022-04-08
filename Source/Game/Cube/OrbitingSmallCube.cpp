#include "OrbitingSmallCube.h"

OrbitingSmallCube::OrbitingSmallCube()
{
	Scale(0.7f, 0.7f, 0.7f);

	XMFLOAT3 trVector(4.0f, 0.0f, 0.0f);
	Translate(XMLoadFloat3(&trVector));
}

void OrbitingSmallCube::Update(_In_ FLOAT deltaTime)
{
	RotateY(2.0f * -1.0f * deltaTime);
	RotateZ(-1.0f * deltaTime);
}
