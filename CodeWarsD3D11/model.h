#pragma once
#include <DirectXMath.h>
#include <string>
#include <vector>

using namespace DirectX;

struct Model {
	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT2> uvs;
	std::vector<unsigned int> indices;
};
