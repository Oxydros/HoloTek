#pragma once

#define ACTI_RENDER_SZ (3)

#include <3D\Entities\Common\Entity.h>
#include "3D\Entities\Common\EmptyEntity.h"
#include "API/IntraAPI.h"

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{
	class Panel;

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
		void renderAtOffset(size_t offset = 0);
		void incOffset() {
			std::scoped_lock lock(m_propertyMutex);
			m_offset = (m_offset + ACTI_RENDER_SZ) % m_activities.size();
		}

		void decOffset() {
			std::scoped_lock lock(m_propertyMutex);
			m_offset = (m_offset - ACTI_RENDER_SZ);
			if (m_offset < 0)
				m_offset += m_activities.size();
		}

	private:
		std::shared_ptr<DX::DeviceResources>	m_devicesResources;
		EmptyEntity								*m_activityList{ nullptr };
		Panel									*m_background{ nullptr };
		Entity									*m_rightArrow{ nullptr };
		Entity									*m_leftArrow{ nullptr };
		IntraAPI								const &m_api;
		std::mutex								m_propertyMutex;
		std::vector<IntraAPI::Activity>			m_activities;
		int										m_offset{ 0 };
	};
}