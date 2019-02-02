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

std::future<void> HoloTek::MainMenu::InitializeMenuAsync()
{
	auto safeScene{ m_scene };

	auto background = std::make_unique<Panel>(m_devicesResources, m_scene, float2(0.65f, 0.35f), float4(0.7f, 0.1f, 0.2f, 0.6f));
	background->SetRelativePosition({ 0.0f, 0.0f, 0.0f });

	auto buttonCube = std::make_unique<Button2D>(m_devicesResources, m_scene, float2(0.15f, 0.1f));
	buttonCube->setLabel(L"This is home");

	auto buttonSphere = std::make_unique<Button2D>(m_devicesResources, m_scene, float2(0.15f, 0.1f));
	buttonSphere->setLabel(L"Spawn sphere");

	auto buttonLeave = std::make_unique<Button2D>(m_devicesResources, m_scene, float2(0.15f, 0.1f));
	buttonLeave->setLabel(L"Go to activity");
	buttonLeave->SetAirTapCallback([safeScene](Spatial::SpatialGestureRecognizer const &, Spatial::SpatialTappedEventArgs const &) {
		TRACE("Switching menu" << std::endl);
		safeScene->GoToActivityMenu();
	});

	background->AddGUIEntity(std::move(buttonCube), { -0.1f, 0.1f });
	background->AddGUIEntity(std::move(buttonSphere), { 0.1f, 0.1f });
	background->AddGUIEntity(std::move(buttonLeave), { -0.1f, -0.1f });

	AddChild(std::move(background));
	co_return;
}
