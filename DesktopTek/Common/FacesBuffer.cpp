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

	IAsyncAction FacesBuffer::InitializeFaces()
	{
		std::wstring path{ Windows::ApplicationModel::Package::Current().InstalledLocation().Path() + m_folder };
		auto storage = co_await Windows::Storage::StorageFolder::GetFolderFromPathAsync(path);
		auto files = co_await storage.GetFilesAsync();

		std::wstringstream str;
		for (auto f : files)
		{
			std::wstring imagePath{ path + L"\\" + f.Name() };
			std::string imageString(imagePath.begin(), imagePath.end());

			matrix<rgb_pixel> img;
			str << "Loading " << imageString.c_str() << std::endl;
			load_image(img, imageString);

			auto faces = m_detector(img);

			str << "Found " << faces.size() << " in " << imageString.c_str() << std::endl;

			for (auto face : faces)
			{
				auto shape = m_sp(img, face);
				matrix<rgb_pixel> face_chip;
				extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
				m_faces.push_back(move(face_chip));
				m_imagesNames.push_back(f.Name().c_str());
			}
		}
		str << "Loaded " << m_faces.size() << " face(s) from " << m_imagesNames.size() << " file(s)" << std::endl;
		OutputDebugString(str.str().c_str());
	}

	concurrency::task<void> FacesBuffer::InitializeAsync()
	{
		InitializeDLib();
		co_await InitializeFaces();
		setInitialized();
		co_return;
	}

	IAsyncOperation<IVector<winrt::hstring>> FacesBuffer::GetMatchingImagesAsync(Windows::Graphics::Imaging::SoftwareBitmap facesToFind)
	{
		std::vector<winrt::hstring> foundImages;

		//Have to protect for multi-thread
		//this need to function one at a time
		if (!isInitialized() || isProcessing())
			co_return winrt::single_threaded_vector(std::move(foundImages));

		setProcessing(true);
		co_await winrt::resume_background();

		try
		{
			auto img = co_await getPixelMatrixAsync(std::move(facesToFind));

			std::vector<matrix<rgb_pixel>> toFindFaces;
			for (auto face : m_detector(img))
			{
				auto shape = m_sp(img, face);
				matrix<rgb_pixel> face_chip;
				extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
				toFindFaces.push_back(move(face_chip));
			}

			if (toFindFaces.size() == 0) {
				OutputDebugString(L"No faces found !\n");
				setProcessing(false);
				co_return winrt::single_threaded_vector(std::move(foundImages));
			}

			auto message = L"Found " + std::to_wstring(toFindFaces.size()) + L" faces\n";
			OutputDebugString(message.c_str());

			std::vector<matrix<rgb_pixel>> facesToProcess;
			facesToProcess.reserve(toFindFaces.size() + m_faces.size());
			facesToProcess.insert(facesToProcess.end(), m_faces.begin(), m_faces.end());
			facesToProcess.insert(facesToProcess.end(), toFindFaces.begin(), toFindFaces.end());

			auto message2 = std::to_wstring(facesToProcess.size()) + L" faces to process\n";
			OutputDebugString(message2.c_str());

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
			std::vector<unsigned long> labels;
			chinese_whispers(edges, labels);

			//Only take labels found > 1 as there is at least 1 for the reference, and 1 for the current
			//face on stream
			std::vector<unsigned int> finalLabels;
			finalLabels.resize(m_faces.size());

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
					if (idx < m_faces.size()) {
						foundImages.push_back(m_imagesNames[idx].c_str());
					}
				}
			}

			std::wstringstream strr;
			strr << "number of people found in the image: " << foundImages.size() << endl;
			OutputDebugString(strr.str().c_str());

			std::wstringstream foundList;
			foundList << "Faces found:" << std::endl;
			for (auto &image : foundImages)
			{
				foundList << image.c_str() << std::endl;
			}
			OutputDebugString(foundList.str().c_str());

			setProcessing(false);
			co_return winrt::single_threaded_vector(std::move(foundImages));
		}
		catch (dlib::fatal_error const &except)
		{
			std::wstringstream stream;

			stream << "Dlib crashed: " << except.info.c_str() << std::endl;
			OutputDebugString(stream.str().c_str());
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
