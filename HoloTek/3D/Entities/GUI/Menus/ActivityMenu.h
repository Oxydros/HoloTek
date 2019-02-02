#pragma once

#include <3D\Entities\Common\Entity.h>
#include "3D\Entities\Common\EmptyEntity.h"
#include "API/IntraAPI.h"

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{
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

	private:
		std::future<void> refreshActivityListAsync();

	private:
		std::shared_ptr<DX::DeviceResources>	m_devicesResources;
		EmptyEntity								*m_activityList{ nullptr };
		Entity									*m_background{ nullptr };
		IntraAPI								const &m_api;
	};
}