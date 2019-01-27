#include "pch.h"
#include "IntraAPI.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Web;
using namespace Windows::Web::Http;

namespace winrt::DesktopIntra::implementation
{
	IntraAPI::IntraAPI(std::string autoLoginURL) :
		m_autoLogin(autoLoginURL)
	{
	}

	IntraAPI::~IntraAPI()
	{
	}

	IAsyncOperation<HttpStatusCode> IntraAPI::LoginAsync()
	{
		auto headers = m_client.DefaultRequestHeaders();

		auto uri = winrt::Windows::Foundation::Uri(winrt::to_hstring(m_autoLogin));
		auto response = co_await m_client.GetAsync(uri);

		co_return response.StatusCode();
	}

	concurrency::task<std::vector<IntraAPI::Activity>> IntraAPI::GetActivitiesAsync()
	{
		return concurrency::create_task([&] {
			std::vector<IntraAPI::Activity> activities;

			//Get current date
			auto currentTime = std::time(nullptr);
			struct tm currentTimeStruct;
			localtime_s(&currentTimeStruct, &currentTime);
			std::stringstream uriStream;

			uriStream << "https://intra.epitech.eu/planning/load?format=json&start=" << std::put_time(&currentTimeStruct, "%Y-%m-%d")
				<< "&end=" << std::put_time(&currentTimeStruct, "%Y-%m-%d");

			auto uriString = uriStream.str();

			auto planningURI = Uri(winrt::to_hstring(uriString));
			auto response = m_client.GetAsync(planningURI).get();

			auto content = response.Content().ReadAsStringAsync().get();

			auto root = Windows::Data::Json::JsonValue::Parse(content);

			for (auto item : root.GetArray())
			{
				auto object = item.GetObject();
				auto scholarYear = object.GetNamedString(L"scolaryear");
				auto codeModule = object.GetNamedString(L"codemodule");
				auto codeInstance = object.GetNamedString(L"codeinstance");
				auto codeActi = object.GetNamedString(L"codeacti");
				auto codeEvent = object.GetNamedString(L"codeevent");
				auto moduleName = object.GetNamedString(L"titlemodule");
				auto newActivity = IntraAPI::Activity{
					winrt::to_string(scholarYear),
					winrt::to_string(codeModule),
					winrt::to_string(codeInstance),
					winrt::to_string(codeActi),
					winrt::to_string(codeEvent),
					winrt::to_string(moduleName)
				};
				activities.push_back(std::move(newActivity));
			}
			return activities;
		});
	}

	//https://intra.epitech.eu/module/2018/B-INN-000/TLS-0-1/acti-326701/event-322207/registered?format=json
	concurrency::task<std::vector<IntraAPI::Student>> IntraAPI::GetRegisteredStudentsAsync(Activity const &activity)
	{
		return concurrency::create_task([&] {
			std::vector<IntraAPI::Student> students;

			std::stringstream uriStream;

			uriStream << "https://intra.epitech.eu/module/"
				<< activity.scholarYear << "/"
				<< activity.codeModule.c_str() << "/"
				<< activity.codeInstance.c_str() << "/"
				<< activity.codeActi.c_str() << "/"
				<< activity.codeEvent.c_str() << "/registered?format=json";

			auto uriString = uriStream.str();

			auto registeredURI = Uri(winrt::to_hstring(uriString));
			auto response = m_client.GetAsync(registeredURI).get();

			auto content = response.Content().ReadAsStringAsync().get();

			auto root = Windows::Data::Json::JsonValue::Parse(content);

			for (auto item : root.GetArray())
			{
				auto object = item.GetObject();
				auto login = object.GetNamedString(L"login");
				auto title = object.GetNamedString(L"title");
				auto picture = object.GetNamedString(L"picture");
				auto student = IntraAPI::Student{
					winrt::to_string(login),
					winrt::to_string(title),
					winrt::to_string(picture)
				};
				students.push_back(std::move(student));
			}
			return students;
		});
	}


}