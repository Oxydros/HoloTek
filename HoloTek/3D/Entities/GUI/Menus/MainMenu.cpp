#include "pch.h"
#include <3D\Entities\GUI\Menus\MainMenu.h>
#include <3D\Entities\GUI\Widgets\Panel.h>
#include <3D\Entities\GUI\Widgets\Button2D.h>
#include "3D\Scene\HolographicScene.h"

using namespace winrt::Windows::UI::Input;

HoloTek::MainMenu::MainMenu(std::shared_ptr<DX::DeviceResources> devicesResources,
								   std::shared_ptr<HolographicScene> scene)
	: Entity(scene), m_devicesResources(devicesResources)
{
}

HoloTek::MainMenu::~MainMenu()
{
}

void HoloTek::MainMenu::InitializeMenu()
{
	auto m_background = std::make_unique<Panel>(m_devicesResources, m_scene, float2(0.65f, 0.35f), float4(0.7f, 0.1f, 0.2f, 0.6f));
	m_background->SetRelativePosition({ 0.0f, 0.0f, 0.0f });

	auto buttonCube = std::make_unique<Button2D>(m_devicesResources, m_scene, float2(0.15f, 0.1f));
	buttonCube->setLabel(L"Spawn cube");

	auto buttonSphere = std::make_unique<Button2D>(m_devicesResources, m_scene, float2(0.15f, 0.1f));
	buttonSphere->setLabel(L"Spawn sphere");

	auto safeScene{ m_scene };

	auto buttonLeave = std::make_unique<Button2D>(m_devicesResources, m_scene, float2(0.15f, 0.1f));
	buttonLeave->setLabel(L"Leave 3D");
	buttonLeave->SetAirTapCallback([safeScene](Spatial::SpatialGestureRecognizer const &, Spatial::SpatialTappedEventArgs const &) {
		TRACE("Killing the scene" << std::endl);
		safeScene->kill();
	});

	m_background->AddGUIEntity(std::move(buttonCube), { -0.1f, 0.1f });
	m_background->AddGUIEntity(std::move(buttonSphere), { 0.1f, 0.1f });
	m_background->AddGUIEntity(std::move(buttonLeave), { -0.1f, -0.1f });

	AddChild(std::move(m_background));
}
