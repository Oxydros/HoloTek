#pragma once

#include <3D\Entities\GUI\GUIEntity.h>

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{
	class Panel : public GUIEntity
	{
	protected:
		float2					_size;
		float4					_color;

	public:
		Panel(std::shared_ptr<DX::DeviceResources> devicesResources,
			  std::shared_ptr<HolographicScene> scene,
			  float2 size, float4 color);
		virtual ~Panel() = default;

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Add a child entity to the panel at a specify XY position. </summary>
		///
		/// <param name="child">	[in,out] If non-null, the child. </param>
		/// <param name="offset">	[in,out] Position (x, y) of the child on the panel. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void AddGUIEntity(IEntity::IEntityPtr child, winrt::Windows::Foundation::Numerics::float2 offsetXY);

		void DoUpdate(DX::StepTimer const &timer) override {};

		std::string const GetLabel() const override { return "Panel"; }
	};
}