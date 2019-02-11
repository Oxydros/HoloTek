#include "pch.h"

#include "FacesBuffer.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Media;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Storage::Streams;

//Heavely based on http://dlib.net/dnn_face_recognition_ex.cpp.html
namespace HoloTek
{
	FacesBuffer::FacesBuffer(std::wstring imagesFolder) :
		m_folder(imagesFolder), m_loaded(false), m_processing(false)
	{
	}


	FacesBuffer::~FacesBuffer()
	{
	}

	//Fetch dat files here
	//http://dlib.net/files/shape_predictor_5_face_landmarks.dat.bz2
	//http://dlib.net/files/dlib_face_recognition_resnet_model_v1.dat.bz2
	void FacesBuffer::InitializeDLib()
	{
		TRACE("Init dlib" << std::endl);
		dlib::deserialize("./Assets/shape_predictor_5_face_landmarks.dat") >> m_sp;
		dlib::deserialize("./Assets/dlib_face_recognition_resnet_model_v1.dat") >> m_net;

		m_detector = dlib::get_frontal_face_detector();

		dlib::deserialize("./Assets/refFaces_labels.dat") >> m_refImagesNames;

		TRACE("Got " << m_refImagesNames.size() << " images names" << std::endl);

		dlib::deserialize("./Assets/refFaces_data.dat") >> m_refFacesDescriptors;

		TRACE("Got " << m_refFacesDescriptors.size() << " descriptors" << std::endl);

		for (auto a : m_refImagesNames) {
			TRACE(a.c_str() << std::endl);
		}
	}

	concurrency::task<void> FacesBuffer::InitializeAsync()
	{
		InitializeDLib();
		setInitialized();
		co_return;
	}

	IAsyncOperation<IVector<winrt::hstring>> FacesBuffer::GetMatchingImagesAsync(winrt::Windows::Graphics::Imaging::SoftwareBitmap facesToFind,
		std::vector<dlib::matrix<float, 0, 1>> const &refFacesDescriptors, std::vector<std::string> const &refFacesName)
	{
		std::vector<winrt::hstring> foundImages;

		//Have to protect for multi-thread
		//this function need to run one at a time
		if (!isInitialized() || isProcessing()) {
			TRACE("Cancel check faces " << !isInitialized() << " " << isProcessing() << std::endl);
			co_return winrt::single_threaded_vector(std::move(foundImages));
		}			

		setProcessing(true);
		//Run in background
		co_await winrt::resume_background();

		try
		{
			//Convert bitmap to dlib format
			auto img = co_await getPixelMatrixAsync(std::move(facesToFind));

			//Extract faces from the video frame
			std::vector<dlib::matrix<dlib::rgb_pixel>> toFindFaces;
			for (auto face : m_detector(img))
			{
				auto shape = m_sp(img, face);
				dlib::matrix<dlib::rgb_pixel> face_chip;
				auto details = dlib::get_face_chip_details(shape, 150, 0.25);
				dlib::extract_image_chip(img, details, face_chip);
				toFindFaces.push_back(std::move(face_chip));
			}

			if (toFindFaces.size() == 0) {
				TRACE("No faces found !" << std::endl);
				setProcessing(false);
				co_return winrt::single_threaded_vector(std::move(foundImages));
			}

			TRACE("Got " << std::to_wstring(toFindFaces.size()) << " faces to compare with ref" << std::endl);

			std::vector<dlib::matrix<float, 0, 1>> toFindFacesDescriptors = m_net(toFindFaces);
			for (auto ref : refFacesName) {
				TRACE("Will check against ref " << ref.c_str() << std::endl);
			}

			TRACE("Size of ref to check is " << refFacesDescriptors.size() << std::endl);

			for (size_t target = 0; target < toFindFacesDescriptors.size(); target++) {

				float smallestLength = 1.0f;
				int finalRef = -1;
				auto targetDesc = toFindFacesDescriptors[target];

				for (int ref = 0; ref < refFacesDescriptors.size(); ref++) {
					auto length = dlib::length(refFacesDescriptors[ref] - targetDesc);
					if (length < 0.5 && length < smallestLength) {
						smallestLength = length;
						finalRef = ref;
					}
				}
				if (finalRef >= 0) {
					TRACE("Name of ref found for target " << target << " is " << refFacesName[finalRef].c_str()
						<< " with confidence of " << smallestLength << std::endl);
					foundImages.push_back(winrt::impl::to_hstring(refFacesName[finalRef]));
				}
				else {
					TRACE("No face found for target " << target << std::endl);
				}
			}
			TRACE("number of people found in the image: " << foundImages.size() << std::endl);

			TRACE("Faces found:" << std::endl);
			for (auto &image : foundImages)
			{
				TRACE(image.c_str() << std::endl);
			}
			setProcessing(false);
			co_return winrt::single_threaded_vector(std::move(foundImages));
		}
		catch (dlib::fatal_error const &except)
		{
			TRACE("Dlib crashed: " << except.info.c_str() << std::endl);
			throw std::runtime_error("DLib Crashed !");
		}
	}

