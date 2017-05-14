#include "stdafx.h"
#include "renderer.h"
#include <dxgi.h>

Renderer::Renderer(int w, int h, HWND hWnd) {
	init(w, h, hWnd);
}

Renderer::~Renderer() {
	_ctx->ClearState();
	_depthStencilView->Release();
	_depthStencilBuffer->Release();
	_backBuffer->Release();
	_swapChain->Release();
	_ctx->Release();
	_debugger->Release();
	_device->Release();
}

void Renderer::clearBackbuffer(float *clearColors) {
	ID3D11RenderTargetView* rtvs[1] = { _rtv };
	_ctx->OMSetRenderTargets(1, rtvs, NULL);
	_ctx->ClearRenderTargetView(_rtv, clearColors);

	// clear our depth target as well
	_ctx->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH, 0, 0);
}

void Renderer::presentBackBuffer() {
	_swapChain->Present(0, 0);
}

/**
	Sets the current viewport, expects
	a valid D3D11 context.

*/
void Renderer::setViewport(int x, int y, int w, int h) {
	D3D11_VIEWPORT vp;
	ZeroMemory(&vp, sizeof(vp));
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = 800;
	vp.Height = 600;
	_ctx->RSSetViewports(1, &vp);
}

void Renderer::render() {
	_ctx->Release();
	_device->Release();	
}

void Renderer::renderMesh(const std::vector<XMFLOAT3> &meshVertices, const XMMATRIX &modelMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projMatrix,
	ID3D11VertexShader* vs, ID3D11PixelShader* ps, ID3D11InputLayout* inputLayout, ID3D11Texture2D* tex) {
	ID3D11Buffer* vbuf;
	MY_VERTEX ourVertices[] = 
	{
		{0, 0.5, 0, 0.5, 0},
		{0.5, -0.5, 0, 1, 1},
		{-0.5, -0.5, 0, 0, 1}
	};
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(MY_VERTEX) * 3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	_device->CreateBuffer(&bd, NULL, &vbuf);

	D3D11_MAPPED_SUBRESOURCE ms;
	_ctx->Map(vbuf, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, ourVertices, sizeof(ourVertices));
	_ctx->Unmap(vbuf, NULL);

	_ctx->VSSetShader(vs, 0, 0);
	_ctx->PSSetShader(ps, 0, 0);

	_ctx->IASetInputLayout(inputLayout);
	UINT stride = sizeof(MY_VERTEX);
	UINT offset = 0;
	_ctx->IASetVertexBuffers(0, 1, &vbuf, &stride, &offset);

	// Handle textures
	ID3D11SamplerState* samplerState = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	if (tex) {
		
		_device->CreateShaderResourceView(tex, NULL, &srv);
		_ctx->PSSetShaderResources(0, 1, &srv);
		
		D3D11_SAMPLER_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

		_device->CreateSamplerState(&sd, &samplerState);
		_ctx->PSSetSamplers(0, 1, &samplerState);
	}

	_ctx->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_ctx->Draw(3, 0);

	if (samplerState) samplerState->Release();
	if (srv) srv->Release();
	
	vbuf->Release();
}

void Renderer::init(int w, int h, HWND hWnd) {
	D3D_FEATURE_LEVEL featureLevels =  D3D_FEATURE_LEVEL_11_0;

	HRESULT result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &featureLevels, 1, D3D11_SDK_VERSION, &_device, NULL, &_ctx);
	if (FAILED(result)) {
		OutputDebugString(L"CreateDevice failed\n");
		exit(2);
	}

	UINT ql;
	_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UINT, 4, &ql);

	DXGI_SWAP_CHAIN_DESC scdesc;
	ZeroMemory(&scdesc, sizeof(scdesc));
	scdesc.BufferCount = 1;
	scdesc.BufferDesc.Height = h;
	scdesc.BufferDesc.Width = w;
	scdesc.Windowed = true;

	scdesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	
	scdesc.BufferDesc.RefreshRate.Numerator = 60;
	scdesc.BufferDesc.RefreshRate.Denominator = 1;
	
	scdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	scdesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scdesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	
	scdesc.OutputWindow = hWnd;
	scdesc.SampleDesc.Count = 1;	// 1 sample per pixel
	scdesc.SampleDesc.Quality = 0;
	scdesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scdesc.Flags = 0;
	
	IDXGIDevice * pDXGIDevice = nullptr;
	result = _device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
	IDXGIAdapter * pDXGIAdapter = nullptr;
	result = pDXGIDevice->GetAdapter(&pDXGIAdapter);
	IDXGIFactory * pIDXGIFactory = nullptr;
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void **)&pIDXGIFactory);
	
	result = pIDXGIFactory->CreateSwapChain(_device, &scdesc, &_swapChain);
	if (FAILED(result)) {
		OutputDebugString(L"error creating swapchain\n");
		exit(1);
	}

	pIDXGIFactory->Release();
	pDXGIAdapter->Release();
	pDXGIDevice->Release();


	// Gather the debug interface
	_debugger = 0;
	result = _device->QueryInterface(__uuidof(ID3D11Debug), (void**)&_debugger);
	if (FAILED(result)) {
		OutputDebugString(L"debuger creation failed\n");
		exit(1);
	}

	// Create a backbuffer
	result = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&_backBuffer);
	if (FAILED(result)) {
		OutputDebugString(L"backbuffer creation failed\n");
		exit(1);
	}

	// Bind the backbuffer as our render target
	result = _device->CreateRenderTargetView(_backBuffer, NULL, &_rtv);
	if (FAILED(result)) {
		OutputDebugString(L"rtv creation failed\n");
		exit(1);
	}

	// Create a depth/stencil buffer
	D3D11_TEXTURE2D_DESC td;
	td.Width = w;
	td.Height = h;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_D32_FLOAT;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;
	
	result = _device->CreateTexture2D(&td, 0, &_depthStencilBuffer);
	if (FAILED(result)) {
		OutputDebugString(L"D S buffer creation failed\n");
		exit(1);
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dpd;
	ZeroMemory(&dpd, sizeof(dpd));
	dpd.Flags = 0;
	dpd.Format = DXGI_FORMAT_D32_FLOAT;
	dpd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	result = _device->CreateDepthStencilView(_depthStencilBuffer, &dpd, &_depthStencilView);
	if (FAILED(result)) {
		OutputDebugString(L"D S view creation failed\n");
		exit(1);
	}

}