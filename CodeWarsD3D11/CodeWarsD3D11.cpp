// CodeWarsD3D11.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "CodeWarsD3D11.h"
#include "renderer.h"
#include <d3dcompiler.h>
#include "shaders.h"
#include <FreeImage.h>
#include "textures.h"
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
Renderer* renderer;

bool importModel(const std::string& file, std::vector<XMFLOAT3>& positions, std::vector<XMFLOAT2>& uvs, std::vector<UINT>& indices) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate );
	if (!scene) {
		OutputDebugString(L"model import failed!\n");
		exit(1);
	}

	unsigned int numMeshes = scene->mRootNode->mChildren[0]->mNumMeshes;
	for (int i = 0; i < numMeshes; ++i) {
		UINT idx = scene->mRootNode->mChildren[0]->mMeshes[i];
		aiMesh* mesh = scene->mMeshes[i];
		UINT numUVChannels = mesh->GetNumUVChannels();
		bool hasTextureCoords = mesh->HasTextureCoords(0);
		for (int v = 0; v < mesh->mNumVertices; ++v) {
			aiVector3D vertex = mesh->mVertices[v];
			aiVector3D texcoord = mesh->mTextureCoords[0][v];

			
								
			positions.push_back({ vertex.x, vertex.y, vertex.z });
			uvs.push_back({ texcoord.x, texcoord.y });

					
		}

		for (int f = 0; f < mesh->mNumFaces; ++f) {
			aiFace face = mesh->mFaces[f];
			indices.push_back({ face.mIndices[0] });
			indices.push_back({ face.mIndices[1] });
			indices.push_back({ face.mIndices[2] });
		}
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CODEWARSD3D11, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CODEWARSD3D11));

    MSG msg;
	float clearColors[] = { 0.1, 0.1, 0.1, 1.0 };
	std::vector<XMFLOAT3> mesh;
	mesh.push_back({ -0.5, 0.5, 0 });
	mesh.push_back({ 0.5, -0.5, 0 });
	mesh.push_back({ -0.5, -0.5, 0 });
	mesh.push_back({ 0.5, 0.5, 0 });
	std::vector<XMFLOAT2> uvs;
	uvs.push_back({ 0, 0 });
	uvs.push_back({ 1, 1 }); 
	uvs.push_back({ 0, 1 });
	uvs.push_back({ 1, 0 });

	XMMATRIX modelMat = DirectX::XMMatrixScaling(2.5, 2.5, 2.5);
	
	XMFLOAT3 eyePos = XMFLOAT3(0, 0, -35);
	XMFLOAT3 eyeDir = XMFLOAT3(0, 0, 1);
	XMFLOAT3 upDir = XMFLOAT3(0, 1, 0);
	XMMATRIX viewMat = DirectX::XMMatrixLookToLH(XMLoadFloat3(&eyePos), XMLoadFloat3(&eyeDir), XMLoadFloat3(&upDir));
	XMMATRIX projMat = DirectX::XMMatrixPerspectiveFovLH(0.45, 4.0f / 3.0f, 0.1, 100);
	modelMat = XMMatrixTranspose(modelMat);
	viewMat = XMMatrixTranspose(viewMat);
	projMat = XMMatrixTranspose(projMat);

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
	ID3D11VertexShader* vshader;
	CreateVertexShader(renderer->getDevice(), vs, &vshader);
	ID3D11PixelShader* pShader;
	CreatePixelShader(renderer->getDevice(), ps, &pShader);

	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// same slot, but 12 bytes after the pos
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// other slot (buffer), starting at 0

	};
	ID3D11InputLayout* inputLayout;
	const_cast<ID3D11Device*>(renderer->getDevice())->CreateInputLayout(ied, 2, vs->GetBufferPointer(), vs->GetBufferSize(), &inputLayout);

	// RenderSplash
	ID3D11Texture2D* tex;
	loadTextureFromFile("textures/Wood512x512.png", &tex, renderer);
	std::vector<UINT> indices; 
	//0, 1, 2, 0, 3, 1
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(1);

	renderer->clearBackbuffer(clearColors);
	renderer->setViewport(0, 0, 800, 600);
	renderer->renderMesh(mesh, uvs, indices, modelMat, viewMat, projMat, vshader, pShader, inputLayout, tex);
	renderer->presentBackBuffer();

	// model loaded from filesystem
	std::vector<XMFLOAT3> imp_pos;
	std::vector<XMFLOAT2> imp_uvs;
	std::vector<UINT> imp_indices;
	importModel("models/plane3.obj", imp_pos, imp_uvs, imp_indices);
	
	
	Sleep(2000);

	// render loading screen
	
	loadTextureFromFile("textures/cube_diff2.png", &tex, renderer);

	renderer->clearBackbuffer(clearColors);
	renderer->setViewport(0, 0, 800, 600);
	renderer->renderMesh(imp_pos, imp_uvs, imp_indices, modelMat, viewMat, projMat, vshader, pShader, inputLayout, tex);
	renderer->presentBackBuffer();

	float rotZ = 0.0f;
	
	// Main message loop when done with the static resources
	while (true) { 
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) break;

		rotZ += 0.0002f;

		modelMat = DirectX::XMMatrixScaling(2.5, 2.5, 2.5);
		XMFLOAT3 zAxis = XMFLOAT3(1, 0, 1);
		XMFLOAT3 yAxis = XMFLOAT3(0, 1, 0);
		XMMATRIX rotMatZ = DirectX::XMMatrixRotationAxis(XMLoadFloat3(&zAxis), rotZ);
		XMMATRIX rotMatY = DirectX::XMMatrixRotationAxis(XMLoadFloat3(&yAxis), rotZ);
		modelMat = XMMatrixMultiply(modelMat, rotMatZ);
		modelMat = XMMatrixMultiply(modelMat, rotMatY);

		renderer->clearBackbuffer(clearColors);
		renderer->setViewport(0, 0, 800, 600);
		renderer->renderMesh(imp_pos, imp_uvs, imp_indices, modelMat, viewMat, projMat, vshader, pShader, inputLayout, nullptr);
		renderer->presentBackBuffer();
	}

	tex->Release();
	inputLayout->Release();
	pShader->Release();
	vshader->Release();

	ps->Release();
	vs->Release();
	
	if (errBlob) errBlob->Release();
	

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CODEWARSD3D11));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CODEWARSD3D11);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   renderer = new Renderer(800, 600, hWnd);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
