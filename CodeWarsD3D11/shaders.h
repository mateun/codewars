#pragma once
#include <d3d11.h>

void CreateVertexShader(const ID3D11Device* device, ID3DBlob* vsBlob, ID3D11VertexShader** vs);
void CreatePixelShader(const ID3D11Device* device, ID3DBlob* psBlob, ID3D11PixelShader** ps);

