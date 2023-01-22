#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle, float ar) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		float nearPlane{ 0.1f };
		float farPlane{ 100.f };

		Matrix projectionMatrix{};
		float aspectRatio{};

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f }, float _ar = 1.f)
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
			aspectRatio = _ar;

			CalculateProjectionMatrix();
		}

		void CalculateViewMatrix()
		{
			forward = Matrix::CreateRotation(Vector3(totalPitch, totalYaw, 0)).TransformVector(Vector3::UnitZ);
			forward.Normalize();

			invViewMatrix = Matrix::CreateLookAtLH(origin, forward, Vector3::UnitY);
			viewMatrix = Matrix::Inverse(invViewMatrix);

			right = viewMatrix.GetAxisX();
			up = viewMatrix.GetAxisY();
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		const dae::Matrix& GetViewMatrix() const
		{
			return viewMatrix;
		}

		const dae::Matrix& GetInverseViewMatrix() const
		{
			return invViewMatrix;
		}

		const dae::Matrix& GetProjectionMatrix() const
		{
			return projectionMatrix;
		}
		void Update(const Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			const float moveSpeed{ 20 };
			const float rotSpeed{ 15 };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += moveSpeed * deltaTime * forward;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= moveSpeed * deltaTime * forward;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= moveSpeed * deltaTime * right;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += moveSpeed * deltaTime * right;
			}
			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) && mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				if (mouseY > 0)
				{
					origin -= moveSpeed * deltaTime * up;
				}
				if (mouseY < 0)
				{
					origin += moveSpeed * deltaTime * up;
				}
				if (mouseX > 0)
				{
					origin += moveSpeed * deltaTime * right;
				}
				if (mouseX < 0)
				{
					origin -= moveSpeed * deltaTime * right;
				}
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				if (mouseY > 0)
				{
					origin -= moveSpeed * deltaTime * forward;
				}
				if (mouseY < 0)
				{
					origin += moveSpeed * deltaTime * forward;
				}
				if (mouseX > 0)
				{
					totalYaw += rotSpeed * deltaTime;
				}
				if (mouseX < 0)
				{
					totalYaw -= rotSpeed * deltaTime;
				}
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				if (mouseY > 0)
				{
					totalPitch -= rotSpeed * deltaTime;
				}
				if (mouseY < 0)
				{
					totalPitch += rotSpeed * deltaTime;
				}
				if (mouseX > 0)
				{
					totalYaw += rotSpeed * deltaTime;
				}
				if (mouseX < 0)
				{
					totalYaw -= rotSpeed * deltaTime;
				}
			}

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}