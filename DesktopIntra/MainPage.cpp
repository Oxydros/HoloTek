#include "pch.h"
#include "MainPage.h"

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::Foundation;

namespace winrt::DesktopIntra::implementation
{
    MainPage::MainPage() : m_api("XXXXXXXXX")
    {
        InitializeComponent();
		PlanningButton().Visibility(Visibility::Collapsed);
		StudentPanel().Visibility(Visibility::Collapsed);
    }

    IAsyncAction MainPage::LoginHandler(IInspectable const&, RoutedEventArgs const&)
    {
		auto respoCode = co_await m_api.LoginAsync();
		if (respoCode != winrt::Windows::Web::Http::HttpStatusCode::Ok) {
			TRACE("Receive error when trying to log in");
			std::runtime_error("Couldn't logged into the intra");
		}
		PlanningButton().Visibility(Visibility::Visible);
    }

	IAsyncAction MainPage::PlanningHandler(IInspectable const&, RoutedEventArgs const&)
	{
		m_propertiesLock.lock();
		m_activities = co_await m_api.GetActivitiesAsync();
		m_propertiesLock.unlock();
		ProcessActivitiesAsync();
	}

	IAsyncAction MainPage::StudentHandler(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & args)
	{
		std::scoped_lock lock(m_propertiesLock);

		auto button = sender.as<Windows::UI::Xaml::Controls::Button>();
		auto studentLogin = winrt::unbox_value<hstring>(button.Content());
		TRACE("Got click on student " << studentLogin.c_str() << " for activity "
			<< m_currentActivity.codeActi.c_str() << std::endl);
		std::vector<std::string> students;

		students.push_back(winrt::to_string(studentLogin));
		co_await m_api.MarkRegisteredStudentsAsync(m_currentActivity, students);
		co_return;
	}

	IAsyncAction MainPage::ActivityHandler(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & args)
	{
		std::scoped_lock lock(m_propertiesLock);

		StudentPanel().Children().Clear();

		auto button = sender.as<Windows::UI::Xaml::Controls::Button>();
		auto codeEvent = winrt::unbox_value<hstring>(button.Content());
		TRACE("Got click on code event " << codeEvent.c_str() << std::endl);
		auto foundActivity = std::find_if(m_activities.begin(), m_activities.end(), [&codeEvent](auto &activity) {
			return activity.codeEvent == winrt::to_string(codeEvent);
		});
		if (foundActivity != m_activities.end()) {
			StudentPanel().Visibility(Visibility::Visible);
			auto activity = *foundActivity;
			m_currentActivity = activity;
			TRACE("Found corresponding activity for " << codeEvent.c_str() << " " << activity.moduleName.c_str() << std::endl);
			auto registeredStudents = co_await m_api.GetRegisteredStudentsAsync(activity);
			TRACE(registeredStudents.size() << " registered students for this activity:" << std::endl);
			for (auto student : registeredStudents) {
				TRACE("		" << student.title.c_str() << std::endl);
				auto button = Windows::UI::Xaml::Controls::Button();
				button.Content(winrt::box_value(winrt::to_hstring(student.login)));
				button.Click(std::bind(&MainPage::StudentHandler, this, std::placeholders::_1, std::placeholders::_2));
				StudentPanel().Children().Append(std::move(button));
			}
		}
		else {
			TRACE("No corresponding activity for " << codeEvent.c_str() << std::endl);
		}
		co_return;
	}

	IAsyncAction MainPage::ProcessActivitiesAsync()
	{
		std::scoped_lock lock(m_propertiesLock);
		ActivityPanel().Children().Clear();

		for (auto activity : m_activities)
		{
			auto button = Windows::UI::Xaml::Controls::Button();
			button.Content(winrt::box_value(winrt::to_hstring(activity.codeEvent)));
			button.Click(std::bind(&MainPage::ActivityHandler, this, std::placeholders::_1, std::placeholders::_2));
			ActivityPanel().Children().Append(std::move(button));
		}
		co_return;
	}
}
