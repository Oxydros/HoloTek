#pragma once

namespace winrt::DesktopTek::implementation
{
	class IntraAPI
	{
	public:
		struct Activity
		{
			winrt::hstring scholarYear;
			winrt::hstring codeModule;
			winrt::hstring codeInstance;
			winrt::hstring codeActi;
			winrt::hstring codeEvent;
			winrt::hstring moduleName;
		};

		struct Student
		{
			winrt::hstring login;
			winrt::hstring title;
			winrt::hstring picture;
		};

	public:
		IntraAPI(std::string autoLoginURL);
		~IntraAPI();

		winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Web::Http::HttpStatusCode> LoginAsync() const;
		concurrency::task<std::vector<Activity>> GetActivitiesAsync() const;
		concurrency::task<std::vector<Student>> GetRegisteredStudentsAsync(Activity const &activity) const;
		winrt::Windows::Foundation::IAsyncAction MarkRegisteredStudentsAsync(Activity activity,
			winrt::Windows::Foundation::Collections::IVector<winrt::hstring> logins) const;
	private:
		std::string								m_autoLogin;
		winrt::Windows::Web::Http::HttpClient	m_client;
	};
}
