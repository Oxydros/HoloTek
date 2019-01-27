#include "pch.h"
#include "MainPage.h"

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::Foundation;

namespace winrt::DesktopIntra::implementation
{
    MainPage::MainPage() : m_api("XXXXXXXXXXXXXXXXXXXXXXXX")
    {
        InitializeComponent();
		PlanningButton().Visibility(Visibility::Collapsed);
    }

    IAsyncAction MainPage::LoginHandler(IInspectable const&, RoutedEventArgs const&)
    {
		auto respoCode = co_await m_api.LoginAsync();
		PlanningButton().Visibility(Visibility::Visible);
    }

	IAsyncAction MainPage::PlanningHandler(IInspectable const&, RoutedEventArgs const&)
	{
		m_propertiesLock.lock();
		m_activities = co_await m_api.GetActivitiesAsync();
		m_propertiesLock.unlock();
		ProcessActivitiesAsync();
	}

	IAsyncAction MainPage::ActivityHandler(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & args)
	{
		std::scoped_lock lock(m_propertiesLock);

		auto button = sender.as<Windows::UI::Xaml::Controls::Button>();
		auto codeEvent = winrt::unbox_value<hstring>(button.Content());
		DBOUT("Got click on code event " << codeEvent.c_str() << std::endl);
		auto foundActivity = std::find_if(m_activities.begin(), m_activities.end(), [&codeEvent](auto &activity) {
			return activity.codeEvent == winrt::to_string(codeEvent);
		});
		if (foundActivity != m_activities.end()) {
			auto activity = *foundActivity;
			DBOUT("Found corresponding activity for " << codeEvent.c_str() << " " << activity.moduleName.c_str() << std::endl);
			auto registeredStudents = co_await m_api.GetRegisteredStudentsAsync(activity);
			DBOUT(registeredStudents.size() << " registered students for this activity:" << std::endl);
			for (auto student : registeredStudents) {
				DBOUT("		" << student.title.c_str() << std::endl);
			}
		}
		else {
			DBOUT("No corresponding activity for " << codeEvent.c_str() << std::endl);
		}
		co_return;
	}

	IAsyncAction MainPage::ProcessActivitiesAsync()
	{
		std::scoped_lock lock(m_propertiesLock);
		Panel().Children().Clear();

		for (auto activity : m_activities)
		{
			auto button = Windows::UI::Xaml::Controls::Button();
			button.Content(winrt::box_value(winrt::to_hstring(activity.codeEvent)));
			button.Click(std::bind(&MainPage::ActivityHandler, this, std::placeholders::_1, std::placeholders::_2));
			Panel().Children().Append(std::move(button));
		}
		co_return;
	}
}
