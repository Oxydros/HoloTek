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
using namespace std;
using namespace dlib;

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
		deserialize("./Assets/shape_predictor_5_face_landmarks.dat") >> m_sp;
		deserialize("./Assets/dlib_face_recognition_resnet_model_v1.dat") >> m_net;

		m_detector = get_frontal_face_detector();
	}

	void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr)
	{
		// Get the first occurrence
		size_t pos = data.find(toSearch);

		// Repeat till end is reached
		while (pos != std::string::npos)
		{
			// Replace this occurrence of Sub String
			data.replace(pos, toSearch.size(), replaceStr);
			// Get the next occurrence from the current position
			pos = data.find(toSearch, pos + replaceStr.size());
		}
	}

	IAsyncAction FacesBuffer::InitializeFaces()
	{
		std::wstring path{ Windows::ApplicationModel::Package::Current().InstalledLocation().Path() + m_folder };
		auto storage = co_await Windows::Storage::StorageFolder::GetFolderFromPathAsync(path);
		auto files = co_await storage.GetFilesAsync();

		for (auto f : files)
		{
			std::wstring imagePath{ path + L"\\" + f.Name() };
			std::string imageString(imagePath.begin(), imagePath.end());

			matrix<rgb_pixel> img;
			TRACE("Loading " << imageString.c_str() << std::endl);
			load_image(img, imageString);

			auto faces = m_detector(img);

			TRACE("Found " << faces.size() << " in " << imageString.c_str() << std::endl);

			for (auto face : faces)
			{
				auto shape = m_sp(img, face);
				auto hName = f.Name();
				string imageName = std::string(hName.begin(), hName.end());
				findAndReplaceAll(imageName, "_", "-");
				findAndReplaceAll(imageName, ".jpg", "@epitech.eu");
				TRACE("New name went from " << hName.c_str() << " to " << imageName.c_str() << std::endl);

				matrix<rgb_pixel> face_chip;
				extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
				m_faces.push_back(move(face_chip));
				m_imagesNames.push_back(imageName);
			}
		}
		TRACE("Loaded " << m_faces.size() << " face(s) from " << m_imagesNames.size() << " file(s)" << std::endl);
	}

	concurrency::task<void> FacesBuffer::InitializeAsync()
	{
		InitializeDLib();
		co_await InitializeFaces();
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
			std::vector<matrix<rgb_pixel>> toFindFaces;
			for (auto face : m_detector(img))
			{
				auto shape = m_sp(img, face);
				matrix<rgb_pixel> face_chip;
				extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
				toFindFaces.push_back(move(face_chip));
			}

			if (toFindFaces.size() == 0) {
				TRACE("No faces found !" << std::endl);
				setProcessing(false);
				co_return winrt::single_threaded_vector(std::move(foundImages));
			}

			TRACE("Got " << std::to_wstring(toFindFaces.size()) << " faces to compare with ref" << std::endl);

			//Only checking against ref that needs to be
			std::vector<matrix<rgb_pixel>> refFaces;
			std::vector<std::string> refFacesName;

			for (auto studentToFind : studentsToCheck) {
				auto foundRef = std::find_if(m_imagesNames.begin(), m_imagesNames.end(), [&studentToFind](std::string const &imageName) {
					return studentToFind == imageName;
				});

				if (foundRef != m_imagesNames.end()) {
					auto idx = std::distance(m_imagesNames.begin(), foundRef);
					refFaces.push_back(m_faces[idx]);
					refFacesName.push_back(studentToFind);
				}
			}

			for (auto ref : refFacesName) {
				TRACE("Will check against ref " << ref.c_str() << std::endl);
			}
			TRACE("Size of ref to check is " << refFaces.size() << std::endl);

			//Compare all the faces to each references
			for (size_t i = 0; i < refFaces.size(); i++) {
				std::vector<matrix<rgb_pixel>> facesToProcess;
				facesToProcess.reserve(toFindFaces.size() + 1);

				auto toProcess = refFaces[i];
				auto toProcessName = refFacesName[i];

				TRACE("Processing with image " << toProcessName.c_str() << std::endl);

				facesToProcess.push_back(toProcess);
				facesToProcess.insert(facesToProcess.end(), toFindFaces.begin(), toFindFaces.end());

				TRACE(std::to_wstring(facesToProcess.size()) << " faces to process" << std::endl);

				//http://dlib.net/dnn_face_recognition_ex.cpp.html
				std::vector<matrix<float, 0, 1>> face_descriptors = m_net(facesToProcess);
				std::vector<sample_pair> edges;
				for (size_t i = 0; i < face_descriptors.size(); ++i)
				{
					for (size_t j = i; j < face_descriptors.size(); ++j)
					{
						if (length(face_descriptors[i] - face_descriptors[j]) < 0.6)
							edges.push_back(sample_pair(i, j));
					}
				}
				if (edges.size() < 2)
					continue;

				std::vector<unsigned long> labels;
				chinese_whispers(edges, labels, 50);

				//Only take labels found > 1 as there is at least 1 for the reference, and 1 for the current
				//face on stream
				std::vector<unsigned int> finalLabels;
				finalLabels.resize(refFaces.size());

				//Aggregate all labels found on the reference
				for (unsigned int i = 0; i < labels.size(); i++) {
					auto label = labels[i];

					if (label < finalLabels.size())
						finalLabels[label] += 1;
				}

				for (unsigned int idx = 0; idx < finalLabels.size(); idx++)
				{
					auto numOfLabel = finalLabels[idx];

					//If we found more than one time the same image, it means that an image on the stream
					//is keep as reference, or that there is two times the same face on stream
					if (numOfLabel > 1) {

						//Only process if it's in the reference images
						//If we have siblings on stream and not in reference, this condition will not hold
						if (idx < 1) {
							foundImages.push_back(winrt::to_hstring(refFacesName[i].c_str()));
						}
					}
				}
			}
			TRACE("number of people found in the image: " << foundImages.size() << endl);

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
			throw runtime_error("DLib Crashed !");
		}
	}

	concurrency::task<matrix<rgb_pixel>> FacesBuffer::getPixelMatrixAsync(SoftwareBitmap softwareBitmap)
	{
		matrix<rgb_pixel> img;
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
