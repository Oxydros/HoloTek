#include "pch.h"
#include "MainPage.h"

#include <dlib/gui_widgets.h>

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
	MainPage::MainPage() : m_frameReader(nullptr), m_latestBitmap(nullptr)
	{
		InitializeComponent();
	}

	void MainPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const &e)
	{
		m_faceBuffer.InitializeAsync();
		InitializeCameraAsync();
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
	
	void MainPage::WriteLine(winrt::hstring str)
	{
		//#ifdef _DEBUG
		OutputDebugString(str.c_str());
		//#endif
	}

	void MainPage::OnFrameArrived(MediaFrameReader const &sender, MediaFrameArrivedEventArgs const &args)
	{
		static int frameCount = 0;
		
		frameCount++;
		if (frameCount == 60)
		{
			frameCount = 0;
			MediaFrameReference frame = sender.TryAcquireLatestFrame();
			if (frame == nullptr)
				return;
			std::lock_guard<std::shared_mutex> lock(m_propertiesLock);
			auto videoFrame = frame.VideoMediaFrame();
			if (videoFrame != nullptr){
				m_latestBitmap = videoFrame.SoftwareBitmap();
				if (m_latestBitmap != nullptr && m_faceBuffer.isProcessing() == false)
					m_faceBuffer.GetMatchingImagesAsync(std::move(m_latestBitmap));
			}			
		}
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
