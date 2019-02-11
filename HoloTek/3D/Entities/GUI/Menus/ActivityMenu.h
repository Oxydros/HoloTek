#pragma once

#define ACTI_RENDER_SZ (3)

#include <3D\Entities\Common\Entity.h>
#include "3D\Entities\Common\EmptyEntity.h"
#include "API/IntraAPI.h"

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{
	class Panel;
	class Button2D;

	class ActivityMenu : public Entity
	{
	public:
		ActivityMenu(std::shared_ptr<DX::DeviceResources> devicesResources,
			std::shared_ptr<HolographicScene> scene, IntraAPI const &api);
		virtual ~ActivityMenu();

	public:
		std::future<void> InitializeMenuAsync();

		// Inherited via Entity
		std::string const GetLabel() const override { return "MainMenu"; };
		void DoUpdate(DX::StepTimer const & timer) override {};
		void setVisible(bool visibility) override final;

	public:
		std::future<void> refreshActivityListAsync();

	private:
		void RenderAtOffset(size_t offset = 0);
		void UpdateInfoLabel();
		void incOffset() {
			std::scoped_lock lock(m_propertyMutex);
			m_offset = (m_offset + ACTI_RENDER_SZ) % m_activities.size();
			m_currentPage = (m_currentPage + 1) % (m_maxPage + 1);
		}

		void decOffset() {
			std::scoped_lock lock(m_propertyMutex);
			m_offset = (m_offset - ACTI_RENDER_SZ);
			m_currentPage -= 1;
			if (m_offset < 0)
				m_offset += m_activities.size();
			if (m_currentPage < 0)
				m_currentPage = m_maxPage;
		}

	private:
		std::shared_ptr<DX::DeviceResources>	m_devicesResources;
		EmptyEntity								*m_activityList{ nullptr };
		Panel									*m_background{ nullptr };
		Entity									*m_rightArrow{ nullptr };
		Entity									*m_leftArrow{ nullptr };
		Button2D								*m_pageInfo{ nullptr };
		size_t									m_maxPage{ 0 };
		size_t									m_currentPage{ 0 };
		IntraAPI								const &m_api;
		std::mutex								m_propertyMutex;
		std::vector<IntraAPI::Activity>			m_activities;
		int										m_offset{ 0 };
	};
}