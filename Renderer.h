#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <D3Dcompiler.h>
#include "Globals.h"

using DirectX::XMFLOAT4;

typedef struct Vertex_t
{
	XMFLOAT4 xyzrhw;
	D3DCOLOR color;

	enum
	{
		FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE
	};
} Vertex;

class Renderer
{
public:

	bool Initalize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11Buffer* pVertexBuffer);
	bool Initalize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	void Release();

	HRESULT SaveState();
	HRESULT RestoreState();

	HRESULT GeneratePixelShader(ID3D11Device* pD3DDevice, ID3D11PixelShader** pShader, ID3DBlob** ppBlob, float r, float g, float b);

	void DrawRectOutline(int x, int y, int w, int h, D3DCOLOR color);

private:
	__forceinline void BuildVertex(XMFLOAT4 xyzrhw, D3DCOLOR color, Vertex* vertices, int index)
	{
		vertices[index].xyzrhw = xyzrhw;
		vertices[index].color = color;
	}

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11VertexShader* m_poVertexShader;
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_poVertexBuffer;
	ID3D11PixelShader* m_pPixelShader;
	ID3D11PixelShader* m_poPixelShader;
	ID3D11Buffer* m_pPixelBuffer;
	ID3D11Buffer* m_poPixelBuffer;
	ID3D11InputLayout* m_pInputLayout;
	ID3D11InputLayout* m_poInputLayout;
};
extern Renderer* g_pRenderer;