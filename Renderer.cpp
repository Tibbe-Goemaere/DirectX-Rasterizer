#include "pch.h"
#include "Renderer.h"
#include "Utils.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}
		
		
		//Initialize camera
		m_Camera.Initialize(45.f,{0.f,0.f,-50.f}, static_cast<float>(m_Width) / m_Height);

		//Initialize textures
		m_pDiffuseTexture = new Texture{ "Resources/vehicle_diffuse.png",m_pDevice };
		m_pNormalTexture = new Texture{ "Resources/vehicle_normal.png",m_pDevice };
		m_pSpecularTexture = new Texture{ "Resources/vehicle_specular.png",m_pDevice };
		m_pGlossinessTexture = new Texture{ "Resources/vehicle_gloss.png",m_pDevice };
		//Initialize mesh
		InitializeMesh();
	}

	Renderer::~Renderer()
	{
		//We need to Delete in reverse order

		delete m_pGlossinessTexture;
		m_pGlossinessTexture = nullptr;

		delete m_pSpecularTexture;
		m_pSpecularTexture = nullptr;

		delete m_pNormalTexture;
		m_pNormalTexture = nullptr;

		delete m_pDiffuseTexture;
		m_pDiffuseTexture = nullptr;

		delete m_pMesh;
		m_pMesh = nullptr;

		m_pRenderTargetView->Release();
		m_pRenderTargetView = nullptr;

		m_pRenderTargetBuffer->Release();
		m_pRenderTargetBuffer = nullptr;

		m_pDepthStencilView->Release();
		m_pDepthStencilView = nullptr;

		m_pDepthStencilBuffer->Release();
		m_pDepthStencilBuffer = nullptr;

		m_pSwapChain->Release();
		m_pSwapChain = nullptr;

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		m_pDevice->Release();
		m_pDevice = nullptr;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);
		m_pMesh->SetMatrix(m_Camera.GetInverseViewMatrix()* m_Camera.GetProjectionMatrix(),m_Camera.GetInverseViewMatrix());

		if (m_UsesRotation)
		{
			const float rotSpeed{ 50.f };
			m_pMesh->SetWorldMatrix(Matrix::CreateRotationY(pTimer->GetElapsed() * rotSpeed * TO_RADIANS) * m_pMesh->GetWorldMatrix());
		}

		const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

		if (pKeyboardState[SDL_SCANCODE_F2])
		{
			if (!m_IsF2Pressed)
			{
				m_pMesh->SwitchTechnique();
			}
			m_IsF2Pressed = true;
		}
		else
		{
			m_IsF2Pressed = false;
		}

		if (pKeyboardState[SDL_SCANCODE_F5])
		{
			if (!m_IsF5Pressed)
			{
				m_UsesRotation = !m_UsesRotation;
			}
			m_IsF5Pressed = true;
		}
		else
		{
			m_IsF5Pressed = false;
		}
	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		// 1. CLEAR RTV & DSV
		ColorRGB clearColor{ 0.0f, 0.0f, 0.3f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// 2. SET PIPELINE + INVOKE DRAWCALLS (= RENDER)
		m_pMesh->Render(m_pDeviceContext);

		// 3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);

	}

	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device & DeviceContext

		D3D_FEATURE_LEVEL feautureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &feautureLevel,
			1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

		if (FAILED(result))
			return result;

		// Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
			return result;

		// 2. Create Swapchain 

		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		// Get the handle (HWND) from the SDL backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		// Create swapchain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result)) 
			return result;

		// 3. Create DepthStencil (DS) and DepthStencilView (DSV)
		// Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		// View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result)) 
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result)) 
			return result;

		// 4. Create RenderTarget (RT) and RenderTargetView (RTV)

		// Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result)) 
			return result;

		// View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result)) 
			return result;

		// 5. Bind RTV and DSV to Output Merger Stage
		
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		// 6. Set viewport

		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<FLOAT>(m_Width);
		viewport.Height = static_cast<FLOAT>(m_Height);
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return S_OK;
	}

	void Renderer::InitializeMesh()
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);
		m_pMesh = new Mesh{ m_pDevice, vertices, indices,m_pDiffuseTexture,
			m_pNormalTexture, m_pSpecularTexture, m_pGlossinessTexture };
	}
}
