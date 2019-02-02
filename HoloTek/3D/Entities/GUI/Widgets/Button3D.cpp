#include "pch.h"
#include <3D\Entities\GUI\Widgets\Button3D.h>
#include <3D\Objects\Mesh\3DFormes\ColoredCube.h>

winrt::Windows::Foundation::Numerics::float4 HoloTek::Button3D::DEFAULT_COLOR = { 0.243f, 0.815f, 0.854f, 1.0f };
winrt::Windows::Foundation::Numerics::float4 HoloTek::Button3D::FOCUS_COLOR = { 0.443f, 0.515f, 0.454f, 1.0f };

HoloTek::Button3D::Button3D(std::shared_ptr<DX::DeviceResources> devicesResources,
	std::shared_ptr<HolographicScene> scene, winrt::Windows::Foundation::Numerics::float3 size)
	: GUIEntity(scene, Spatial::SpatialGestureSettings::Tap | Spatial::SpatialGestureSettings::ManipulationTranslate)
{
	m_color = DEFAULT_COLOR;
	SetSize(size);
	auto buttonMesh = std::make_unique<ColoredCube>(devicesResources, size, m_color);
	addMesh(std::move(buttonMesh));
}

bool HoloTek::Button3D::OnGetFocus()
{
	auto button = dynamic_cast<ColoredCube*>(m_mesh.get());
	button->SetColor(FOCUS_COLOR);
	return (true);
}

bool HoloTek::Button3D::OnLostFocus()
{
	auto button = dynamic_cast<ColoredCube*>(m_mesh.get());
	button->SetColor(DEFAULT_COLOR);
	return (true);
}