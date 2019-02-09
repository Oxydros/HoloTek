#include "pch.h"
#include "3D\Scene\HolographicScene.h"
#include "3D\Entities\CursorEntity.h"

#include "API/IntraAPI.h"

using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Media::Capture;
using namespace winrt::Windows::Media::Capture::Frames;

HoloTek::HolographicScene::HolographicScene(std::shared_ptr<DX::DeviceResources> deviceResources)
	: m_deviceResources(deviceResources), m_api("XXXXXXXXXXXXXXX")
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

	co_await mainMenu->InitializeMenuAsync();
	m_mainMenu = mainMenu.get();

	addEntity(std::move(mainMenu));

	auto actiMenu = std::make_unique<ActivityMenu>(m_deviceResources, safeScene, m_api);
	actiMenu->SetRelativePosition({ 1.0f, 0.0f, -3.0f });

	co_await actiMenu->InitializeMenuAsync();
	actiMenu->setVisible(false);
	m_activityMenu = actiMenu.get();

	addEntity(std::move(actiMenu));

	auto status = co_await m_api.LoginAsync();
	if (status != winrt::Windows::Web::Http::HttpStatusCode::Ok)
	{
		TRACE("Logged in status " << "NOT OK" << std::endl);
	}
	else {
		TRACE("Logged in status " << "NICKEL" << std::endl);
	}

	m_facesBuffer = std::make_shared<FacesBuffer>();
	co_await m_facesBuffer->InitializeAsync();

	m_videoFrameProcessor = co_await VideoFrameProcessor::CreateAsync();
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

	if (m_videoFrameProcessor)
	{
		std::scoped_lock lock(m_propertyMutex);

		MediaFrameReference frame = m_videoFrameProcessor->GetLatestFrame();
		//Check that we are currently processing faces for an activity and
		//that we got a valid frame
		if (m_processingFaces && frame != nullptr)
		{
			auto videoFrame = frame.VideoMediaFrame();
			if (videoFrame != nullptr)
			{
				auto bitmap = videoFrame.SoftwareBitmap();
				//Check that the face buffer is not processing right now
				//as it is async (not necessary since there is already a 
				//check inside, but just to be sure)
				if (bitmap != nullptr && m_facesBuffer->isProcessing() == false)
				{
					TRACE("Launching match faces for activity " << m_currentActivity.codeEvent.c_str() << std::endl);
					/*m_facesBuffer->GetMatchingImagesAsync(std::move(bitmap));*/
					ProcessStudentsFacesForActivityAsync(std::move(bitmap), m_studentsToCheck, m_currentActivity);
				}
			}
		}
	}

	m_root->Update(timer);
}

//Call when clicking on an activity in the Activity Menu
std::future<void> HoloTek::HolographicScene::StartFaceDetection(IntraAPI::Activity const &activity) {
	m_propertyMutex.lock();

	if (m_processingFaces) {
		m_propertyMutex.unlock();
		co_return;
	}
	m_propertyMutex.unlock();
	auto registeredStudents = co_await m_api.GetRegisteredStudentsAsync(activity);
	m_propertyMutex.lock();
	TRACE(registeredStudents.size() << " registered students for this activity:" << std::endl);
	m_studentsToCheck.clear();
	m_currentActivity = activity;
	for (auto student : registeredStudents) {
		TRACE("		" << student.title.c_str() << std::endl);
		m_studentsToCheck.push_back(std::string(student.login.begin(), student.login.end()));
	}
	m_processingFaces = true;
	m_mainMenu->DisplayStopButton();
	m_propertyMutex.unlock();
	co_return;
}

//Call during the update, as a non blocking co routine
winrt::Windows::Foundation::IAsyncAction HoloTek::HolographicScene::ProcessStudentsFacesForActivityAsync(winrt::Windows::Graphics::Imaging::SoftwareBitmap facesToFind,
	std::vector<std::string> studentsToFind, IntraAPI::Activity currentActivity)
{
	TRACE("Launching process on students to check" << std::endl);
	auto studentsFound = co_await m_facesBuffer->GetMatchingImagesAsync(std::move(facesToFind), studentsToFind);
	TRACE("Found " << studentsFound.Size() << " present in the activity " << m_currentActivity.codeEvent.c_str() << std::endl);
	for (auto student : studentsFound) {
		TRACE("Found " << student.c_str() << " !" << std::endl);
	}
	if (studentsFound.Size() > 0) {
		co_await m_api.MarkRegisteredStudentsAsync(currentActivity, studentsFound);
	}
	co_return;
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
