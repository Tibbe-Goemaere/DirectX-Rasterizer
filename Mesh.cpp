#include "pch.h"
#include "Mesh.h"
#include "Math.h"

Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex> &vertices, const std::vector<uint32_t>& indices, Texture* pDiffuseTexture,
	Texture* pNormalTexture, Texture* pSpecularTexture, Texture* pGlossinessTexture)
	:m_pEffect{new Effect(pDevice,L"Resources/PosCol3D.fx")}
{
	// Create Vertex Layout
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 20;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	m_pEffect->GetTechnique(m_CurrentTechnique)->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result{ pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pInputLayout)};

	if (FAILED(result))
		return;

	// Create vertex buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result)) return;

	// Create index buffer
	m_NumIndices = static_cast<uint32_t>(indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result)) 
		return;

	m_pEffect->SetDiffuseMap(pDiffuseTexture);
	m_pEffect->SetNormalMap(pNormalTexture);
	m_pEffect->SetSpecularMap(pSpecularTexture);
	m_pEffect->SetGlossinessMap(pGlossinessTexture);
}

Mesh::~Mesh()
{
	delete m_pEffect;
	m_pEffect = nullptr;

	if (m_pInputLayout)
	{
		m_pInputLayout->Release();
		m_pInputLayout = nullptr;
	}
	
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}

	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
{
	//1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	//3. Set Vertex Buffer
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//4. Set Index Buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pEffect->GetTechnique(m_CurrentTechnique)->GetDesc(&techDesc);
	for (UINT p{}; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique(m_CurrentTechnique)->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

void Mesh::SetMatrix(const dae::Matrix& matrix, const dae::Matrix& invViewMatrix)
{
	m_pEffect->SetWorldViewProjMatrix(m_WorldMatrix * matrix);
	m_pEffect->SetWorldMatrix(m_WorldMatrix);
	m_pEffect->SetInvViewMatrix(invViewMatrix);
}

void Mesh::SwitchTechnique()
{
	switch (m_CurrentTechnique)
	{
	case Effect::Technique::Point:
		m_CurrentTechnique = Effect::Technique::Linear;
		break;
	case Effect::Technique::Linear:
		m_CurrentTechnique = Effect::Technique::Anisotropic;
		break;
	case Effect::Technique::Anisotropic:
		m_CurrentTechnique = Effect::Technique::Point;
		break;
	default:
		break;
	}
}

void Mesh::SetWorldMatrix(const dae::Matrix& matrix)
{
	m_WorldMatrix = matrix;
}

dae::Matrix Mesh::GetWorldMatrix() const
{
	return m_WorldMatrix;
}
