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

	void MainPage::TriggerFaceDetected(Windows::Media::Core::FaceDetectionEffect const &sender,
		Windows::Media::Core::FaceDetectedEventArgs const &args) {

		HighlightDetectedFacesAsync(args.ResultFrame().DetectedFaces());
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

	IAsyncAction MainPage::processFace(SoftwareBitmap softwareBitmap,
		BitmapBounds const &face)
	{

		if (softwareBitmap == nullptr)
			return;

		SoftwareBitmap croppedBitmap = co_await GetCroppedBitmapAsync(softwareBitmap, face.X, face.Y,
			face.Width, face.Height);

		SoftwareBitmap displayableImage = SoftwareBitmap::Convert(croppedBitmap,
			BitmapPixelFormat::Bgra8,
			BitmapAlphaMode::Premultiplied);

	/*	m_faceBuffer.GetMatchingImage(displayableImage);*/
	}

	IAsyncAction MainPage::HighlightDetectedFacesAsync(Windows::Foundation::Collections::IVectorView<winrt::Windows::Media::FaceAnalysis::DetectedFace> faces)
	{
		//co_await winrt::resume_background();

		//if (faces.Size() == 0)
		//	co_return;

		//m_faceBuffer.GetMatchingImage(std::move(m_latestBitmap));
		//m_latestBitmap = nullptr;

		//co_await winrt::resume_foreground::resume_foreground(FacesCanvas().Dispatcher());

		// Remove any existing rectangles from previous events
		//FacesCanvas().Children().Clear();

		/*auto detectedFace = faces.GetAt(0);

		auto faceBoxInPreviewCoordinates = detectedFace.FaceBox();

		faceBoxInPreviewCoordinates.X -= 60;
		faceBoxInPreviewCoordinates.Y -= 100;
		faceBoxInPreviewCoordinates.Width += 120;
		faceBoxInPreviewCoordinates.Height += 160;

		faceBoxInPreviewCoordinates.X = std::max(faceBoxInPreviewCoordinates.X, 0u);
		faceBoxInPreviewCoordinates.Y = std::max(faceBoxInPreviewCoordinates.Y, 0u);*/

		/*co_await processFace(m_latestBitmap, faceBoxInPreviewCoordinates);*/

		//// Face coordinate units are preview resolution pixels, which can be a different scale from our display resolution, so a conversion may be necessary
		//Windows::UI::Xaml::Shapes::Rectangle faceBoundingBox = ConvertPreviewToUiRectangle(faceBoxInPreviewCoordinates);

		//// Set bounding box stroke properties
		//faceBoundingBox.StrokeThickness(10);

		//// Highlight the first face in the set
		//faceBoundingBox.Stroke(SolidColorBrush(Windows::UI::Colors::DeepSkyBlue()));

		//std::wstringstream str;
		//str << L"Got a new rectangle at position " << faceBoundingBox.Margin().Left << L" " << faceBoundingBox.Margin().Top
		//	<< " with size " << faceBoundingBox.Width() << " " << faceBoundingBox.Height() << std::endl;
		//WriteLine(str.str().c_str());

		//// Add grid to canvas containing all face UI objects
		//FacesCanvas().Children().Append(faceBoundingBox);

		//// Update the face detection bounding box canvas orientation
		//SetFacesCanvasRotation();

		//std::wstringstream str;
		//str << L"Got Canvas size " << FacesCanvas().Width() << L" " << FacesCanvas().Height()
		//	<< " with pos " << FacesCanvas().Margin().Left << " " << FacesCanvas().Margin().Top << std::endl;
		//WriteLine(str.str().c_str());
		co_return;
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
		auto previewStream = m_previewProperties.as<MediaProperties::VideoEncodingProperties>();
		auto previewArea = GetPreviewStreamRectInControl(previewStream, PreviewControl());
		FacesCanvas().Width(previewArea.Width);
		FacesCanvas().Height(previewArea.Height);

		Controls::Canvas::SetLeft(FacesCanvas(), previewArea.X);
		Controls::Canvas::SetTop(FacesCanvas(), previewArea.Y);
		FacesCanvas().FlowDirection(Windows::UI::Xaml::FlowDirection::LeftToRight);
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
}
