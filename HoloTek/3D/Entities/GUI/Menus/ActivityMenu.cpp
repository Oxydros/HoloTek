#include "pch.h"
#include <3D\Entities\GUI\Menus\ActivityMenu.h>
#include <3D\Entities\GUI\Widgets\Panel.h>
#include <3D\Entities\GUI\Widgets\Button2D.h>
#include "3D\Scene\HolographicScene.h"

using namespace winrt::Windows::UI::Input;

HoloTek::ActivityMenu::ActivityMenu(std::shared_ptr<DX::DeviceResources> devicesResources,
	std::shared_ptr<HolographicScene> scene, IntraAPI const &api)
	: Entity(scene), m_devicesResources(devicesResources), m_api(api)
{
}

HoloTek::ActivityMenu::~ActivityMenu()
{
}

std::future<void> HoloTek::ActivityMenu::InitializeMenuAsync()
{
	auto safeScene{ m_scene };

	auto background = std::make_unique<Panel>(m_devicesResources, m_scene, float2(0.65f, 0.35f), float4(0.7f, 0.1f, 0.2f, 0.6f));
	background->SetRelativePosition({ 0.0f, 0.0f, 0.0f });

	auto buttonCube = std::make_unique<Button2D>(m_devicesResources, m_scene, float2(0.15f, 0.1f));
	buttonCube->setLabel(L"This is Activity");

	auto buttonLeave = std::make_unique<Button2D>(m_devicesResources, m_scene, float2(0.15f, 0.1f));
	buttonLeave->setLabel(L"Go back home");
	buttonLeave->SetAirTapCallback([safeScene](Spatial::SpatialGestureRecognizer const &, Spatial::SpatialTappedEventArgs const &) {
		TRACE("Switching menu" << std::endl);
		safeScene->GoToMainMenu();
	});

	background->AddGUIEntity(std::move(buttonCube), { -0.1f, 0.1f });
	background->AddGUIEntity(std::move(buttonLeave), { -0.1f, -0.1f });

	m_background = background.get();

	AddChild(std::move(background));

	co_await refreshActivityListAsync();
	co_return;
}

void HoloTek::ActivityMenu::setVisible(bool visibility)
{
	Entity::setVisible(visibility);
	//Trigger refresh when visibility turn back to true
	if (visibility == true) {
		refreshActivityListAsync();
	}
}

std::future<void> HoloTek::ActivityMenu::refreshActivityListAsync()
{
	auto safeScene{ m_scene };

	//Schedule old list to be removed at the end of the frame
	if (m_activityList != nullptr) {
		m_activityList->kill();
		m_activityList = nullptr;
	}

	//Create new list
	auto activityList = std::make_unique<EmptyEntity>(safeScene, "ActivityList", true);
	activityList->SetRelativePosition({ 0.0f, 0.0f, 0.0f });

	auto activities =  co_await m_api.GetActivitiesAsync();

	auto zPos = (m_background->GetSize().z / 2) + 0.025f;
	constexpr float sizeX = 0.15f;
	constexpr float sizeY = 0.15f;

	for (size_t i = 0; i < 3 && i < activities.size(); i++) {
		auto &acti = activities[i];
		auto name = acti.codeActi;

		auto position = float3((sizeX * i) + 0.4, 0, zPos);

		TRACE("Adding activity " << acti.codeActi.c_str() << " to activity menu" << std::endl;);
		auto buttonSphere = std::make_unique<Button2D>(m_devicesResources, safeScene, float2(sizeX, sizeY));
		buttonSphere->setLabel(name.c_str());
		buttonSphere->SetRelativePosition(position);
		buttonSphere->SetAirTapCallback([=](Spatial::SpatialGestureRecognizer const &, Spatial::SpatialTappedEventArgs const &) {
			TRACE("Got interraction on activity " << name.c_str() << std::endl);
		});

		activityList->AddChild(std::move(buttonSphere));
	}

	m_activityList = activityList.get();
	AddChild(std::move(activityList));
	co_return;
}
