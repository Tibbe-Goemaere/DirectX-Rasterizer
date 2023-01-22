#pragma once
class Matrix;
#include "Texture.h"

class Effect
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	~Effect();
	//Rule of 5
	Effect(const Effect& other) = delete;
	Effect& operator=(const Effect& other) = delete;
	Effect(Effect&& other) = delete;
	Effect& operator=(Effect&& other) = delete;

	enum class Technique{Point,Linear,Anisotropic};

	ID3DX11Effect* GetEffect() const;
	ID3DX11EffectTechnique* GetTechnique(const Technique technique) const;

	void SetWorldViewProjMatrix(const dae::Matrix& matrix);
	void SetInvViewMatrix(const dae::Matrix& matrix);
	void SetWorldMatrix(const dae::Matrix& matrix);
	
	void SetDiffuseMap(dae::Texture* pDiffuseTexture);
	void SetNormalMap(dae::Texture* pNormalTexture);
	void SetSpecularMap(dae::Texture* pSpecularTexture);
	void SetGlossinessMap(dae::Texture* pGlossinessTexture);

private:
	ID3DX11Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pPointTechnique{};
	ID3DX11EffectTechnique* m_pLinearTechnique{};
	ID3DX11EffectTechnique* m_pAnisotropicTechnique{};

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
	ID3DX11EffectMatrixVariable* m_pWorldVariable{};
	ID3DX11EffectMatrixVariable* m_pViewInverseVariable{};

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};
