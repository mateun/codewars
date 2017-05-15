#pragma once

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <Windows.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include "model.h"

using namespace DirectX;

bool importModel(const std::string& file, std::vector<XMFLOAT3>& positions,
	std::vector<XMFLOAT2>& uvs,
	std::vector<UINT>& indices);
bool importModel(const std::string& file, Model* model);
	