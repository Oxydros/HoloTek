#pragma once

namespace HoloTek
{
	class InteractionListener
	{
	public:
		InteractionListener() = default;
		virtual ~InteractionListener() = default;

	public:
		virtual void InteractionDetectedEvent(
			winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const &sender,
			winrt::Windows::UI::Input::Spatial::SpatialInteractionDetectedEventArgs const &args) = 0;
	};
}