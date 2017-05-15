#pragma once
#include "game.h"
#include "model.h"

class Spacefight : public Game {

public:
	Spacefight();
	~Spacefight();
	virtual void Init(Renderer& renderer); 
	virtual void DoFrame(Renderer& renderer);
	virtual void ShutDown();


private:
	float clearColors[4];
	XMMATRIX _modelMat;
	XMMATRIX _viewMat;
	XMMATRIX _projMat;
	ID3D11VertexShader* _vs;
	ID3D11PixelShader* _ps;
	ID3D11InputLayout* _inputLayout;
	Model* _shipModel;
	ID3D11Texture2D* _shipTexture;
	float _menuShipRot = 0;
};
