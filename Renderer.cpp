#include "Renderer.h"

Renderer* g_pRenderer = new Renderer();

bool Renderer::Initalize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11Buffer* pVertexBuffer)
{
	if (!pDevice || !pDeviceContext || !pVertexBuffer)
		return false;
	
	m_pDevice = pDevice;
	m_pDeviceContext = pDeviceContext;
	m_pVertexBuffer = pVertexBuffer;
	m_pDeviceContext->IAGetInputLayout(&this->m_pInputLayout);

	return true;
}

bool Renderer::Initalize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext) // this needs more shit
{
	if (!pDevice || !pDeviceContext)
		return false;

	m_pDevice = pDevice;
	m_pDeviceContext = pDeviceContext;

	HRESULT hr;
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = 4 * sizeof(Vertex);
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;


	D3D11_INPUT_ELEMENT_DESC layout[] =
	{ 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	
	ID3DBlob* pBlob = NULL;

	if (FAILED(hr = GeneratePixelShader(g_pDevice, &m_pPixelShader, &pBlob, 0.2f, 0.7f, 0.3f)))
		return hr;

	//if (FAILED(hr = GeneratePixelShader(g_pDevice, &m_pVertexShader, &pBlob, 0.2f, 0.7f, 0.3f)))
	//	return hr;

	if (FAILED(hr = m_pDevice->CreateInputLayout(layout, _ARRAYSIZE(layout), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &this->m_pInputLayout)))
		return hr;

	if (FAILED(hr = m_pDevice->CreateBuffer(&desc, NULL, &this->m_pVertexBuffer)))
		return hr;

	return true;
}

HRESULT Renderer::GeneratePixelShader(ID3D11Device* pD3DDevice, ID3D11PixelShader** pShader, ID3DBlob** ppBlob, float r, float g, float b)
{
	char szCast[] =
		"struct VS_OUT"
		"{"
		" float4 Position : POSITION;"
		" float4 Color : COLOR;"
		"};"

		"float4 main( VS_OUT input ) : SV_Target"
		"{"
		" float4 fake;"
		" fake.a = 1.0f;"
		" fake.r = %f;"
		" fake.g = %f;"
		" fake.b = %f;"
		" return fake;"
		"}";

	ID3DBlob* pD3DErrorMsgBlob;
	char szPixelShader[1024];
	HRESULT hr;

	sprintf_s(szPixelShader, szCast, r, g, b);

	if (FAILED(hr = D3DCompile(szPixelShader, sizeof(szPixelShader), "shader", NULL, NULL, "main", "ps_4_0", NULL, NULL, ppBlob, &pD3DErrorMsgBlob)))
		return hr;

	if (FAILED(hr = pD3DDevice->CreatePixelShader((*ppBlob)->GetBufferPointer(), (*ppBlob)->GetBufferSize(), NULL, pShader)))
		return hr;

	return S_OK;
}

HRESULT Renderer::SaveState()
{
	if (!m_pDeviceContext)
		return E_FAIL;

	m_pDeviceContext->IAGetInputLayout(&this->m_poInputLayout);
	m_pDeviceContext->IASetInputLayout(this->m_pInputLayout);

	m_pDeviceContext->VSGetShader(&this->m_poVertexShader, NULL, 0);
	m_pDeviceContext->VSSetShader(this->m_pVertexShader, NULL, 0);


	return S_OK;
}

HRESULT Renderer::RestoreState()
{
	if (!m_pDeviceContext)
		return E_FAIL;

	m_pDeviceContext->IASetInputLayout(this->m_poInputLayout);

	return S_OK;
}

void Renderer::DrawRectOutline(int x, int y, int w, int h, D3DCOLOR color)
{
	D3D11_MAPPED_SUBRESOURCE mapped;

	if (FAILED(this->m_pDeviceContext->Map(this->m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		return;

	Vertex* pVertices = (Vertex*)mapped.pData;

	BuildVertex(XMFLOAT4((FLOAT)x, (FLOAT)(y + h), 0, 1), color, pVertices, 0);			//near up left
	BuildVertex(XMFLOAT4((FLOAT)x, (FLOAT)y, 0, 1), color, pVertices, 1);				//near left
	BuildVertex(XMFLOAT4((FLOAT)(x + w), (FLOAT)(y + h), 0, 1), color, pVertices, 2);   //far up right
	BuildVertex(XMFLOAT4((FLOAT)(x + w), (FLOAT)y, 0, 1), color, pVertices, 3);			//far right

	this->m_pDeviceContext->Unmap(this->m_pVertexBuffer, 0);

	UINT stride = sizeof(Vertex_t);
	UINT offset = 0;

	this->m_pDeviceContext->IASetVertexBuffers(0, 1, &this->m_pVertexBuffer, &stride, &offset);
	this->m_pDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	this->m_pDeviceContext->IASetInputLayout(this->m_pInputLayout);
	this->m_pDeviceContext->Draw(4, 0);
}