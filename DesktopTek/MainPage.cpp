#include "pch.h"
#include "Common/FaceTrackerProcessor.h"
#include "Common/VideoFrameProcessor.h"
#include "MainPage.h"

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace std;

namespace winrt::DesktopTek::implementation
{
	MainPage::MainPage()
	{
		InitializeComponent();
	}

	std::future<void> MainPage::InitializeAsync()
	{
		m_videoFrameProcessor = co_await VideoFrameProcessor::CreateAsync();
		m_faceTrackerProcessor = co_await FaceTrackerProcessor::CreateAsync(m_videoFrameProcessor);
	}

	winrt::Windows::Foundation::IAsyncAction MainPage::ClickHandler(IInspectable const&, RoutedEventArgs const&)
	{
		InitializeAsync();
		co_return;
		////myButton().Content(box_value(L"Clicked"));

		//m_mediaSettings.StreamingCaptureMode(Windows::Media::Capture::StreamingCaptureMode::Video);
		//co_await m_mediaCapture.InitializeAsync(m_mediaSettings);

		//m_displayRequest.RequestActive();

		//PreviewControl().Source(m_mediaCapture);
		//co_await m_mediaCapture.StartPreviewAsync();

		//co_return;
	}
}
