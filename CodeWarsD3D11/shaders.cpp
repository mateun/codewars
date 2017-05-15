#include "stdafx.h"
#include "shaders.h"

void CreateVertexShader(const ID3D11Device* device, ID3DBlob* vsBlob, ID3D11VertexShader** vs) {
	HRESULT res = const_cast<ID3D11Device*>(device)->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, vs);
	if (FAILED(res)) {
		OutputDebugString(L"failed to create vertex shader\n");
		exit(1);
	}
}

void CreatePixelShader(const ID3D11Device* device, ID3DBlob* psBlob, ID3D11PixelShader** ps) {
	HRESULT res = const_cast<ID3D11Device*>(device)->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, ps);
	if (FAILED(res)) {
		OutputDebugString(L"failed to create vertex shader\n");
		exit(1);
	}
}
