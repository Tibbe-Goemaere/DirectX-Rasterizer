#pragma once
#include "Math.h"

using namespace dae;

struct Vertex final
{
	Vector3 position{};
	Vector2 uv{}; 
	Vector3 normal{};
	Vector3 tangent{};
};

struct Vertex_Out final
{
	Vector3 position{};
	Vector2 uv{};
	Vector3 normal{};
	Vector3 tangent{};
	//Vector3 viewDirection{};
};

