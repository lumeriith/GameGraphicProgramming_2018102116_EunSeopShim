#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class RotatingCube : public BaseCube
{
	void Update(_In_ FLOAT deltaTime);
};