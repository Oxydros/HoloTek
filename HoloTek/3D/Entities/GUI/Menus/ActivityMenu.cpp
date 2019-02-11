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
	auto background = std::make_unique<Panel>(m_devicesResources, m_scene, float2(0.65f, 0.35f), float4(0.85f, 0.42f, 0.27f, 0.6f));
	background->SetRelativePosition({ 0.0f, 0.0f, 0.0f });

	auto leftArrow = std::make_unique<Button2D>(m_devicesResources, m_scene, float2(0.15f, 0.1f));
	leftArrow->setLabel(L"<-");
	leftArrow->SetAirTapCallback([this](Spatial::SpatialGestureRecognizer const &, Spatial::SpatialTappedEventArgs const &) {
		decOffset();
		RenderAtOffset(m_offset);
	});
	
	auto rightArrow = std::make_unique<Button2D>(m_devicesResources, m_scene, float2(0.15f, 0.1f));
	rightArrow->setLabel(L"->");
	rightArrow->SetAirTapCallback([this](Spatial::SpatialGestureRecognizer const &, Spatial::SpatialTappedEventArgs const &) {
		incOffset();
		RenderAtOffset(m_offset);
	});

	auto pageInfo = std::make_unique<Button2D>(m_devicesResources, m_scene, float2(0.15f, 0.1f));
	pageInfo->setLabel(L"N/A");

	m_background = background.get();
	m_leftArrow = leftArrow.get();
	m_rightArrow = rightArrow.get();
	m_pageInfo = pageInfo.get();

	background->AddGUIEntity(std::move(pageInfo), { -0.2f, 0.1f });
	background->AddGUIEntity(std::move(leftArrow), { -0.1f, 0.1f });
	background->AddGUIEntity(std::move(rightArrow), { 0.1f, 0.1f });

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
	m_propertyMutex.lock();
	m_activities =  co_await m_api.GetActivitiesAsync();
	m_offset = 0;
	m_currentPage = 0;
	m_maxPage = m_activities.size() / ACTI_RENDER_SZ;
	UpdateInfoLabel();
	m_propertyMutex.unlock();
	RenderAtOffset(m_offset);
	co_return;
}

void HoloTek::ActivityMenu::UpdateInfoLabel()
{
	std::wstringstream ss{};

	ss << m_currentPage << " / " << m_maxPage;
	m_pageInfo->setLabel(ss.str());
}

void HoloTek::ActivityMenu::RenderAtOffset(size_t offset)
{
	std::scoped_lock lock(m_propertyMutex);
	auto safeScene{ m_scene };

	UpdateInfoLabel();
	TRACE("Rendering activities at offset" << offset << std::endl;);
	//Schedule old list to be removed at the end of the frame
	if (m_activityList != nullptr) {
		/*m_background->RemoveChild(m_activityList);*/
		m_activityList->kill();
		m_activityList = nullptr;
	}

	//Create new list
	auto activityList = std::make_unique<EmptyEntity>(safeScene, "ActivityList", true);
	activityList->SetRelativePosition({ 0.0f, 0.0f, 0.0f });

	float posZ = (m_background->GetSize().z / 2) + 0.025f;
	constexpr float posXOffset = -0.3f;
	constexpr float posYOffset = -0.1f;
	constexpr float sizeX = 0.2f;
	constexpr float sizeY = 0.3f;

	for (size_t actiIdx = 0; actiIdx < ACTI_RENDER_SZ && (actiIdx + offset) < m_activities.size(); actiIdx++) {
		auto &acti = m_activities[actiIdx + offset];
		auto name = acti.actiTitle;

		auto position = float3(posXOffset + (sizeX * actiIdx) + 0.2, posYOffset, posZ);

		TRACE("Adding activity " << name.c_str() << " to activity menu" << std::endl;);
		auto activityButton = std::make_unique<Button2D>(m_devicesResources, safeScene, float2(sizeX, sizeY));
		activityButton->setLabel(name.c_str());
		activityButton->SetRelativePosition(position);
		activityButton->SetAirTapCallback([acti, safeScene](Spatial::SpatialGestureRecognizer const &, Spatial::SpatialTappedEventArgs const &) {
			TRACE("Begining face detection on activity " << acti.moduleName.c_str() << "-" << acti.codeEvent.c_str() << std::endl);
			safeScene->StartFaceDetection(acti);
		});

		activityList->AddChild(std::move(activityButton));
	}

	m_activityList = activityList.get();
	/*m_background->AddGUIEntity(std::move(activityList), { -0.1f, 0.2f });*/
	AddChild(std::move(activityList));
}
