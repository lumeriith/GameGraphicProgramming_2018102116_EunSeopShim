/*+===================================================================
  File:      MAIN.CPP

  Summary:   This application demonstrates creating a Direct3D 11 device

  Origin:    http://msdn.microsoft.com/en-us/library/windows/apps/ff729718.aspx

  Originally created by Microsoft Corporation under MIT License
  ?2022 Kyung Hee University
===================================================================+*/

#include "Common.h"

#include <cstdio>
#include <memory>

#include "Game/Game.h"

#include "Cube/Cube.h"
#include "Cube/RotatingCube.h"
#include "Light/RotatingPointLight.h"
#include "Model/Model.h"

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: wWinMain

  Summary:  Entry point to the program. Initializes everything and
			goes into a message processing loop. Idle time is used to
			render the scene.

  Args:     HINSTANCE hInstance
			  Handle to an instance.
			HINSTANCE hPrevInstance
			  Has no meaning.
			LPWSTR lpCmdLine
			  Contains the command-line arguments as a Unicode
			  string
			INT nCmdShow
			  Flag that says whether the main application window
			  will be minimized, maximized, or shown normally

  Returns:  INT
			  Status code.
-----------------------------------------------------------------F-F*/
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"Game Graphics Programming Lab 07: Modeling");
	const auto& renderer = game->GetRenderer();
	const auto& loadShader = [&](PCWSTR pszPath, PCWSTR pszName, PCSTR pszVSEntry, PCSTR pszVSModel, PCSTR pszPSEntry, PCSTR pszPSModel)
	{
		HRESULT hr;

		std::shared_ptr<library::VertexShader> vs = std::make_shared<library::VertexShader>(pszPath, pszVSEntry, pszVSModel);
		hr = renderer->AddVertexShader(pszName, vs);
		if (FAILED(hr)) return hr;

		std::shared_ptr<library::PixelShader> ps = std::make_shared<library::PixelShader>(pszPath, pszPSEntry, pszPSModel);
		hr = renderer->AddPixelShader(pszName, ps);
		if (FAILED(hr)) return hr;

		return S_OK;
	};
	const auto& loadModel = [&](PCWSTR pszPath, PCWSTR pszName, PCWSTR pszShader, float xOffset = 0)
	{
		HRESULT hr;

		std::shared_ptr<library::Model> model = std::make_shared<library::Model>(pszPath);
		model->Translate(XMVectorSet(xOffset, 0.0f, 0.0f, 0.0f));

		hr = renderer->AddRenderable(pszName, model);
		if (FAILED(hr)) return hr;

		hr = renderer->SetVertexShaderOfRenderable(pszName, pszShader);
		if (FAILED(hr)) return hr;

		hr = renderer->SetPixelShaderOfRenderable(pszName, pszShader);
		if (FAILED(hr)) return hr;

		return S_OK;
	};


	HRESULT hr;

	// Light Cube Shaders
	hr = loadShader(
		L"Shaders/PhongShaders.fxh", L"LightShader",
		"VSLightCube", "vs_5_0",
		"PSLightCube", "ps_5_0"
	);
	if (FAILED(hr)) return 0;

	// Phong Shaders
	hr = loadShader(
		L"Shaders/PhongShaders.fxh", L"PhongShader",
		"VSPhong", "vs_5_0",
		"PSPhong", "ps_5_0"
	);
	if (FAILED(hr)) return 0;

	hr = loadModel(L"../../Content/nanosuit/nanosuit.obj", L"NanoSuitModel", L"PhongShader");
	if (FAILED(hr)) return 0;

#ifdef SHOW_OTHER_MODELS
	hr = loadModel(L"../../Content/backpack/backpack.obj", L"BackpackModel", L"PhongShader", 5.0f);
	if (FAILED(hr)) return 0;

	hr = loadModel(L"../../Content/cyborg/cyborg.obj", L"CyborgModel", L"PhongShader", -5.0f);
	if (FAILED(hr)) return 0;
#endif

	XMFLOAT4 color;
	XMStoreFloat4(&color, Colors::White);

	std::shared_ptr<library::PointLight> directionalLight = std::make_shared<library::PointLight>(
		XMFLOAT4(-5.77f, 5.77f, -5.77f, 1.0f),
		color
		);
	if (FAILED(renderer->AddPointLight(0, directionalLight)))
	{
		return 0;
	}

	std::shared_ptr<Cube> lightCube = std::make_shared<Cube>(color);
	lightCube->Translate(XMVectorSet(-5.77f, 5.77f, -5.77f, 0.0f));
	if (FAILED(renderer->AddRenderable(L"LightCube", lightCube)))
	{
		return 0;
	}
	if (FAILED(renderer->SetVertexShaderOfRenderable(L"LightCube", L"LightShader")))
	{
		return 0;
	}
	if (FAILED(renderer->SetPixelShaderOfRenderable(L"LightCube", L"LightShader")))
	{
		return 0;
	}

	XMStoreFloat4(&color, Colors::Red);
	std::shared_ptr<RotatingPointLight> rotatingDirectionalLight = std::make_shared<RotatingPointLight>(
		XMFLOAT4(0.0f, 0.0f, -5.0f, 1.0f),
		color
		);
	if (FAILED(renderer->AddPointLight(1, rotatingDirectionalLight)))
	{
		return 0;
	}

	std::shared_ptr<RotatingCube> rotatingLightCube = std::make_shared<RotatingCube>(color);
	rotatingLightCube->Translate(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
	if (FAILED(renderer->AddRenderable(L"RotatingLightCube", rotatingLightCube)))
	{
		return 0;
	}
	if (FAILED(renderer->SetVertexShaderOfRenderable(L"RotatingLightCube", L"LightShader")))
	{
		return 0;
	}
	if (FAILED(renderer->SetPixelShaderOfRenderable(L"RotatingLightCube", L"LightShader")))
	{
		return 0;
	}

	if (FAILED(game->Initialize(hInstance, nCmdShow)))
	{
		return 0;
	}

	return game->Run();
}