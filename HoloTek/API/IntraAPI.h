#pragma once

namespace HoloTek
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

		winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Web::Http::HttpStatusCode> LoginAsync() const;
		concurrency::task<std::vector<Activity>> GetActivitiesAsync() const;
		concurrency::task<std::vector<Student>> GetRegisteredStudentsAsync(Activity const &activity) const;
	private:
		std::string								m_autoLogin;
		winrt::Windows::Web::Http::HttpClient	m_client;
	};
}
