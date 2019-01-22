//
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

using namespace winrt;
using namespace Windows::Media;
using namespace Windows::Media::Capture;
using namespace Windows::Media::Capture::Frames;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

namespace winrt::DesktopTek::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();
		void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const &e);

	private:
		IAsyncAction MainPage::InitializeCameraAsync();
		IAsyncAction MainPage::StartPreviewAsync();
		IAsyncAction MainPage::CreateFaceDetectionEffectAsync();

		IAsyncAction HighlightDetectedFacesAsync(IVectorView<winrt::Windows::Media::FaceAnalysis::DetectedFace> faces);

		void TriggerFaceDetected(Windows::Media::Core::FaceDetectionEffect const &sender,
			Windows::Media::Core::FaceDetectedEventArgs const &args);

		winrt::Windows::UI::Xaml::Shapes::Rectangle MainPage::ConvertPreviewToUiRectangle(winrt::Windows::Graphics::Imaging::BitmapBounds faceBoxInPreviewCoordinates);
		Windows::Foundation::Rect MainPage::GetPreviewStreamRectInControl(winrt::Windows::Media::MediaProperties::VideoEncodingProperties &previewResolution,
			winrt::Windows::UI::Xaml::Controls::CaptureElement previewControl);
		IAsyncOperation<Capture::Frames::MediaFrameSource> GetMediaSourceAsync();
		void SetFacesCanvasRotation();
		void WriteLine(winrt::hstring str);
		void OnFrameArrived(MediaFrameReader const &sender, MediaFrameArrivedEventArgs const &args);
		IAsyncAction processFace(Capture::Frames::MediaFrameReference const &frame,
			Windows::Graphics::Imaging::BitmapBounds const &face);


		//Camera device interraction
		Windows::Media::Capture::MediaCapture m_mediaCapture;
		Windows::Media::Capture::MediaCaptureInitializationSettings m_mediaSettings;
		Windows::Media::Capture::Frames::MediaFrameReader m_frameReader;
		Windows::Media::MediaProperties::IMediaEncodingProperties m_previewProperties = nullptr;

		//Face detection
		Windows::Media::Core::FaceDetectionEffect	m_faceDetectionEffet = nullptr;
		
		//Prevent screen extinction while camera is running
		Windows::System::Display::DisplayRequest	m_displayRequest;
		bool m_isPreviewing;
		std::shared_mutex							m_propertiesLock;
		MediaFrameReference							m_latestFrame;

		winrt::event_token							m_faceDetectedEventToken;
    };
}

namespace winrt::DesktopTek::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
