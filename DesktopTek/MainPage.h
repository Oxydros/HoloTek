//
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "Common/FacesBuffer.h"
#include "Common/IntraAPI.h"

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
		std::future<void> OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const &e);
		IAsyncAction PlanningHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
		IAsyncAction ActivityHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

	private:
		IAsyncAction ProcessActivitiesAsync();
		IAsyncAction InitializeCameraAsync();
		IAsyncAction StartPreviewAsync();
		IAsyncAction ProcessVisagesAsync(winrt::Windows::Graphics::Imaging::SoftwareBitmap facesToFind);

		IAsyncOperation<Capture::Frames::MediaFrameSource> GetMediaSourceAsync();
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
		std::vector<IntraAPI::Activity>				m_activities;

		std::vector<std::string>					m_studentsToCheck;

		bool										m_processFaces;
		FacesBuffer									m_faceBuffer;
		IntraAPI::Activity							m_currentActivity;
		IntraAPI									m_api;
    };
}

namespace winrt::DesktopTek::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
