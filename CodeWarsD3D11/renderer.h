#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <vector>

using namespace DirectX;

struct MY_VERTEX {
	FLOAT X, Y, Z;
	FLOAT u, v;
};


class Renderer {

public:
	Renderer(int w, int h, HWND hWnd);
	~Renderer();
	void render();
	void clearBackbuffer(float *colors);
	void presentBackBuffer();
	void Renderer::setViewport(int x, int y, int w, int h);
	void renderMesh(const std::vector<XMFLOAT3> &meshVertices, const XMMATRIX &modelMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projMatrix, 
							ID3D11VertexShader* vs, ID3D11PixelShader* ps, ID3D11InputLayout* inputLayout,
							ID3D11Texture2D* tex);
	const ID3D11Device* getDevice() {
		return _device;
	}
	ID3D11DeviceContext* getContext() {
		return _ctx;
	}

private:
	void init(int w, int h, HWND hWnd);
	ID3D11Device *_device;
	ID3D11DeviceContext *_ctx;
	IDXGISwapChain *_swapChain;
	ID3D11Debug* _debugger;
	ID3D11Texture2D *_backBuffer;
	ID3D11RenderTargetView* _rtv;
	ID3D11Texture2D* _depthStencilBuffer;
	ID3D11DepthStencilView *_depthStencilView;
};
