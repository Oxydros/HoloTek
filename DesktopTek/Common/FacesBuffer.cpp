#include "pch.h"

#include "FacesBuffer.h"

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Media;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Storage::Streams;

//Heavely based on http://dlib.net/dnn_face_recognition_ex.cpp.html
namespace winrt::DesktopTek::implementation
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
	}

	concurrency::task<void> FacesBuffer::InitializeAsync()
	{
		InitializeDLib();
		setInitialized();
		co_return;
	}

	IAsyncOperation<IVector<winrt::hstring>> FacesBuffer::GetMatchingImagesAsync(Windows::Graphics::Imaging::SoftwareBitmap facesToFind,
		std::vector<std::string> const &studentsToCheck)
	{
		std::vector<winrt::hstring> foundImages;

		//Have to protect for multi-thread
		//this function need to run one at a time
		if (!isInitialized() || isProcessing())
			co_return winrt::single_threaded_vector(std::move(foundImages));

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
				extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
				toFindFaces.push_back(std::move(face_chip));
			}

			if (toFindFaces.size() == 0) {
				TRACE("No faces found !" << std::endl);
				setProcessing(false);
				co_return winrt::single_threaded_vector(std::move(foundImages));
			}

			TRACE("Got " << std::to_wstring(toFindFaces.size()) << " faces to compare with ref" << std::endl);

			std::vector<dlib::matrix<float, 0, 1>> toFindFacesDescriptors = m_net(toFindFaces);

			//Only checking against ref that needs to be
			std::vector<dlib::matrix<float, 0, 1>> refFacesDescriptors = m_refFacesDescriptors;
			std::vector<std::string> refFacesName = m_refImagesNames;

			//for (auto studentToFind : studentsToCheck) {
			//	auto foundRef = std::find_if(m_refImagesNames.begin(), m_refImagesNames.end(), [&studentToFind](std::string const &imageName) {
			//		return studentToFind == imageName;
			//	});

			//	if (foundRef != m_refImagesNames.end()) {
			//		auto idx = std::distance(m_refImagesNames.begin(), foundRef);
			//		refFacesDescriptors.push_back(m_refFacesDescriptors[idx]);
			//		refFacesName.push_back(studentToFind);
			//	}
			//}

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
					if (length < 0.6 && length < smallestLength) {
						smallestLength = length;
						finalRef = ref;
					}
				}
				if (finalRef >= 0) {
					TRACE("Name of ref found for target " << target << " is " << refFacesName[finalRef].c_str()
						<< " with confidence of " << smallestLength << std::endl);
					foundImages.push_back(winrt::to_hstring(refFacesName[finalRef]));
				}
				else {
					TRACE("No face found for target " << target << std::endl);
				}
			}

			//Compare all the faces to each references
			//for (size_t i = 0; i < toFindFacesDescriptors.size(); i++) {
			//	auto toProcessDescriptor = toFindFacesDescriptors[i];
			//	/*auto toProcessName = refFacesName[i];*/

			//	/*TRACE("Processing with image " << toProcessName.c_str() << std::endl);*/

			//	//http://dlib.net/dnn_face_recognition_ex.cpp.html
			//	std::vector<dlib::matrix<float, 0, 1>> face_descriptors;
			//	face_descriptors.reserve(refFacesDescriptors.size() + 1);
			//	face_descriptors.push_back(toProcessDescriptor);
			//	face_descriptors.insert(face_descriptors.end(), refFacesDescriptors.begin(), refFacesDescriptors.end());

			//	std::vector<dlib::sample_pair> edges;
			//	for (size_t descI = 0; descI < face_descriptors.size(); ++descI)
			//	{
			//		for (size_t descJ = descI; descJ < face_descriptors.size(); ++descJ)
			//		{
			//			auto length = dlib::length(face_descriptors[descI] - face_descriptors[descJ]);
			//			if (length < 0.6) {
			//				edges.push_back(dlib::sample_pair(descI, descJ));
			//			}							
			//		}
			//	}

			//	if (edges.size() < 2)
			//		continue;

			//	std::vector<unsigned long> labels;
			//	dlib::chinese_whispers(edges, labels);

			//	unsigned long targetLabel = labels[0];

			//	//Only take labels found > 1 as there is at least 1 for the reference, and 1 for the current
			//	//face on stream
			//	std::vector<unsigned long> finalLabels;
			//	finalLabels.reserve(labels.size());

			//	//Aggregate all labels found on the reference
			//	for (unsigned int i = 0; i < labels.size(); i++) {
			//		auto label = labels[i];
			//		finalLabels[label] += 1;
			//	}

			//	for (unsigned int idx = 0; idx < finalLabels.size(); idx++)
			//	{
			//		auto numOfLabel = finalLabels[idx];

			//		//If we found more than one time the same image, it means that an image on the stream
			//		//is keep as reference, or that there is two times the same face on stream
			//		if (numOfLabel > 1) {

			//			//Only process if it's in the reference images
			//			//If we have siblings on stream and not in reference, this condition will not hold
			//			if (idx >= 1) {
			//				foundImages.push_back(winrt::to_hstring(refFacesName[idx - 1].c_str()));
			//			}
			//		}
			//	}
			//}
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

	concurrency::task<dlib::matrix<dlib::rgb_pixel>> FacesBuffer::getPixelMatrixAsync(SoftwareBitmap softwareBitmap)
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
