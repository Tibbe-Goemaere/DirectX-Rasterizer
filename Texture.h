#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		Texture(const std::string& path, ID3D11Device* pDevice);
		~Texture();

		ID3D11ShaderResourceView* GetResourceView() const;
	private:
		ID3D11Texture2D* m_pResource{};
		ID3D11ShaderResourceView* m_pResourceView{};
	};
}
