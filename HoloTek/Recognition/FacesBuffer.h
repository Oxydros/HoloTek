#pragma once

using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Graphics::Imaging;

namespace HoloTek {

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

	class FacesBuffer
	{
	public:
		FacesBuffer(std::wstring imagesFolder = L"\\Assets\\Images");
		~FacesBuffer();

		std::wstring const &GetFolder() const { return m_folder; }

		concurrency::task<void> InitializeAsync();

		winrt::Windows::Foundation::IAsyncOperation<IVector<winrt::hstring>> GetMatchingImagesAsync(winrt::Windows::Graphics::Imaging::SoftwareBitmap facesToFind);

		winrt::Windows::Foundation::IAsyncOperation<IVector<winrt::hstring>> GetMatchingImagesAsync(winrt::Windows::Graphics::Imaging::SoftwareBitmap facesToFind,
			std::vector<std::string> const &studentsToCheck);

		winrt::Windows::Foundation::IAsyncOperation<IVector<winrt::hstring>> GetMatchingImagesAsync(winrt::Windows::Graphics::Imaging::SoftwareBitmap facesToFind,
			std::vector<dlib::matrix<float, 0, 1>> const &refFacesDescriptors, std::vector<std::string> const &refFacesName);

		bool isInitialized() {
			std::lock_guard<std::shared_mutex> lock(m_propertiesLock);
			return m_loaded;
		}

		bool isProcessing() {
			std::lock_guard<std::shared_mutex> lock(m_propertiesLock);
			return m_processing;
		}

		void setInitialized(bool init = true) {
			std::lock_guard<std::shared_mutex> lock(m_propertiesLock);
			m_loaded = init;
		}

		void setProcessing(bool processing = true) {
			std::lock_guard<std::shared_mutex> lock(m_propertiesLock);
			m_processing = processing;
		}

	private:
		void InitializeDLib();
		concurrency::task<dlib::matrix<dlib::rgb_pixel>> getPixelMatrixAsync(winrt::Windows::Graphics::Imaging::SoftwareBitmap softwareBitmap);

	private:
		std::wstring const							m_folder;
		bool										m_loaded;
		bool										m_processing;

		dlib::shape_predictor						m_sp;
		anet_type									m_net;
		dlib::frontal_face_detector					m_detector;
		std::vector<dlib::matrix<float, 0, 1>>		m_refFacesDescriptors;
		std::vector<std::string>					m_refImagesNames;
		std::shared_mutex							m_propertiesLock;
	};
}

