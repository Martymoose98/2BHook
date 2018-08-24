#include "Renderer.h"

Renderer* g_pRenderer = new Renderer();

HRESULT Renderer::Initalize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11Buffer* pVertexBuffer)
{
	if (!pDevice || !pDeviceContext || !pVertexBuffer)
		return S_FALSE;
	
	m_pDevice = pDevice;
	m_pDeviceContext = pDeviceContext;
	m_pVertexBuffer = pVertexBuffer;
	m_pDeviceContext->IAGetInputLayout(&this->m_pInputLayout);

	return D3D_OK;
}

HRESULT Renderer::Initalize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext) // this needs more shit
{
	HRESULT hr;
	D3D11_BUFFER_DESC Desc;
	D3D11_MAPPED_SUBRESOURCE Mapped;

	if (!pDevice || !pDeviceContext)
		return false;

	m_pDevice = pDevice;
	m_pDeviceContext = pDeviceContext;

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{ 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	
	ID3DBlob* pVSBlob = NULL;
	ID3DBlob* pPSBlob = NULL;

	if (FAILED(hr = GeneratePixelShader(&m_pPixelShader, &pPSBlob, g_szShader)))
		return hr;

	if (FAILED(hr = GenerateVertexShader(&m_pVertexShader, &pVSBlob, g_szShader)))
		return hr;

	if (FAILED(hr = m_pDevice->CreateInputLayout(layout, _ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &this->m_pInputLayout)))
		return hr;

	if (pVSBlob)
	{
		pVSBlob->Release();
		pVSBlob = NULL;
	}

	if (pPSBlob)
	{
		pPSBlob->Release();
		pPSBlob = NULL;
	}

	ZeroMemory(&Desc, sizeof(D3D11_BUFFER_DESC));

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.ByteWidth = m_RenderList.m_uMaxVertices * sizeof(Vertex);
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;

	if (FAILED(hr = m_pDevice->CreateBuffer(&Desc, NULL, &m_pVertexBuffer)))
		return hr;

	ZeroMemory(&Desc, sizeof(D3D11_BUFFER_DESC));

	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.ByteWidth = sizeof(XMMATRIX);
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;

	if (FAILED(hr = m_pDevice->CreateBuffer(&Desc, NULL, &m_pViewMatrix)))
		return hr;

	D3DXMatrixOrthoOffCenterLH((D3DXMATRIX*)&m_ViewMatrix, 0.0f, (FLOAT)g_pGraphicDevice->iScreenWidth, (FLOAT)g_pGraphicDevice->iScreenHeight, 0.0f, -100.0f, 100.0f);
	//XMMatrixOrthographicOffCenterLH(0.0f, g_pGraphicDevice->iScreenWidth, g_pGraphicDevice->iScreenHeight, 0.0f, -100.0f, 100.0f);

	if (FAILED(hr = m_pDeviceContext->Map(m_pViewMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
		return hr;

	memcpy(Mapped.pData, &m_ViewMatrix, sizeof(XMMATRIX));

	m_pDeviceContext->Unmap(m_pViewMatrix, 0);

	return D3D_OK;
}

template<size_t N>
HRESULT Renderer::GeneratePixelShader(ID3D11PixelShader** pShader, ID3DBlob** ppBlob, const char (&szShader)[N])
{
	ID3DBlob* pD3DErrorMsgBlob;
	HRESULT hr;

	if (!m_pDevice)
		return D3DERR_DEVICELOST;

	if (FAILED(hr = D3DCompile(szShader, N, NULL, NULL, NULL, "PS", "ps_4_0", 0, 0, ppBlob, &pD3DErrorMsgBlob)))
		return hr;

	if (FAILED(hr = m_pDevice->CreatePixelShader((*ppBlob)->GetBufferPointer(), (*ppBlob)->GetBufferSize(), NULL, pShader)))
		return hr;

	return S_OK;
}

HRESULT Renderer::GeneratePixelShader(ID3D11PixelShader** pShader, ID3DBlob** ppBlob, const char* szShader, size_t cchSource)
{
	ID3DBlob* pD3DErrorMsgBlob;
	HRESULT hr;

	if (!m_pDevice)
		return D3DERR_DEVICELOST;

	if (FAILED(hr = D3DCompile(szShader, cchSource, NULL, NULL, NULL, "PS", "ps_4_0", 0, 0, ppBlob, &pD3DErrorMsgBlob)))
		return hr;

	if (FAILED(hr = m_pDevice->CreatePixelShader((*ppBlob)->GetBufferPointer(), (*ppBlob)->GetBufferSize(), NULL, pShader)))
		return hr;

	return S_OK;
}

template<size_t N>
HRESULT Renderer::GenerateVertexShader(ID3D11VertexShader** pShader, ID3DBlob** ppBlob, const char (&szShader)[N])
{
	ID3DBlob* pD3DErrorMsgBlob;
	HRESULT hr;

	if (!m_pDevice)
		return D3DERR_DEVICELOST;

	if (FAILED(hr = D3DCompile(szShader, N, NULL, NULL, NULL, "VS", "vs_4_0", 0, 0, ppBlob, &pD3DErrorMsgBlob)))
		return hr;

	if (FAILED(hr = m_pDevice->CreateVertexShader((*ppBlob)->GetBufferPointer(), (*ppBlob)->GetBufferSize(), NULL, pShader)))
		return hr;

	return S_OK;
}

HRESULT Renderer::GenerateVertexShader(ID3D11VertexShader** pShader, ID3DBlob** ppBlob, const char* szShader, size_t cchSource)
{
	ID3DBlob* pD3DErrorMsgBlob;
	HRESULT hr;

	if (!m_pDevice)
		return D3DERR_DEVICELOST;

	if (FAILED(hr = D3DCompile(szShader, cchSource, NULL, NULL, NULL, "VS", "vs_4_0", 0, 0, ppBlob, &pD3DErrorMsgBlob)))
		return hr;

	if (FAILED(hr = m_pDevice->CreateVertexShader((*ppBlob)->GetBufferPointer(), (*ppBlob)->GetBufferSize(), NULL, pShader)))
		return hr;

	return S_OK;
}

HRESULT Renderer::SaveState()
{
	if (!m_pDeviceContext)
		return E_FAIL;

	m_pDeviceContext->IAGetInputLayout(&m_poInputLayout);
	m_pDeviceContext->VSGetShader(&m_poVertexShader, NULL, 0);
	m_pDeviceContext->PSGetShader(&m_poPixelShader, NULL, 0);

	return S_OK;
}

HRESULT Renderer::RestoreState()
{
	if (!m_pDeviceContext || !m_poInputLayout)
		return E_FAIL;

	m_pDeviceContext->IASetInputLayout(m_poInputLayout);
	m_pDeviceContext->VSSetShader(m_poVertexShader, NULL, 0);
	m_pDeviceContext->PSSetShader(m_poPixelShader, NULL, 0);


	return S_OK;
}

HRESULT Renderer::Begin()
{
	HRESULT hr = SaveState();

	m_pDeviceContext->PSSetShader(m_pPixelShader, NULL, 0);
	m_pDeviceContext->VSSetShader(m_pVertexShader, NULL, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pViewMatrix);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	return hr;
}

HRESULT Renderer::End()
{
	m_RenderList.clear();
	return RestoreState();
}

void Renderer::Draw()
{
	return Draw(m_RenderList);
}

void Renderer::Draw(const RenderList& list)
{
	D3D11_MAPPED_SUBRESOURCE mapped;

	if (list.m_vertices.empty())
		return;

	if (FAILED(this->m_pDeviceContext->Map(this->m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		return;

	memcpy(mapped.pData, list.m_vertices.data(), sizeof(Vertex) * list.m_vertices.size());

	this->m_pDeviceContext->Unmap(this->m_pVertexBuffer, 0);

	UINT pos = 0;

	for (const auto& batch : list.m_batches)
	{
		this->m_pDeviceContext->IASetPrimitiveTopology(batch.topology);
		this->m_pDeviceContext->Draw(batch.count, pos);

		pos += batch.count;
	}
}

void Renderer::DrawPixel(int x, int y, const Color& color)
{
	DrawFilledRect(x, y, 1, 1, color);
}

void Renderer::DrawLine(int x0, int y0, int x1, int y1, const Color& color)
{
	float rgba[4];

	color.GetFloatColor(rgba);

	Vertex v[]
	{
		{ XMFLOAT4(x0, y0, 0.0f, 1.0f), XMFLOAT4(rgba) },
		{ XMFLOAT4(x1, y1, 0.0f, 1.0f), XMFLOAT4(rgba) }
	};

	m_RenderList.AddVertices(v, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}


void Renderer::DrawFilledRect(int x, int y, int width, int height, const Color& color)
{
	float rgba[4];

	color.GetFloatColor(rgba);

	Vertex v[]
	{
		{ XMFLOAT4(x,			y,			0.f,	1.f), XMFLOAT4(rgba) },
		{ XMFLOAT4(x + width,	y,			0.f,	1.f), XMFLOAT4(rgba) },
		{ XMFLOAT4(x,			y + height, 0.f,	1.f), XMFLOAT4(rgba) },
		{ XMFLOAT4(x + width,	y,			0.f,	1.f), XMFLOAT4(rgba) },
		{ XMFLOAT4(x + width,	y + height,	0.f,	1.f), XMFLOAT4(rgba) },
		{ XMFLOAT4(x,			y + height,	0.f,	1.f), XMFLOAT4(rgba) }
	};

	m_RenderList.AddVertices(v, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::DrawFilledRect(const RenderList& list, const Vector4& rect, const Color& color)
{
	//Vertex v[]
	//{
	//	{ rect.x,			rect.y,				0.f, color },
	//	{ rect.x + rect.z,	rect.y,				0.f, color },
	//	{ rect.x,			rect.y + rect.w,	0.f, color },

	//	{ rect.x + rect.z,	rect.y,				0.f, color },
	//	{ rect.x + rect.z,	rect.y + rect.w,	0.f, color },
	//	{ rect.x,			rect.y + rect.w,	0.f, color }
	//};

	//m_RenderList.AddVertices(v, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::DrawRectOutline(int x, int y, int w, int h, D3DCOLOR color)
{
	Vertex vertices[4];

	BuildVertex(XMFLOAT4((FLOAT)x, (FLOAT)(y + h), 0, 1), color, vertices, 0);			//near up left
	BuildVertex(XMFLOAT4((FLOAT)x, (FLOAT)y, 0, 1), color, vertices, 1);				//near left
	BuildVertex(XMFLOAT4((FLOAT)(x + w), (FLOAT)(y + h), 0, 1), color, vertices, 2);    //far up right
	BuildVertex(XMFLOAT4((FLOAT)(x + w), (FLOAT)y, 0, 1), color, vertices, 3);			//far right

	m_RenderList.AddVertices(vertices, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//D3D11_MAPPED_SUBRESOURCE mapped;

	//if (FAILED(this->m_pDeviceContext->Map(this->m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	//	return;

	//Vertex* pVertices = (Vertex*)mapped.pData;

	//BuildVertex(XMFLOAT4((FLOAT)x, (FLOAT)(y + h), 0, 1), color, pVertices, 0);			//near up left
	//BuildVertex(XMFLOAT4((FLOAT)x, (FLOAT)y, 0, 1), color, pVertices, 1);				//near left
	//BuildVertex(XMFLOAT4((FLOAT)(x + w), (FLOAT)(y + h), 0, 1), color, pVertices, 2);   //far up right
	//BuildVertex(XMFLOAT4((FLOAT)(x + w), (FLOAT)y, 0, 1), color, pVertices, 3);			//far right

	//this->m_pDeviceContext->Unmap(this->m_pVertexBuffer, 0);

	//UINT stride = sizeof(Vertex);
	//UINT offset = 0;

	//this->m_pDeviceContext->IASetVertexBuffers(0, 1, &this->m_pVertexBuffer, &stride, &offset);
	//this->m_pDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//this->m_pDeviceContext->IASetInputLayout(this->m_pInputLayout);
	//this->m_pDeviceContext->Draw(4, 0);
}

void Renderer::DrawRectCorners(int x, int y, int w, int h, int thickness, const Color& color)
{
	int v_leg = h / 4;
	int h_leg = v_leg / 2;

	//top near
	DrawFilledRect(x, y, h_leg, thickness, color); //h
	DrawFilledRect(x, y, thickness, v_leg, color); //v

	//top far
	DrawFilledRect((x + w) - h_leg, y, h_leg, thickness, color); //h
	DrawFilledRect(x + w, y, thickness, v_leg, color); //v

	//btm near
	DrawFilledRect(x, y + h, h_leg, thickness, color); // h
	DrawFilledRect(x, (y + h) - v_leg, thickness, v_leg, color); //v

	//btm far
	DrawFilledRect((x + w) - (h_leg - thickness), y + h, h_leg, thickness, color); //h
	DrawFilledRect(x + w, (y + h) - v_leg, thickness, v_leg, color); //v
}