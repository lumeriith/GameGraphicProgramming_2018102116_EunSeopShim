#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class HeartbeatCube : public BaseCube
{
	HeartbeatCube();
	void Update(_In_ FLOAT deltaTime);

private:
	int framesCount;
};