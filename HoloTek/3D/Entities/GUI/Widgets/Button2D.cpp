#include "pch.h"
#include <3D\Entities\GUI\Widgets\Button2D.h>
#include <3D\Objects\GUI\ButtonObject.h>

winrt::Windows::Foundation::Numerics::float4 HoloTek::Button2D::DEFAULT_COLOR = { 0.443f, 0.515f, 0.454f, 1.0f };
winrt::Windows::Foundation::Numerics::float4 HoloTek::Button2D::FOCUS_COLOR = { 0.343f, 0.415f, 0.354f, 1.0f };

HoloTek::Button2D::Button2D(std::shared_ptr<DX::DeviceResources> devicesResources,
							   std::shared_ptr<HolographicScene> scene,
							   winrt::Windows::Foundation::Numerics::float2 size)
	: GUIEntity(scene, Spatial::SpatialGestureSettings::Tap)
{
	m_color = DEFAULT_COLOR;
	SetSize({ size.x, size.y, 0.0f });
	auto buttonMesh = std::make_unique<ButtonObject>(devicesResources, size, m_color);
	addMesh(std::move(buttonMesh));
}

HoloTek::Button2D::~Button2D()
{}

void HoloTek::Button2D::OnLabelChanged()
{
	auto button = dynamic_cast<ButtonObject*>(m_mesh.get());
	button->setText(getLabel());
}

bool HoloTek::Button2D::OnGetFocus()
{
	auto button = dynamic_cast<ButtonObject*>(m_mesh.get());
	button->setColor(FOCUS_COLOR);
	return (true);
}

bool HoloTek::Button2D::OnLostFocus()
{
	auto button = dynamic_cast<ButtonObject*>(m_mesh.get());
	button->setColor(DEFAULT_COLOR);
	return (true);
}
