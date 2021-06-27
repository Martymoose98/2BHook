#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include <D3Dcompiler.h>
#include "Globals.h"
#include "Color.h"
#include "Log.h"

using DirectX::XMFLOAT4;
using DirectX::XMMATRIX;

const char g_szShader[] = 
R"(cbuffer m_pViewMatrix : register(b0)
{
	matrix m_ViewMatrix;
};
 
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};
 
struct VS_INPUT
{
	float4 pos : POSITION;
	float4 col : COLOR;
};
 
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
 
	output.pos = mul(m_ViewMatrix, float4(input.pos.xy, 0.f, 1.f));
	output.col = input.col;
 
	return output;
}
 
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return input.col;
})";

////other shitty shader
//char szCast[] =
//"struct VS_OUT"
//"{"
//" float4 Position : POSITION;"
//" float4 Color : COLOR;"
//"};"
//
//"float4 main( VS_OUT input ) : SV_Target"
//"{"
//" float4 fake;"
//" fake.a = 1.0f;"
//" fake.r = %f;"
//" fake.g = %f;"
//" fake.b = %f;"
//" return fake;"
//"}";

typedef struct Vertex
{
	XMFLOAT4 xyzrhw;
	XMFLOAT4 color;
	//	D3DCOLOR color;
} Vertex;

typedef struct RenderBatch
{
	RenderBatch(UINT count, D3D11_PRIMITIVE_TOPOLOGY topology)
		: count(count), topology(topology)
	{}

	UINT count;
	D3D11_PRIMITIVE_TOPOLOGY topology;
} RenderBatch;

class RenderList
{
	friend class Renderer;
public:
	
	RenderList()
		: m_vertices()
	{

	}

	RenderList(unsigned int uReserveVertices)
		: m_uMaxVertices(uReserveVertices)
	{
		m_vertices.reserve(uReserveVertices);
	}

	void AddVertex(const Vertex& vertex, D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		assert(topology != D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
			&& "addVertex >Use addVertices to draw line/triangle strips!");
		assert(topology != D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ
			&& "addVertex >Use addVertices to draw line/triangle strips!");
		assert(topology != D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
			&& "addVertex >Use addVertices to draw line/triangle strips!");
		assert(topology != D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ
			&& "addVertex >Use addVertices to draw line/triangle strips!");


		if (m_vertices.size() >= m_uMaxVertices)
		{
			LOG("Vertex buffer exhausted! Increase the size of the vertex buffer or add a custom implementation.");
			return;
		}

		if (m_batches.empty() || m_batches.back().topology != topology)
			m_batches.emplace_back(0, topology);

		m_batches.back().count++;
		m_vertices.emplace_back(vertex);
	}

	void AddVertex(const Vertex* pVertices, UINT uVerticeCount, D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		assert(topology != D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
			&& "AddVertex >Use AddVertices to draw line/triangle strips!");
		assert(topology != D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ
			&& "AddVertex >Use AddVertices to draw line/triangle strips!");
		assert(topology != D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
			&& "AddVertex >Use AddVertices to draw line/triangle strips!");
		assert(topology != D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ
			&& "AddVertex >Use AddVertices to draw line/triangle strips!");

		if (!pVertices)
			return;

		if (m_vertices.size() + uVerticeCount >= m_uMaxVertices)
		{
			LOG("Vertex buffer exhausted! Increase the size of the vertex buffer or add a custom implementation.");
			return;
		}

		if (m_batches.empty() || m_batches.back().topology != topology)
			m_batches.emplace_back(0, topology);

		m_batches.back().count += uVerticeCount;
		m_vertices.resize(m_vertices.size() + uVerticeCount);
		memcpy(&m_vertices[m_vertices.size() - uVerticeCount], pVertices, uVerticeCount * sizeof(Vertex));

		switch (topology)
		{
		case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
		case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
		case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
			Vertex seperator = {};
			AddVertex(seperator, D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
			return;
		}
	}

	template<unsigned int N>
	void AddVertices(Vertex(&vertices)[N], D3D11_PRIMITIVE_TOPOLOGY topology)
	{

		if (m_vertices.size() + N >= m_uMaxVertices)
		{
			LOG("Vertex buffer exhausted! Increase the size of the vertex buffer or add a custom implementation.");
			return;
		}

		if (m_batches.empty() || m_batches.back().topology != topology)
			m_batches.emplace_back(0, topology);

		m_batches.back().count += N;
		m_vertices.resize(m_vertices.size() + N);
		memcpy(&m_vertices[m_vertices.size() - N], &vertices[0], N * sizeof(Vertex));

		switch (topology)
		{
		case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
		case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
		case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
			Vertex seperator = {};
			AddVertex(seperator, D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
			return;
		}
	}

	void clear()
	{
		m_vertices.clear();
		m_batches.clear();
	}

protected:
	unsigned int				m_uMaxVertices;
	std::vector<Vertex>			m_vertices;
	std::vector<RenderBatch>	m_batches;
};

class Renderer
{
public:
	 
	Renderer() : m_RenderList(16384 * 8 * 4 * 3) {}
	~Renderer();

	HRESULT Initalize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11Buffer* pVertexBuffer);
	HRESULT Initalize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	void Release();

	HRESULT Begin(void);
	HRESULT End(void);

	HRESULT SaveState(void);
	HRESULT RestoreState(void);

	void Draw(const RenderList& list);
	void Draw(void);

	template<size_t N>
	HRESULT GeneratePixelShader(ID3D11PixelShader** pShader, ID3DBlob** ppBlob, const char (&szShader)[N]);
	HRESULT GeneratePixelShader(ID3D11PixelShader** pShader, ID3DBlob** ppBlob, const char* szShader, size_t cchSource);

	template<size_t N>
	HRESULT GenerateVertexShader(ID3D11VertexShader** pShader, ID3DBlob** ppBlob, const char (&szShader)[N]);
	HRESULT GenerateVertexShader(ID3D11VertexShader** pShader, ID3DBlob** ppBlob, const char* szShader, size_t cchSource);

	void DrawPixel(int x, int y, const Color& color);
	void DrawPixel(const RenderList& list, int x, int y, D3DCOLOR color);
	
	void DrawLine(int x0, int y0, int x1, int y1, const Color& color);
	void DrawLine(const RenderList& list, int x0, int y0, int x1, int y1, D3DCOLOR color);
	
	void DrawText(int x, int y, const Color& color, const char* szText, ...);

	void DrawFilledRect(int x, int y, int width, int height, const Color& color);
	void DrawFilledRect(const RenderList& list, const Vector4& rect, const Color& color);
	void DrawRectOutline(int x, int y, int w, int h, D3DCOLOR color);
	void DrawRectBordered(int x, int y, int width, int height, int thickness, const Color& color);
	void DrawRectCorners(int x, int y, int w, int h, int thickness, const Color& color);

	void DrawHealthBar(int x, int y, int height, int health, int max_health);

private:
	__forceinline void BuildVertex(XMFLOAT4 xyzrhw, D3DCOLOR color, Vertex* vertices, int index)
	{
		vertices[index].xyzrhw = xyzrhw;
		vertices[index].color = XMFLOAT4(((color >> 16) & 0xFF) / 255.f, ((color >> 8) & 0xFF) / 255.f, (color & 0xFF) / 255.f, ((color >> 24) & 0xFF) / 255.f);
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
	ID3D11Buffer* m_pViewMatrix;
	XMMATRIX m_ViewMatrix;
	RenderList m_RenderList;
};
extern Renderer* g_pRenderer;