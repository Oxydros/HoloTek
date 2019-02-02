#pragma once

#include "3D/Resources/DeviceResources.h"
#include "3D/Utility/StepTimer.h"
#include "3D/Shaders/ShaderStructures.h"

namespace HoloTek
{
	// This sample renderer instantiates a basic rendering pipeline.
	class SpinningCubeRenderer : public DX::Resource
	{
	public:
		SpinningCubeRenderer(std::shared_ptr<DX::DeviceResources> deviceResources);
		std::future<void> CreateDeviceDependentResourcesAsync() override;
		void ReleaseDeviceDependentResources() override;
		void Update(DX::StepTimer const& timer);
		void Render();

		void SetTargetPosition(winrt::Windows::Foundation::Numerics::float3 pos) { m_targetPosition = pos; }

		// Property accessors.
		winrt::Windows::Foundation::Numerics::float3 const& GetPosition() const { return m_position; }
		winrt::Windows::Foundation::Numerics::float3 const& GetVelocity() const { return m_velocity; }

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources>            m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>            m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>            m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>    m_geometryShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>            m_modelConstantBuffer;

		// System resources for cube geometry.
		CubeModelConstantBuffer                         m_modelConstantBufferData;
		uint32_t                                        m_indexCount = 0;

		// Variables used with the rendering loop.
		bool                                            m_loadingComplete = false;
		float                                           m_degreesPerSecond = 45.f;
		winrt::Windows::Foundation::Numerics::float3    m_targetPosition = { 0.f, 0.f, -2.f };
		winrt::Windows::Foundation::Numerics::float3    m_position = { 0.f, 0.f, -2.f };
		winrt::Windows::Foundation::Numerics::float3    m_velocity = { 0.f, 0.f, 0.f };

		// If the current D3D Device supports VPRT, we can avoid using a geometry
		// shader just to set the render target array index.
		bool                                            m_usingVprtShaders = false;

		// This is the rate at which the hologram position is interpolated (LERPed) to the current location.
		const float                                     c_lerpRate = 4.0f;
	};
}
