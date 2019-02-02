#pragma once

#include <3D\Entities\Common\Entity.h>

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{
	class MainMenu : public Entity
	{
	private:
		std::shared_ptr<DX::DeviceResources>	m_devicesResources;

	public:
		MainMenu(std::shared_ptr<DX::DeviceResources> devicesResources,
				 std::shared_ptr<HolographicScene> scene);
		virtual ~MainMenu();

	public:
		void InitializeMenu();

		// Inherited via Entity
		std::string const GetLabel() const override { return "MainMenu"; };
		void DoUpdate(DX::StepTimer const & timer) override {};
	};
}