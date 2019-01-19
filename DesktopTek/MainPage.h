//
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace winrt {
	namespace DesktopTek {
		class VideoFrameProcessor;
		class FaceTrackerProcessor;
	}

}
namespace winrt::DesktopTek::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

		winrt::Windows::Foundation::IAsyncAction ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

	private:
		std::future<void> MainPage::InitializeAsync();


		Windows::Media::Capture::MediaCapture m_mediaCapture;
		Windows::Media::Capture::MediaCaptureInitializationSettings m_mediaSettings;
		Windows::System::Display::DisplayRequest m_displayRequest;

		// Video and face tracking processors
		std::shared_ptr<DesktopTek::VideoFrameProcessor>                            m_videoFrameProcessor;
		std::shared_ptr<DesktopTek::FaceTrackerProcessor>                           m_faceTrackerProcessor;
		bool m_isPreviewing;
    };
}

namespace winrt::DesktopTek::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
