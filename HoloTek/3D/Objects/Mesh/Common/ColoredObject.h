#pragma once

#include "3D\Shaders\ShaderStructures.h"
#include "3D\Utility\DirectXHelper.h"
#include "3D\Objects\Mesh\Interfaces\AColoredObject.h"

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{
	//Abstract class representing an object to be draw in
	//the holographic space
	class ColoredObject : public AColoredObject
	{
	protected:
		std::wstring 									m_vertexShaderString{};
		std::wstring 									m_pixelShaderString{};
		std::wstring 									m_geometryShaderString{};
		std::wstring 									m_vprtVertexShaderString{};

		Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout{};
		Microsoft::WRL::ComPtr<ID3D11Buffer>            m_vertexBuffer{};
		Microsoft::WRL::ComPtr<ID3D11Buffer>            m_indexBuffer{};
		Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_vertexShader{};
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>    m_geometryShader{};
		Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pixelShader{};
		Microsoft::WRL::ComPtr<ID3D11Buffer>            m_modelConstantBuffer{};

		ColorModelConstantBuffer                        m_modelConstantBufferData{};
		uint32_t                                        m_indexCount = 0;

		bool                                            m_loadingComplete = false;
		bool                                            m_usingVprtShaders = false;

	protected:
		winrt::Windows::Foundation::Numerics::float4	m_color = { 1.0f, 1.0f, 1.0f, 1.0f };

		DirectX::XMMATRIX								m_modelTransform;
		DirectX::XMFLOAT4X4								m_transform;
		DirectX::BoundingOrientedBox					m_boundingBox;

	public:
		ColoredObject(std::shared_ptr<DX::DeviceResources> devicesResources,
			std::wstring const &vertexShaderString = L"ms-appx:///VertexShaderColor.cso",
			std::wstring const &pixelShaderString = L"ms-appx:///PixelShader.cso",
			std::wstring const &geometryShaderString = L"ms-appx:///GeometryShader.cso",
			std::wstring const &vprtVertexShaderString = L"ms-appx:///VPRTVertexShader.cso"
		);
		virtual ~ColoredObject();

	public:
		std::future<void>						CreateDeviceDependentResourcesAsync() override;
		void									ReleaseDeviceDependentResources() override;
		void									Render() override;
		void									SetModelTransform(DirectX::XMMATRIX const &m) override;

	private:
		std::future<void >					InitializeShaders();
		virtual void						CreateMesh() = 0;

	public:
		void GetBoundingBox(DirectX::BoundingOrientedBox &boundingBox) override;
		void SetColor(winrt::Windows::Foundation::Numerics::float4 color) override { m_color = color; }
		float4 GetColor() const override { return m_color; }

	public:
		// Required for align of 16B for XMMAtrix
		// https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16
		void* operator new(size_t i)
		{
			return _mm_malloc(i, 16);
		}

		// Required for align of 16B for XMMAtrix
		// https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16
		void operator delete(void* p)
		{
			_mm_free(p);
		}
	};
}