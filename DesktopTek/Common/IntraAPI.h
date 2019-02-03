#pragma once

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Web;
using namespace Windows::Web::Http;

namespace winrt::DesktopTek::implementation
{
	class IntraAPI
	{
	public:
		struct Activity
		{
			std::string scholarYear;
			std::string codeModule;
			std::string codeInstance;
			std::string codeActi;
			std::string codeEvent;
			std::string moduleName;
		};

		struct Student
		{
			std::string login;
			std::string title;
			std::string picture;
		};

	public:
		IntraAPI(std::string autoLoginURL);
		~IntraAPI();

		IAsyncOperation<HttpStatusCode> LoginAsync();
		concurrency::task<std::vector<Activity>> GetActivitiesAsync();
		concurrency::task<std::vector<Student>> GetRegisteredStudentsAsync(Activity const &activity);
		IAsyncAction MarkRegisteredStudentsAsync(Activity activity,
			winrt::Windows::Foundation::Collections::IVector<winrt::hstring> logins);
	private:
		std::string								m_autoLogin;
		winrt::Windows::Web::Http::HttpClient	m_client;
	};
}
