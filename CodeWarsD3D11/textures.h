#pragma once
#include <d3d11.h>
#include <FreeImage.h>
#include <string>
#include "renderer.h"


HRESULT loadTextureFromFile(const std::string& fileName, ID3D11Texture2D** textureTarget, Renderer* renderer);


