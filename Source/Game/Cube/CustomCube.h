#pragma once

#include "Common.h"
#include "Cube/BaseCube.h"

class CustomCube : public BaseCube
{
public:
	CustomCube(const std::filesystem::path& textureFilePath);
	CustomCube(const CustomCube& other) = delete;
	CustomCube(CustomCube&& other) = delete;
	CustomCube& operator=(const CustomCube& other) = delete;
	CustomCube& operator=(CustomCube&& other) = delete;
	~CustomCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;
};