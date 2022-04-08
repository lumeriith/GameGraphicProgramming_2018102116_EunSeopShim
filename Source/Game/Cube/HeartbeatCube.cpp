#include "HeartbeatCube.h"

HeartbeatCube::HeartbeatCube() : framesCount(0)
{
	XMFLOAT3 trVector(0.0f, 4.0f, 0.0f);
	Translate(XMLoadFloat3(&trVector));
}

void HeartbeatCube::Update(_In_ FLOAT deltaTime)
{
	float scale = 0.9f;

	if (framesCount / 50 % 2 == 0)
	{
		Scale(scale, scale, scale);
	}
	else
	{
		Scale(1 / scale, 1 / scale, 1 / scale);
	}

	framesCount++;
}
