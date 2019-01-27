//
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "IntraAPI.h"

namespace winrt::DesktopIntra::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

		IAsyncAction LoginHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
		IAsyncAction PlanningHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
		IAsyncAction ActivityHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

	private:
		IAsyncAction ProcessActivitiesAsync();

		IntraAPI						m_api;
		std::vector<IntraAPI::Activity>	m_activities;
		std::mutex						m_propertiesLock;
    };
}

namespace winrt::DesktopIntra::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
