#include "pch.h"
#include "IntraAPI.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Web;
using namespace winrt::Windows::Web::Http;

namespace HoloTek
{
	IntraAPI::IntraAPI(std::string autoLoginURL) :
		m_autoLogin(autoLoginURL)
	{
	}

	IntraAPI::~IntraAPI()
	{
	}

	IAsyncOperation<HttpStatusCode> IntraAPI::LoginAsync() const
	{
		auto headers = m_client.DefaultRequestHeaders();

		auto uri = winrt::Windows::Foundation::Uri(winrt::impl::to_hstring(m_autoLogin));
		auto response = co_await m_client.GetAsync(uri);

		co_return response.StatusCode();
	}

	concurrency::task<std::vector<IntraAPI::Activity>> IntraAPI::GetActivitiesAsync(bool timeCheck) const
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

			auto planningURI = Uri(winrt::impl::to_hstring(uriString));
			auto response = m_client.GetAsync(planningURI).get();

			auto content = response.Content().ReadAsStringAsync().get();

			auto root = winrt::Windows::Data::Json::JsonValue::Parse(content);

			try
			{
				for (auto item : root.GetArray())
				{
					auto object = item.GetObject();

					auto startTime = object.GetNamedString(L"start");
					auto endTime = object.GetNamedString(L"end");

					if (timeCheck)
					{
						struct tm actiStartTime;
						struct tm actiEndTime;
						std::stringstream ssStart(std::string(startTime.begin(), startTime.end()));
						std::stringstream ssEnd(std::string(startTime.begin(), startTime.end()));
						ssStart >> std::get_time(&actiStartTime, "%Y-%m-%d %H:%M:%S");
						ssEnd >> std::get_time(&actiEndTime, "%Y-%m-%d %H:%M:%S");

						auto mkStart = std::mktime(&actiStartTime);
						auto mkEnd = std::mktime(&actiEndTime);
						//If not current time inside the activity hours, just skip it
						//currentTime must be > than mkStart
						//mkEnd must be > than currentTime
						if (!(std::difftime(currentTime, mkStart) >= 0 &&
							std::difftime(mkEnd, currentTime) >= 0))
							continue;
					}

					auto scholarYear = object.GetNamedString(L"scolaryear");
					auto codeModule = object.GetNamedString(L"codemodule");
					auto codeInstance = object.GetNamedString(L"codeinstance");
					auto codeActi = object.GetNamedString(L"codeacti");
					auto codeEvent = object.GetNamedString(L"codeevent");
					auto moduleName = object.GetNamedString(L"titlemodule");
					auto actiTitle = object.GetNamedString(L"acti_title");
					auto newActivity = IntraAPI::Activity{
						scholarYear,
						codeModule,
						codeInstance,
						codeActi,
						codeEvent,
						moduleName
					};
					activities.push_back(std::move(newActivity));
				}
			}
			catch (winrt::hresult_illegal_method_call const &e)
			{
				TRACE("Error while processing actvities " << e.message().c_str()
					<< " content is " << content.c_str() << std::endl);
				return activities;
			}
			return activities;
		});
	}

	//https://intra.epitech.eu/module/2018/B-INN-000/TLS-0-1/acti-326701/event-322207/registered?format=json
	concurrency::task<std::vector<IntraAPI::Student>> IntraAPI::GetRegisteredStudentsAsync(Activity const &activity) const
	{
		return concurrency::create_task([&] {
			std::vector<IntraAPI::Student> students;

			std::wstringstream uriStream;

			uriStream << "https://intra.epitech.eu/module/"
				<< activity.scholarYear.c_str() << "/"
				<< activity.codeModule.c_str() << "/"
				<< activity.codeInstance.c_str() << "/"
				<< activity.codeActi.c_str() << "/"
				<< activity.codeEvent.c_str() << "/registered?format=json";

			auto uriString = uriStream.str();

			auto registeredURI = Uri(uriString);
			auto response = m_client.GetAsync(registeredURI).get();

			auto content = response.Content().ReadAsStringAsync().get();

			auto root = winrt::Windows::Data::Json::JsonValue::Parse(content);

			try
			{
				for (auto item : root.GetArray())
				{
					auto object = item.GetObject();
					auto login = object.GetNamedString(L"login");
					auto title = object.GetNamedString(L"title");
					auto picture = object.GetNamedString(L"picture");
					auto student = IntraAPI::Student{
						login,
						title,
						picture
					};
					students.push_back(std::move(student));
				}
			}
			catch (winrt::hresult_illegal_method_call const &e)
			{
				TRACE("Error while processing registered students " << e.message().c_str()
					<< " content is " << content.c_str() << std::endl);
				return students;
			}
			return students;
		});
	}

	//https://intra.epitech.eu/module/2018/B-INN-000/TLS-0-1/acti-331246/event-329201/updateregistered?format=json
	IAsyncAction IntraAPI::MarkRegisteredStudentsAsync(Activity activity,
		winrt::Windows::Foundation::Collections::IVector<winrt::hstring> logins) const
	{
		std::wstringstream uriStream;

		uriStream << "https://intra.epitech.eu/module/"
			<< activity.scholarYear.c_str() << "/"
			<< activity.codeModule.c_str() << "/"
			<< activity.codeInstance.c_str() << "/"
			<< activity.codeActi.c_str() << "/"
			<< activity.codeEvent.c_str() << "/updateregistered?format=json";

		TRACE("URL is " << uriStream.str().c_str() << std::endl);

		auto uriString = uriStream.str();
		auto registeredURI = Uri(uriString);

		std::map<winrt::hstring, winrt::hstring> contentMap{};

		for (size_t i = 0; i < logins.Size(); i++) {
			auto student = logins.GetAt(i);
			//{ L"items[0][present]", L"present" }
			std::wstringstream itemStream;
			itemStream << "items[" << i << "]";

			auto loginKey = itemStream.str() + L"[login]";
			auto presenceKey = itemStream.str() + L"[present]";

			contentMap[loginKey.c_str()] = student;
			contentMap[presenceKey.c_str()] = L"present";
		}

		auto requestContent = winrt::single_threaded_map<winrt::hstring, winrt::hstring>(std::move(contentMap));
		winrt::Windows::Web::Http::HttpFormUrlEncodedContent contents(requestContent);

		auto c = co_await contents.ReadAsStringAsync();
		TRACE("Got " << c.c_str() << std::endl);

		try
		{
			auto response = co_await m_client.PostAsync(registeredURI, contents);
			response.EnsureSuccessStatusCode();
			auto content = co_await response.Content().ReadAsStringAsync();
			TRACE("Got response content " << content.c_str() << std::endl);
		}
		catch (winrt::hresult_error const &ex) {
			TRACE("Error " << ex.message().c_str() << std::endl);
		}
		co_return;
	}


}