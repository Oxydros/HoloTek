#pragma once

#include "3D\Shaders\ShaderStructures.h"
#include "3D\Objects\GUI\Text\ATextObject.h"
#include "3D\Objects\GUI\Text\TextRenderer.h"
#include "3D\Resources\Texture2D.h"

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{
	class TextObject : public ATextObject
	{
	protected:
		std::wstring										m_text;
		float												m_fontSize = 60.0f;

		Microsoft::WRL::ComPtr<ID3D11InputLayout>			m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>			m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>		m_geometryShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>			m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_modelConstantBuffer;

		// Direct3D resources for the default texture.
		//Microsoft::WRL::ComPtr<ID3D11Resource>              _quadTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_quadTextureView;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_quadTextureSamplerState;
															
		ColorModelConstantBuffer							m_modelConstantBufferData;
		uint32_t											m_indexCount = 0;
															
		bool												m_loadingComplete = false;
		bool												m_usingVprtShaders = false;
		bool												m_updateText = false;															
															
		float4												m_color = { 0.2f, 0.2f, 0.2f, 0.0f };
		float2												m_size;

		DirectX::XMMATRIX									m_modelTransform;
															
		DirectX::XMFLOAT4X4									m_transform;
		DirectX::BoundingOrientedBox						m_boundingBox;
															
		std::unique_ptr<TextRenderer>						m_textRenderer;
		//std::unique_ptr<DistanceFieldRenderer>				m_distanceFieldRenderer;
	public:
		TextObject(std::shared_ptr<DX::DeviceResources> deviceResources,
			float2 size, float4 color = { 0.2f, 0.2f, 0.2f, 0.0f },
			std::wstring const &text = L"Default text");
		virtual ~TextObject() = default;

	public:
		std::future<void> CreateDeviceDependentResourcesAsync() override;
		void ReleaseDeviceDependentResources() override;
		void Render() override;
		void SetModelTransform(DirectX::XMMATRIX const & modelTransform) override;
		void GetBoundingBox(DirectX::BoundingOrientedBox & boundingBox) override;

	public:
		std::wstring const &getText() const override { return m_text; };
		void setText(std::wstring const &text) override {
			m_text = text;
			m_updateText = true;
		}

		float const getFontSize() const override { return m_fontSize; }
		void setFontSize(float font) override { m_fontSize = font; }

		void setColor(float4 color) override { m_color = color; };
		float4 getColor() const override { return (m_color); }

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