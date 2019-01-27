//
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "Common/FacesBuffer.h"

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
		IAsyncAction InitializeCameraAsync();
		IAsyncAction StartPreviewAsync();

		IAsyncOperation<Capture::Frames::MediaFrameSource> GetMediaSourceAsync();
		void WriteLine(winrt::hstring str);
		void OnFrameArrived(MediaFrameReader const &sender, MediaFrameArrivedEventArgs const &args);
		
		//Camera device interraction
		Windows::Media::Capture::MediaCapture m_mediaCapture;
		Windows::Media::Capture::MediaCaptureInitializationSettings m_mediaSettings;
		Windows::Media::Capture::Frames::MediaFrameReader m_frameReader;
		Windows::Media::MediaProperties::IMediaEncodingProperties m_previewProperties = nullptr;
				
		//Prevent screen extinction while camera is running
		Windows::System::Display::DisplayRequest	m_displayRequest;
		bool m_isPreviewing;
		std::shared_mutex							m_propertiesLock;
		Windows::Graphics::Imaging::SoftwareBitmap	m_latestBitmap = nullptr;

		FacesBuffer									m_faceBuffer;
    };
}

namespace winrt::DesktopTek::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
