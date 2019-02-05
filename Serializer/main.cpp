#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>

#include <dlib\dnn.h>
#include <dlib\clustering.h>
#include <dlib\string.h>
#include <dlib\image_io.h>
#include <dlib\image_processing\frontal_face_detector.h>

template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = dlib::add_prev1<block<N, BN, 1, dlib::tag1<SUBNET>>>;

template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = dlib::add_prev2<dlib::avg_pool<2, 2, 2, 2, dlib::skip1<dlib::tag2<block<N, BN, 2, dlib::tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block = BN<dlib::con<N, 3, 3, 1, 1, dlib::relu<BN<dlib::con<N, 3, 3, stride, stride, SUBNET>>>>>;

template <int N, typename SUBNET> using ares = dlib::relu<residual<block, N, dlib::affine, SUBNET>>;
template <int N, typename SUBNET> using ares_down = dlib::relu<residual_down<block, N, dlib::affine, SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
template <typename SUBNET> using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

using anet_type = dlib::loss_metric< dlib::fc_no_bias<128, dlib::avg_pool_everything<
	alevel0<
	alevel1<
	alevel2<
	alevel3<
	alevel4<
	dlib::max_pool<3, 3, 2, 2, dlib::relu<dlib::affine<dlib::con<32, 7, 7, 2, 2,
	dlib::input_rgb_image_sized<150>
	>>>>>>>>>>>>;

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

int main(int argc, char **argv)
{
	std::string path = argv[1];

	dlib::shape_predictor						m_sp;
	anet_type									m_net;
	dlib::frontal_face_detector					m_detector;

	std::vector<dlib::matrix<dlib::rgb_pixel>>	m_refFaces;
	std::vector<dlib::matrix<float, 0, 1>>		m_refFacesDescriptors;
	std::vector<std::string>					m_refImagesNames;

	dlib::deserialize("./refFaces_data.dat") >> m_refFacesDescriptors;

	std::cout << "Got " << m_refFacesDescriptors.size() << " descriptors" << std::endl;

	return 0;

	std::cout << "Init..." << std::endl;

	dlib::deserialize("./shape_predictor_5_face_landmarks.dat") >> m_sp;
	dlib::deserialize("./dlib_face_recognition_resnet_model_v1.dat") >> m_net;

	m_detector = dlib::get_frontal_face_detector();

	std::cout << "Success" << std::endl;

	for (const auto & entry : std::filesystem::directory_iterator(path)) {
		if (!entry.is_regular_file())
			continue;

		if (entry.path().extension() != ".jpg")
			continue;

		auto imageName = entry.path().filename().string();

		dlib::matrix<dlib::rgb_pixel> img;
		dlib::load_image(img, entry.path().string());

		auto faces = m_detector(img);

		std::cout << "Found " << faces.size() << " in " << imageName << std::endl;

		for (auto face : faces)
		{
			auto shape = m_sp(img, face);
			std::string newName = std::string(imageName.begin(), imageName.end());
			findAndReplaceAll(newName, "_", "-");
			findAndReplaceAll(newName, ".jpg", "@epitech.eu");
			std::cout << "New name went from " << imageName << " to " << newName << std::endl;

			dlib::matrix<dlib::rgb_pixel> face_chip;
			extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
			m_refFaces.push_back(std::move(face_chip));
			m_refImagesNames.push_back(newName);
		}

	}
	std::cout << "Loaded " << m_refFaces.size() << " face(s) from " << m_refImagesNames.size() << " file(s)" << std::endl;

	std::cout << "Serialize labels" << std::endl;
	std::ofstream outputLabels;
	outputLabels.open("./refFaces_labels.dat");
	dlib::serialize(m_refImagesNames, outputLabels);
	outputLabels.flush();
	outputLabels.close();

	std::cout << "Calculating descriptors" << std::endl;
	m_refFacesDescriptors = m_net(m_refFaces);

	std::cout << "Serialize descriptors" << std::endl;
	std::ofstream outputData;
	outputData.open("./refFaces_data.dat");
	outputData.close();

	dlib::serialize("./refFaces_data.dat") << m_refFacesDescriptors;
	/*outputData.flush();
	outputData.close();*/

	std::cout << "Success !" << std::endl;

	/*std::ifstream inputData;
	inputData.open("./refFaces_data.dat");*/
	dlib::deserialize("./refFaces_data.dat") >> m_refFacesDescriptors;

	std::cout << "Got " << m_refFacesDescriptors.size() << " descriptors" << std::endl;

	return 0;
}