#include "stdafx.h"
#include "spacefight.h"
#include "model_import.h"
#include "textures.h"
#include "shaders.h"
#include <d3dcompiler.h>
#include "model.h"

Game* GetGame() {
	return new Spacefight();
}

std::string GetIntroImageName() {
	return "textures/games/spacefight/spacefight_intro.png";
}

void Spacefight::DoFrame(Renderer& renderer) {
	
	_menuShipRot = 0.00002f;

	XMFLOAT3 zAxis = XMFLOAT3(1, 0, 1);
	XMFLOAT3 yAxis = XMFLOAT3(0, 1, 0);
	XMMATRIX rotMatZ = DirectX::XMMatrixRotationAxis(XMLoadFloat3(&zAxis), _menuShipRot*2.0f);
	XMMATRIX rotMatY = DirectX::XMMatrixRotationAxis(XMLoadFloat3(&yAxis), _menuShipRot);
	//_modelMat = XMMatrixMultiply(_modelMat, rotMatZ);
	_modelMat = XMMatrixMultiply(_modelMat, rotMatY);


	renderer.clearBackbuffer(clearColors);
	renderer.setViewport(0, 0, 800, 600);
	renderer.renderMesh(_shipModel->positions, _shipModel->uvs, _shipModel->indices, _modelMat, _viewMat, _projMat, _vs, _ps, _inputLayout, _shipTexture);
	renderer.presentBackBuffer();
}

Spacefight::Spacefight() : clearColors{ 0.01f, 0.02f, 0.02f, 1.0f } {

}

Spacefight::~Spacefight() {
	ShutDown();
}

void Spacefight::Init(Renderer& renderer) {
	
	// load ship model from filesystem
	std::vector<XMFLOAT3> imp_pos;
	std::vector<XMFLOAT2> imp_uvs;
	std::vector<UINT> imp_indices;
	_shipModel = new Model();
	importModel("models/corvette1.obj", _shipModel);
	loadTextureFromFile("textures/SF_Corvette-F3_diffuse.jpg", &_shipTexture, &renderer);

	XMFLOAT3 eyePos = XMFLOAT3(0, 0, -55);
	XMFLOAT3 eyeDir = XMFLOAT3(0, 0, 1);
	XMFLOAT3 upDir = XMFLOAT3(0, 1, 0);
	_modelMat = DirectX::XMMatrixScaling(1.5, 1.5, 1.5);
	_viewMat = DirectX::XMMatrixLookToLH(XMLoadFloat3(&eyePos), XMLoadFloat3(&eyeDir), XMLoadFloat3(&upDir));
	_projMat = DirectX::XMMatrixPerspectiveFovLH(0.45, 4.0f / 3.0f, 0.1, 100);
	_modelMat = XMMatrixTranspose(_modelMat);
	_viewMat = XMMatrixTranspose(_viewMat);
	_projMat = XMMatrixTranspose(_projMat);


	/// SHADER SETUP
	ID3DBlob* vs = nullptr;
	ID3DBlob* errBlob = nullptr;
	HRESULT res = D3DCompileFromFile(L"shaders/basic.hlsl", NULL, NULL, "VShader", "vs_5_0", 0, 0, &vs, &errBlob);
	if (FAILED(res)) {
		OutputDebugString(L"shader load failed\n");
		if (errBlob)
		{
			OutputDebugStringA((char*)errBlob->GetBufferPointer());
			errBlob->Release();
		}

		if (vs)
			vs->Release();

		exit(1);
	}
	ID3DBlob* ps = nullptr;
	res = D3DCompileFromFile(L"shaders/basic.hlsl", NULL, NULL, "PShader", "ps_5_0", 0, 0, &ps, &errBlob);
	if (FAILED(res)) {
		OutputDebugString(L"shader load failed\n");
		if (errBlob)
		{
			OutputDebugStringA((char*)errBlob->GetBufferPointer());
			errBlob->Release();
		}

		if (ps)
			ps->Release();

		exit(1);
	}
	
	CreateVertexShader(renderer.getDevice(), vs, &_vs);
	ID3D11PixelShader* pShader;
	CreatePixelShader(renderer.getDevice(), ps, &_ps);
	
	/// END SHADER SETUP

	/// INPUT LAYOUT SETUP
	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// same slot, but 12 bytes after the pos
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// other slot (buffer), starting at 0

	};
	res = const_cast<ID3D11Device*>(renderer.getDevice())->CreateInputLayout(ied, 2, vs->GetBufferPointer(), vs->GetBufferSize(), &_inputLayout);
	if (FAILED(res)) {
		OutputDebugString(L"Failed to create input layout\n");
		exit(1);
	}

	ps->Release();
	vs->Release();
	/// END INPUT LAYOUT SETUP

	
}

void Spacefight::ShutDown() {
	_vs->Release(); _vs = nullptr;
	_ps->Release(); _ps = nullptr;
	_shipTexture->Release(); _shipTexture = nullptr;
	_inputLayout->Release(); _inputLayout = nullptr;
	delete(_shipModel); _shipModel = nullptr;
}