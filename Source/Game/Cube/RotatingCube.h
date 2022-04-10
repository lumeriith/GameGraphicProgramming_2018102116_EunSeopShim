#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class RotatingCube : public BaseCube
{
public:
	RotatingCube() = default;
	RotatingCube(const RotatingCube& other) = delete;
	RotatingCube(RotatingCube&& other) = delete;
	RotatingCube& operator=(const RotatingCube& other) = delete;
	RotatingCube& operator=(RotatingCube&& other) = delete;
	~RotatingCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;
};