#include "pch.h"
#include "3D\Scene\HolographicScene.h"
#include "3D\Entities\CursorEntity.h"

HoloTek::HolographicScene::HolographicScene(std::shared_ptr<DX::DeviceResources> deviceResources)
	: m_deviceResources(deviceResources)
{
}

HoloTek::HolographicScene::~HolographicScene()
{
}

std::future<void> HoloTek::HolographicScene::InitializeAsync()
{
	TRACE("Initializing scene" << std::endl);
	auto safeScene{ shared_from_this() };

	m_root = std::make_unique<EmptyEntity>(safeScene, "Root", true);

	TRACE("Created root" << std::endl);

	////Declare gaze
	auto gaze = std::make_unique<CursorEntity>(m_deviceResources, safeScene);

	TRACE("Created cursor" << std::endl);
	m_cursor = gaze.get();
	TRACE("Adding cursor" << std::endl);
	addEntity(std::move(gaze));

	auto mainMenu = std::make_unique<MainMenu>(m_deviceResources, safeScene);
	mainMenu->SetRelativePosition({ 0.0f, 0.0f, -3.0f });

	mainMenu->InitializeMenu();
	m_mainMenu = mainMenu.get();

	addEntity(std::move(mainMenu));
	co_return;
}

void HoloTek::HolographicScene::InteractionDetectedEvent(winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const &sender,
	winrt::Windows::UI::Input::Spatial::SpatialInteractionDetectedEventArgs const &args)
{	
	if (m_focusedEntity)
		m_focusedEntity->CaptureInteraction(args.Interaction());
}

void HoloTek::HolographicScene::Update(DX::StepTimer const& timer)
{
	auto pair = m_root->getNearestInGazeEntity();

	//Focus only nearest entity in gaze direction
	if (pair.second >= 0)
	{
		/*TRACE("Nearest entity is " << pair.first << " " << pair.first->GetLabel() << " " << pair.second << std::endl);*/
		if (m_focusedEntity != nullptr && m_focusedEntity != pair.first)
			m_focusedEntity->setFocus(false);
		pair.first->setFocus(true);
		m_focusedEntity = pair.first;
	}
	else if (m_focusedEntity != nullptr)
	{
		//No entity found in gaze, remove focus from actual entity
		m_focusedEntity->setFocus(false);
		m_focusedEntity = nullptr;
	}

	m_root->Update(timer);
}

void HoloTek::HolographicScene::Render()
{
	m_root->Render();
}

void HoloTek::HolographicScene::UpdateCoordinateSystem(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const &coordinateSystem)
{
	m_coordinateSystem = coordinateSystem;
}

void HoloTek::HolographicScene::UpdatePointerPose(winrt::Windows::UI::Input::Spatial::SpatialPointerPose const &pointerPose)
{
	m_pointerPose = pointerPose;
}

void HoloTek::HolographicScene::OnDeviceLost()
{
	m_root->ReleaseMesh();
}

void HoloTek::HolographicScene::OnDeviceRestored()
{
	m_root->InitializeMeshAsync();
}
void HoloTek::HolographicScene::addEntity(IEntity::IEntityPtr e)
{
	//TRACE("Adding new entity to pending list " << e.get() << std::endl);
	if (m_root == nullptr)
		std::runtime_error("The root entity is not allocated yet !");
	m_root->AddChild(std::move(e));
}

void HoloTek::HolographicScene::addEntityInFront(IEntity::IEntityPtr e, float dist)
{
	if (m_pointerPose != nullptr)
	{
		// Get the gaze direction relative to the given coordinate system.
		const winrt::Windows::Foundation::Numerics::float3 headPosition = m_pointerPose.Head().Position();
		const winrt::Windows::Foundation::Numerics::float3 headDirection = m_pointerPose.Head().ForwardDirection();

		// The tag-along hologram follows a point 2.0m in front of the user's gaze direction.
		const winrt::Windows::Foundation::Numerics::float3 gazeAtTwoMeters = headPosition + (dist * headDirection);

		e->SetRealPosition(gazeAtTwoMeters);
		e->SetRealRotation({ 0, 0, 0 });
		addEntity(std::move(e));
	}
}

winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const &HoloTek::HolographicScene::getCoordinateSystem() const
{
	return (m_coordinateSystem);
}

winrt::Windows::UI::Input::Spatial::SpatialPointerPose const &HoloTek::HolographicScene::getPointerPose() const
{
	return (m_pointerPose);
}
