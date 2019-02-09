#pragma once

#include <3D\Entities\Common\Entity.h>
#include "3D\Entities\Common\EmptyEntity.h"

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{
	class MainMenu : public Entity
	{
	public:
		MainMenu(std::shared_ptr<DX::DeviceResources> devicesResources,
			std::shared_ptr<HolographicScene> scene);
		virtual ~MainMenu();

	public:
		std::future<void> InitializeMenuAsync();

		// Inherited via Entity
		std::string const GetLabel() const override { return "MainMenu"; };
		void DoUpdate(DX::StepTimer const & timer) override {};

		inline  void DisableRefreshButton() {
			m_refreshButton->SetIgnoreInGaze(true);
		}

		inline void ActivateRefreshButton() {
			m_refreshButton->SetIgnoreInGaze(false);
		}

		inline void DisplayStopButton() {
			m_stopButton->setVisible(true);
		}
	private:
		std::shared_ptr<DX::DeviceResources>	m_devicesResources;
		Entity									*m_refreshButton;
		Entity									*m_stopButton;
	};
}