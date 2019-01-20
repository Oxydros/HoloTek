#include "pch.h"
#include "MainPage.h"

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::Foundation;
using namespace Windows::Media;
using namespace Windows::Graphics::Imaging;
using namespace std;

namespace winrt::DesktopTek::implementation
{
	MainPage::MainPage()
	{
		InitializeComponent();
	}

	void MainPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const &e)
	{
		InitializeCameraAsync();
	}


	IAsyncAction MainPage::InitializeCameraAsync()
	{
		auto settings = Windows::Media::Capture::MediaCaptureInitializationSettings();
		settings.StreamingCaptureMode(Windows::Media::Capture::StreamingCaptureMode::Video);

		co_await m_mediaCapture.InitializeAsync(settings);
		co_await StartPreviewAsync();
		co_await CreateFaceDetectionEffectAsync();
	}

	IAsyncAction MainPage::StartPreviewAsync()
	{
		m_displayRequest.RequestActive();

		PreviewControl().Source(m_mediaCapture);

		co_await m_mediaCapture.StartPreviewAsync();
		m_previewProperties = m_mediaCapture.VideoDeviceController().GetMediaStreamProperties(Capture::MediaStreamType::VideoPreview);
	}

	void MainPage::TriggerFaceDetected(Windows::Media::Core::FaceDetectionEffect const &sender,
		Windows::Media::Core::FaceDetectedEventArgs const &args) {

		HighlightDetectedFacesAsync(args.ResultFrame().DetectedFaces());
	}

	IAsyncAction MainPage::CreateFaceDetectionEffectAsync()
	{
		auto def = winrt::Windows::Media::Core::FaceDetectionEffectDefinition();

		def.SynchronousDetectionEnabled(false);
		def.DetectionMode(winrt::Windows::Media::Core::FaceDetectionMode::HighPerformance);

		auto ret = co_await m_mediaCapture.AddVideoEffectAsync(def, winrt::Windows::Media::Capture::MediaStreamType::VideoPreview);

		m_faceDetectionEffet = ret.as<winrt::Windows::Media::Core::FaceDetectionEffect>();

		m_faceDetectedEventToken = m_faceDetectionEffet.FaceDetected(std::bind(&MainPage::TriggerFaceDetected, this, std::placeholders::_1,
			std::placeholders::_2));

		Windows::Foundation::TimeSpan duration = Windows::Foundation::TimeSpan(330000);
		m_faceDetectionEffet.DesiredDetectionInterval(duration);

		m_faceDetectionEffet.Enabled(true);
	}

	IAsyncAction MainPage::HighlightDetectedFacesAsync(Windows::Foundation::Collections::IVectorView<winrt::Windows::Media::FaceAnalysis::DetectedFace> faces)
	{
		co_await winrt::resume_foreground::resume_foreground(FacesCanvas().Dispatcher());
		// Remove any existing rectangles from previous events
		FacesCanvas().Children().Clear();

		// For each detected face
		for (unsigned int i = 0; i < faces.Size(); i++)
		{
			// Face coordinate units are preview resolution pixels, which can be a different scale from our display resolution, so a conversion may be necessary
			Windows::UI::Xaml::Shapes::Rectangle faceBoundingBox = ConvertPreviewToUiRectangle(faces.GetAt(i).FaceBox());

			// Set bounding box stroke properties
			faceBoundingBox.StrokeThickness(10);

			// Highlight the first face in the set
			faceBoundingBox.Stroke((i == 0 ? SolidColorBrush(Windows::UI::Colors::Blue()) : SolidColorBrush(Windows::UI::Colors::DeepSkyBlue())));

			std::wstringstream str;
			str << L"Got a new rectangle at position " << faceBoundingBox.Margin().Left << L" " << faceBoundingBox.Margin().Top
				<< " with size " << faceBoundingBox.Width()<< " " << faceBoundingBox.Height() << std::endl;
			WriteLine(str.str().c_str());

			// Add grid to canvas containing all face UI objects
			FacesCanvas().Children().Append(faceBoundingBox);
		}

		// Update the face detection bounding box canvas orientation
		SetFacesCanvasRotation();

		std::wstringstream str;
		str << L"Got Canvas size " << FacesCanvas().Width() << L" " << FacesCanvas().Height()
			<< " with pos " << FacesCanvas().Margin().Left << " " << FacesCanvas().Margin().Top << std::endl;
		WriteLine(str.str().c_str());
	}

	Shapes::Rectangle MainPage::ConvertPreviewToUiRectangle(BitmapBounds faceBoxInPreviewCoordinates)
	{
		std::wstringstream str;
		str << L"Got a new face at position " << faceBoxInPreviewCoordinates.X << L" " << faceBoxInPreviewCoordinates.Y
			<< " with size " << faceBoxInPreviewCoordinates.Width << " " << faceBoxInPreviewCoordinates.Height << std::endl;
		WriteLine(str.str().c_str());
		auto result = Shapes::Rectangle();
		auto previewStream = m_previewProperties.as<MediaProperties::VideoEncodingProperties>();

		// If there is no available information about the preview, return an empty rectangle, as re-scaling to the screen coordinates will be impossible
		if (previewStream == nullptr)
		{
			return result;
		}

		// Similarly, if any of the dimensions is zero (which would only happen in an error case) return an empty rectangle
		if (previewStream.Width() == 0 || previewStream.Height() == 0)
		{
			return result;
		}

		double streamWidth = previewStream.Width();
		double streamHeight = previewStream.Height();

		auto previewInUI = GetPreviewStreamRectInControl(previewStream, PreviewControl());

		// Scale the width and height from preview stream coordinates to window coordinates
		result.Width((faceBoxInPreviewCoordinates.Width / streamWidth) * previewInUI.Width);
		result.Height((faceBoxInPreviewCoordinates.Height / streamHeight) * previewInUI.Height);

		// Scale the X and Y coordinates from preview stream coordinates to window coordinates
		auto x = (faceBoxInPreviewCoordinates.X / streamWidth) * previewInUI.Width;
		auto y = (faceBoxInPreviewCoordinates.Y / streamHeight) * previewInUI.Height;
		Controls::Canvas::SetLeft(result, x);
		Controls::Canvas::SetTop(result, y);

		return result;
	}

	Windows::Foundation::Rect MainPage::GetPreviewStreamRectInControl(MediaProperties::VideoEncodingProperties &previewResolution, Controls::CaptureElement previewControl)
	{
		// In case this function is called before everything is initialized correctly, return an empty result
		if (previewControl == nullptr || previewControl.ActualHeight() < 1 || previewControl.ActualWidth() < 1 ||
			previewResolution == nullptr || previewResolution.Height() == 0 || previewResolution.Width() == 0)
		{
			return RectHelper::FromCoordinatesAndDimensions(0, 0, 0, 0);
		}

		auto streamWidth = previewResolution.Width();
		auto streamHeight = previewResolution.Height();

		// Start by assuming the preview display area in the control spans the entire width and height both (this is corrected in the next if for the necessary dimension)
		auto width = previewControl.ActualWidth();
		auto height = previewControl.ActualHeight();
		double x = 0.0;
		double y = 0.0;

		// If UI is "wider" than preview, letterboxing will be on the sides
		if ((previewControl.ActualWidth() / previewControl.ActualHeight() > streamWidth / static_cast<double>(streamHeight)))
		{
			auto scale = previewControl.ActualHeight() / streamHeight;
			auto scaledWidth = streamWidth * scale;

			x = (previewControl.ActualWidth() - scaledWidth) / 2.0;
			width = scaledWidth;
		}
		else // Preview stream is "wider" than UI, so letterboxing will be on the top+bottom
		{
			auto scale = previewControl.ActualWidth() / streamWidth;
			auto scaledHeight = streamHeight * scale;

			y = static_cast<float>((previewControl.ActualHeight() - scaledHeight) / 2.0);
			height = scaledHeight;
		}

		return RectHelper::FromCoordinatesAndDimensions(
			static_cast<float>(x),
			static_cast<float>(y),
			static_cast<float>(width),
			static_cast<float>(height));
	}

	void MainPage::SetFacesCanvasRotation()
	{
		// Calculate how much to rotate the canvas
		int rotationDegrees = 0;

		// Apply the rotation
		//auto transform = RotateTransform();
		//transform.Angle(rotationDegrees);
		//FacesCanvas().RenderTransform(transform);

		auto previewStream = m_previewProperties.as<MediaProperties::VideoEncodingProperties>();
		auto previewArea = GetPreviewStreamRectInControl(previewStream, PreviewControl());

		//// For portrait mode orientations, swap the width and height of the canvas after the rotation, so the control continues to overlap the preview
		//if (_displayOrientation == DisplayOrientations::Portrait || _displayOrientation == DisplayOrientations::PortraitFlipped)
		//{
		//FacesCanvas().Width(previewArea.Height);
		//FacesCanvas().Height(previewArea.Width);

		//// The position of the canvas also needs to be adjusted, as the size adjustment affects the centering of the control
		//Controls::Canvas::SetLeft(FacesCanvas(), previewArea.X - (previewArea.Height - previewArea.Width) / 2);
		//Controls::Canvas::SetTop(FacesCanvas(), previewArea.Y - (previewArea.Width - previewArea.Height) / 2);
		//}
		//else
		//{
		FacesCanvas().Width(previewArea.Width);
		FacesCanvas().Height(previewArea.Height);

		Controls::Canvas::SetLeft(FacesCanvas(), previewArea.X);
		Controls::Canvas::SetTop(FacesCanvas(), previewArea.Y);
		//}

		// Also mirror the canvas if the preview is being mirrored
		/*FacesCanvas->FlowDirection = _mirroringPreview ? Windows::UI::Xaml::FlowDirection::RightToLeft : Windows::UI::Xaml::FlowDirection::LeftToRight;*/
		FacesCanvas().FlowDirection(Windows::UI::Xaml::FlowDirection::LeftToRight);
	}

	void MainPage::WriteLine(winrt::hstring str)
	{
		OutputDebugString(str.c_str());
	}
}
