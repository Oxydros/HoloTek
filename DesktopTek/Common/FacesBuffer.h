#pragma once

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Graphics::Imaging;
using namespace dlib;

namespace winrt::DesktopTek::implementation {

	template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
	using residual = add_prev1<block<N, BN, 1, tag1<SUBNET>>>;

	template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
	using residual_down = add_prev2<avg_pool<2, 2, 2, 2, skip1<tag2<block<N, BN, 2, tag1<SUBNET>>>>>>;

	template <int N, template <typename> class BN, int stride, typename SUBNET>
	using block = BN<con<N, 3, 3, 1, 1, relu<BN<con<N, 3, 3, stride, stride, SUBNET>>>>>;

	template <int N, typename SUBNET> using ares = relu<residual<block, N, affine, SUBNET>>;
	template <int N, typename SUBNET> using ares_down = relu<residual_down<block, N, affine, SUBNET>>;

	template <typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
	template <typename SUBNET> using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
	template <typename SUBNET> using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
	template <typename SUBNET> using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
	template <typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

	using anet_type = loss_metric<fc_no_bias<128, avg_pool_everything<
		alevel0<
		alevel1<
		alevel2<
		alevel3<
		alevel4<
		max_pool<3, 3, 2, 2, relu<affine<con<32, 7, 7, 2, 2,
		input_rgb_image_sized<150>
		>>>>>>>>>>>>;

	class FacesBuffer
	{
	public:
		FacesBuffer(std::wstring imagesFolder = L"\\Assets\\Images");
		~FacesBuffer();

		std::wstring const &GetFolder() const { return m_folder; }

		concurrency::task<void> InitializeAsync();

		IAsyncOperation<IVector<winrt::hstring>> GetMatchingImagesAsync(SoftwareBitmap facesToFind);

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
		Windows::Foundation::IAsyncAction InitializeFaces();
		concurrency::task<dlib::matrix<dlib::rgb_pixel>> getPixelMatrixAsync(Windows::Graphics::Imaging::SoftwareBitmap softwareBitmap);

	private:
		std::wstring const							m_folder;
		bool										m_loaded;
		bool										m_processing;

		dlib::shape_predictor						m_sp;
		anet_type									m_net;
		frontal_face_detector						m_detector;
		std::vector<matrix<rgb_pixel>>				m_faces;
		std::vector<std::wstring>					m_imagesNames;
		std::shared_mutex							m_propertiesLock;
	};
}

