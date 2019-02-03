#include "pch.h"
#include "MainPage.h"

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Media;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Storage::Streams;
using namespace std;

namespace winrt::DesktopTek::implementation
{
	MainPage::MainPage() : m_frameReader(nullptr), m_latestBitmap(nullptr),
		m_api("XXXXXXXXXXX"),
		m_processFaces(false)
	{
		InitializeComponent();
		PlanningButton().Visibility(Visibility::Collapsed);
		StudentPanel().Visibility(Visibility::Collapsed);
	}

	std::future<void> MainPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const &e)
	{
		auto respoCode = co_await m_api.LoginAsync();
		if (respoCode != winrt::Windows::Web::Http::HttpStatusCode::Ok) {
			TRACE("Receive error when trying to log in");
			std::runtime_error("Couldn't logged into the intra");
		}
		PlanningButton().Visibility(Visibility::Visible);
		m_faceBuffer.InitializeAsync();
		InitializeCameraAsync();
	}

	IAsyncAction MainPage::PlanningHandler(IInspectable const&, RoutedEventArgs const&)
	{
		m_propertiesLock.lock();
		m_activities = co_await m_api.GetActivitiesAsync();
		m_propertiesLock.unlock();
		ProcessActivitiesAsync();
	}

	IAsyncAction MainPage::ActivityHandler(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & args)
	{
		std::scoped_lock lock(m_propertiesLock);

		if (m_processFaces)
			return;
		StudentPanel().Children().Clear();

		auto button = sender.as<Windows::UI::Xaml::Controls::Button>();
		auto codeEvent = winrt::unbox_value<hstring>(button.Content());
		TRACE("Got click on code event " << codeEvent.c_str() << std::endl);
		auto foundActivity = std::find_if(m_activities.begin(), m_activities.end(), [&codeEvent](auto &activity) {
			return activity.codeEvent == winrt::to_string(codeEvent);
		});
		if (foundActivity != m_activities.end()) {
			StudentPanel().Visibility(Visibility::Visible);
			auto activity = *foundActivity;
			TRACE("Found corresponding activity for " << codeEvent.c_str() << " " << activity.moduleName.c_str() << std::endl);
			auto registeredStudents = co_await m_api.GetRegisteredStudentsAsync(activity);
			TRACE(registeredStudents.size() << " registered students for this activity:" << std::endl);
			m_studentsToCheck.clear();
			m_currentActivity = activity;
			for (auto student : registeredStudents) {
				TRACE("		" << student.title.c_str() << std::endl);
				auto button = Windows::UI::Xaml::Controls::Button();
				button.Content(winrt::box_value(winrt::to_hstring(student.title)));
				StudentPanel().Children().Append(std::move(button));
				m_studentsToCheck.push_back(student.login);
			}
			m_processFaces = true;
		}
		else {
			TRACE("No corresponding activity for " << codeEvent.c_str() << std::endl);
		}
		co_return;
	}

	IAsyncAction MainPage::ProcessActivitiesAsync()
	{
		std::scoped_lock lock(m_propertiesLock);
		ActivityPanel().Children().Clear();

		for (auto activity : m_activities)
		{
			auto button = Windows::UI::Xaml::Controls::Button();
			button.Content(winrt::box_value(winrt::to_hstring(activity.codeEvent)));
			button.Click(std::bind(&MainPage::ActivityHandler, this, std::placeholders::_1, std::placeholders::_2));
			ActivityPanel().Children().Append(std::move(button));
		}
		co_return;
	}

	IAsyncOperation<Capture::Frames::MediaFrameSource> MainPage::GetMediaSourceAsync()
	{
		IVectorView<Capture::Frames::MediaFrameSourceGroup> groups = co_await Capture::Frames::MediaFrameSourceGroup::FindAllAsync();

		Capture::Frames::MediaFrameSourceGroup selectedGroup = nullptr;
		Capture::Frames::MediaFrameSourceInfo selectedSourceInfo = nullptr;

		for (Capture::Frames::MediaFrameSourceGroup sourceGroup : groups)
		{
			for (Capture::Frames::MediaFrameSourceInfo sourceInfo : sourceGroup.SourceInfos())
			{
				if (sourceInfo.SourceKind() == Capture::Frames::MediaFrameSourceKind::Color)
				{
					selectedSourceInfo = sourceInfo;
					break;
				}
			}

			if (selectedSourceInfo != nullptr)
			{
				selectedGroup = sourceGroup;
				break;
			}
		}

		// No valid camera was found. This will happen on the emulator.
		if (selectedGroup == nullptr || selectedSourceInfo == nullptr)
		{
			throw runtime_error("No valid camera found !");
		}
		co_return m_mediaCapture.FrameSources().Lookup(selectedSourceInfo.Id());
	}

	IAsyncAction MainPage::InitializeCameraAsync()
	{
		auto settings = Windows::Media::Capture::MediaCaptureInitializationSettings();
		settings.StreamingCaptureMode(Windows::Media::Capture::StreamingCaptureMode::Video);
		settings.MemoryPreference(MediaCaptureMemoryPreference::Cpu); // Need SoftwareBitmaps for FaceAnalysis

		co_await m_mediaCapture.InitializeAsync(settings);

		Capture::Frames::MediaFrameSource mediaSource = co_await GetMediaSourceAsync();
		auto sourceType = Windows::Media::MediaProperties::MediaEncodingSubtypes::Bgra8();
		m_frameReader = co_await m_mediaCapture.CreateFrameReaderAsync(mediaSource,
			sourceType);

		m_frameReader.FrameArrived(std::bind(&MainPage::OnFrameArrived, this, std::placeholders::_1, std::placeholders::_2));
		auto ret = co_await m_frameReader.StartAsync();

		if (ret != MediaFrameReaderStartStatus::Success)
			throw runtime_error("Couldn't launch frame reader");

		co_await StartPreviewAsync();
		//co_await CreateFaceDetectionEffectAsync();
	}

	IAsyncAction MainPage::StartPreviewAsync()
	{
		m_displayRequest.RequestActive();

		PreviewControl().Source(m_mediaCapture);

		co_await m_mediaCapture.StartPreviewAsync();
		m_previewProperties = m_mediaCapture.VideoDeviceController().GetMediaStreamProperties(Capture::MediaStreamType::VideoPreview);
	}
	
	void MainPage::OnFrameArrived(MediaFrameReader const &sender, MediaFrameArrivedEventArgs const &args)
	{
		static int frameCount = 0;
		
		m_propertiesLock.lock();
		if (!m_processFaces){
			m_propertiesLock.unlock();
			return;
		}
		m_propertiesLock.unlock();
		frameCount++;
		if (frameCount == 120)
		{
			MediaFrameReference frame = sender.TryAcquireLatestFrame();

			frameCount = 0;
			if (frame == nullptr) {
				return;
			}

			auto videoFrame = frame.VideoMediaFrame();
			if (videoFrame != nullptr){
				m_latestBitmap = videoFrame.SoftwareBitmap();
				if (m_latestBitmap != nullptr && m_faceBuffer.isProcessing() == false) {
					TRACE("Triggering process visages async" << std::endl);
					ProcessVisagesAsync(std::move(m_latestBitmap));
				}					
			}			
		}
	}

	IAsyncAction MainPage::ProcessVisagesAsync(winrt::Windows::Graphics::Imaging::SoftwareBitmap facesToFind)
	{
		std::lock_guard<std::shared_mutex> lock(m_propertiesLock);

		TRACE("Launching process on students to check" << std::endl);
		auto studentsFound = co_await m_faceBuffer.GetMatchingImagesAsync(std::move(facesToFind), m_studentsToCheck);
		TRACE("Found " << studentsFound.Size() << " present in the activity " << m_currentActivity.codeEvent.c_str() << std::endl);
		for (auto student : studentsFound) {
			TRACE("Found " << student.c_str() << " !" << std::endl);
		}
		if (studentsFound.Size() > 0)
			co_await m_api.MarkRegisteredStudentsAsync(m_currentActivity, studentsFound);
		m_processFaces = false;
		co_return;
	}


	IAsyncOperation<SoftwareBitmap> GetCroppedBitmapAsync(SoftwareBitmap softwareBitmap,
		unsigned int startPointX, unsigned int startPointY,
		unsigned int width, unsigned int height)
	{
		InMemoryRandomAccessStream stream;
		BitmapEncoder encoder = co_await BitmapEncoder::CreateAsync(BitmapEncoder::BmpEncoderId(), stream);

		encoder.SetSoftwareBitmap(softwareBitmap);

		auto bitmapBound = BitmapBounds();
		bitmapBound.X = startPointX;
		bitmapBound.Y = startPointY;
		bitmapBound.Width = width;
		bitmapBound.Height = height;

		encoder.BitmapTransform().Bounds(bitmapBound);

		co_await encoder.FlushAsync();

		BitmapDecoder decoder = co_await BitmapDecoder::CreateAsync(stream);

		auto res = co_await decoder.GetSoftwareBitmapAsync(softwareBitmap.BitmapPixelFormat(), softwareBitmap.BitmapAlphaMode());

		return res;
	}
}
