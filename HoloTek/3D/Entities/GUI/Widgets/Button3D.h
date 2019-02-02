#pragma once

#include <3D\Entities\GUI\GUIEntity.h>

using namespace winrt::Windows::UI::Input;

namespace HoloTek
{
	class Button3D : public GUIEntity
	{
	public:
		static winrt::Windows::Foundation::Numerics::float4 DEFAULT_COLOR;
		static winrt::Windows::Foundation::Numerics::float4 FOCUS_COLOR;

	public:
		Button3D(std::shared_ptr<DX::DeviceResources> devicesResources, std::shared_ptr<HolographicScene> scene,
			winrt::Windows::Foundation::Numerics::float3 size = { 0.2f, 0.2f, 0.2f });
		virtual ~Button3D() = default;

	public:
		void DoUpdate(DX::StepTimer const &timer) override {};

		bool OnGetFocus() override;
		bool OnLostFocus() override;

	public:
		std::string const GetLabel() const override { return std::string(m_label.begin(), m_label.end()); }
	};
}