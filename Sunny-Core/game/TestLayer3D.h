#pragma once

#include <iostream>
#include "../graphics/layers/Layer3D.h"
#include "../app/Application.h"
#include "../graphics/shaders/ShaderFactory.h"

using namespace std;
using namespace sunny;
using namespace graphics;
using namespace directx;
class TestLayer3D : public Layer3D
{
public:

public:
	TestLayer3D();
	~TestLayer3D();

	void OnInit(Renderer3D& renderer) override;
	void OnTick() override;
	void OnUpdate(const utils::Timestep& ts) override;
};