	IAsyncOperation<IVector<winrt::hstring>> FacesBuffer::GetMatchingImagesAsync(winrt::Windows::Graphics::Imaging::SoftwareBitmap facesToFind,
		std::vector<std::string> const &studentsToCheck)
	{
		//Only checking against ref that needs to be
		std::vector<dlib::matrix<float, 0, 1>> refFacesDescriptors;
		std::vector<std::string> refFacesName;

		for (auto studentToFind : studentsToCheck) {
			//TRACE("Trying to find " << studentToFind.c_str() << std::endl);
			auto foundRef = std::find_if(m_refImagesNames.begin(), m_refImagesNames.end(), [&studentToFind](std::string const &imageName) {
				//TRACE("Text " << imageName.c_str() << " is it the same as " << studentToFind.c_str() << " ? "
				//			<< (studentToFind == imageName ? "OUI" : "NON") << std::endl);
				return studentToFind == imageName;
			});

			if (foundRef != m_refImagesNames.end()) {
				auto idx = std::distance(m_refImagesNames.begin(), foundRef);
				refFacesDescriptors.push_back(m_refFacesDescriptors[idx]);
				refFacesName.push_back(studentToFind);
			}
		}
		auto ret = co_await GetMatchingImagesAsync(facesToFind, refFacesDescriptors, refFacesName);
		co_return ret;
	}

	IAsyncOperation<IVector<winrt::hstring>> FacesBuffer::GetMatchingImagesAsync(winrt::Windows::Graphics::Imaging::SoftwareBitmap facesToFind)
	{
		auto ret = co_await GetMatchingImagesAsync(facesToFind, m_refFacesDescriptors, m_refImagesNames);
		co_return ret;
	};

	concurrency::task<dlib::matrix<dlib::rgb_pixel>> FacesBuffer::getPixelMatrixAsync(winrt::Windows::Graphics::Imaging::SoftwareBitmap softwareBitmap)
	{
		dlib::matrix<dlib::rgb_pixel> img;
		InMemoryRandomAccessStream stream;

		BitmapEncoder encoder = co_await BitmapEncoder::CreateAsync(BitmapEncoder::BmpEncoderId(), stream);
		encoder.SetSoftwareBitmap(softwareBitmap);
		co_await encoder.FlushAsync();

		BitmapDecoder decoder = co_await BitmapDecoder::CreateAsync(stream);

		auto width = softwareBitmap.PixelWidth();
		auto height = softwareBitmap.PixelHeight();

		BitmapTransform transform;
		transform.ScaledWidth(width);
		transform.ScaledHeight(height);

		PixelDataProvider pixelData = co_await decoder.GetPixelDataAsync(
			BitmapPixelFormat::Bgra8,
			BitmapAlphaMode::Straight,
			transform,
			ExifOrientationMode::IgnoreExifOrientation,
			ColorManagementMode::DoNotColorManage
		);

		//with * heigh * 4 (B G R A)
		auto sourcePixels = pixelData.DetachPixelData();

		img.set_size(height, width);
		TRACE("Created img of size " << width << " " << height << std::endl);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				auto &pixel = img(y, x);

				auto posPixel = (y * (width * 4)) + (x * 4);
				auto b = sourcePixels[posPixel];
				auto g = sourcePixels[posPixel + 1];
				auto r = sourcePixels[posPixel + 2];
				auto a = sourcePixels[posPixel + 3];

				pixel.red = r;
				pixel.green = g;
				pixel.blue = b;
			}
		}
		softwareBitmap.Close();
		return img;
	}
}
