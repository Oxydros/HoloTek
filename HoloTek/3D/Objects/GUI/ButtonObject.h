#pragma once

#include <3D\Objects\GUI\Text\ATextObject.h>
#include <3D\Objects\Mesh\2DFormes\ColoredRectangle.h>
#include <3D\Objects\GUI\Text\TextObject.h>

namespace HoloTek
{
	class ButtonObject : public ATextObject
	{
	protected:
		std::unique_ptr<TextObject>					m_textMesh;
		std::unique_ptr<ColoredRectangle>			m_backgroundMesh;

		winrt::Windows::Foundation::Numerics::float2		m_size;
		winrt::Windows::Foundation::Numerics::float4		m_color = { 1.0f, 1.0f, 1.0f, 1.0f };

	public:
		ButtonObject(std::shared_ptr<DX::DeviceResources> devicesResources,
					 winrt::Windows::Foundation::Numerics::float2 size,
					 winrt::Windows::Foundation::Numerics::float4 backgroundColor = { 0.243f, 0.815f, 0.854f, 1.0f });
		~ButtonObject() = default;

		// Inherited via ATextObject
		std::future<void> CreateDeviceDependentResourcesAsync() override;
		void ReleaseDeviceDependentResources() override;
		void Render() override;
		void SetModelTransform(DirectX::XMMATRIX const & modelTransform) override;
		void GetBoundingBox(DirectX::BoundingOrientedBox & boundingBox) override;

	public:
		std::wstring const & getText() const override;
		void setText(std::wstring const & text) override;

		float const getFontSize() const override;
		void setFontSize(float font) override;

		void setColor(winrt::Windows::Foundation::Numerics::float4) override;
		winrt::Windows::Foundation::Numerics::float4 getColor() const override;
	};
}