#pragma once
#include "Effect.h"
#include "Datatypes.h"
#include <vector>

class Matrix;

class Mesh
{
public:
	Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Texture* pDiffuseTexture,
		Texture* pNormalTexture, Texture* pSpecularTexture, Texture* pGlossinessTexture);
	~Mesh();
	//Rule of 5
	Mesh(const Mesh& other) = delete;
	Mesh& operator=(const Mesh& other) = delete;
	Mesh(Mesh&& other) = delete;
	Mesh& operator=(Mesh&& other) = delete;

	void Render(ID3D11DeviceContext* pDeviceContext) const;
	void SetMatrix(const dae::Matrix& matrix, const dae::Matrix& invViewMatrix);
	void SwitchTechnique();
	void SetWorldMatrix(const dae::Matrix& matrix);
	dae::Matrix GetWorldMatrix() const;

private:
	Effect* m_pEffect{};
	Effect::Technique m_CurrentTechnique{Effect::Technique::Point};
	
	ID3D11InputLayout* m_pInputLayout{};
	ID3D11Buffer* m_pVertexBuffer{};
	uint32_t m_NumIndices{};
	ID3D11Buffer* m_pIndexBuffer{};

	dae::Matrix m_WorldMatrix{};
};
