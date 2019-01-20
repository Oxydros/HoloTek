//
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace winrt::DesktopTek::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();
		void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const &e);

	private:
		winrt::Windows::Foundation::IAsyncAction MainPage::InitializeCameraAsync();
		winrt::Windows::Foundation::IAsyncAction MainPage::StartPreviewAsync();
		winrt::Windows::Foundation::IAsyncAction MainPage::CreateFaceDetectionEffectAsync();

		winrt::Windows::Foundation::IAsyncAction HighlightDetectedFacesAsync(Windows::Foundation::Collections::IVectorView<winrt::Windows::Media::FaceAnalysis::DetectedFace> faces);

		void TriggerFaceDetected(Windows::Media::Core::FaceDetectionEffect const &sender,
			Windows::Media::Core::FaceDetectedEventArgs const &args);

		winrt::Windows::UI::Xaml::Shapes::Rectangle MainPage::ConvertPreviewToUiRectangle(winrt::Windows::Graphics::Imaging::BitmapBounds faceBoxInPreviewCoordinates);
		Windows::Foundation::Rect MainPage::GetPreviewStreamRectInControl(winrt::Windows::Media::MediaProperties::VideoEncodingProperties &previewResolution,
			winrt::Windows::UI::Xaml::Controls::CaptureElement previewControl);
		void SetFacesCanvasRotation();
		void MainPage::WriteLine(winrt::hstring str);


		//Camera device interraction
		Windows::Media::Capture::MediaCapture m_mediaCapture;
		Windows::Media::Capture::MediaCaptureInitializationSettings m_mediaSettings;
		Windows::Media::MediaProperties::IMediaEncodingProperties m_previewProperties = nullptr;

		//Face detection
		Windows::Media::Core::FaceDetectionEffect	m_faceDetectionEffet = nullptr;
		
		//Prevent screen extinction while camera is running
		Windows::System::Display::DisplayRequest	m_displayRequest;
		bool m_isPreviewing;

		winrt::event_token							m_faceDetectedEventToken;
    };
}

namespace winrt::DesktopTek::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
