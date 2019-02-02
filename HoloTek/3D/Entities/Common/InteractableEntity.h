#pragma once

#include <3D\Entities\Common\Entity.h>

using namespace winrt::Windows::UI::Input;

namespace HoloTek
{
	class InteractableEntity : public Entity
	{
	public:
		typedef std::function<void(Spatial::SpatialGestureRecognizer const &sender, Spatial::SpatialTappedEventArgs const &args)> AirTapCallback;

		typedef std::function<void(Spatial::SpatialGestureRecognizer const &,Spatial::SpatialManipulationStartedEventArgs const &)> ManipulationStartedCallback;
		typedef std::function<void(Spatial::SpatialGestureRecognizer const &, Spatial::SpatialManipulationUpdatedEventArgs const &)> ManipulationUpdatedCallback;
		typedef std::function<void(Spatial::SpatialGestureRecognizer const &, Spatial::SpatialManipulationCompletedEventArgs const &)> ManipulationCompletedCallback;
		typedef std::function<void(Spatial::SpatialGestureRecognizer const &, Spatial::SpatialManipulationCanceledEventArgs const &)> ManipulationCanceledCallback;
																	 
	private:
		winrt::event_token							m_tappedToken;
		winrt::event_token							m_manipStartedToken;
		winrt::event_token							m_manipUpdatedToken;
		winrt::event_token							m_manipCompletedToken;
		winrt::event_token							m_manipCanceledToken;

		AirTapCallback								m_airTapCallback;
		ManipulationStartedCallback					m_manipStart;
		ManipulationUpdatedCallback					m_manipUpdate;
		ManipulationCompletedCallback				m_manipComplete;
		ManipulationCanceledCallback				m_manipCanceled;

	public:
		InteractableEntity(std::shared_ptr<HolographicScene> scene,
							Spatial::SpatialGestureSettings = Spatial::SpatialGestureSettings::None);
		~InteractableEntity();

	public:
		void SetAirTapCallback(AirTapCallback callback) { m_airTapCallback = callback; }
		void SetManipulationStartedCallback(ManipulationStartedCallback callback) { m_manipStart = callback; }
		void SetManipulationUpdatedCallback(ManipulationUpdatedCallback callback) { m_manipUpdate = callback; }
		void SetManipulationCompletedCallback(ManipulationCompletedCallback callback) { m_manipComplete = callback; }
		void SetManipulationCanceledCallback(ManipulationCanceledCallback callback) { m_manipCanceled = callback; }

	private:
		virtual void OnAirTap(Spatial::SpatialGestureRecognizer const &sender,
			Spatial::SpatialTappedEventArgs const &args) {
			if (m_airTapCallback) m_airTapCallback(sender, args);
		}

		virtual void OnManipulationStarted(Spatial::SpatialGestureRecognizer const & sender,
			Spatial::SpatialManipulationStartedEventArgs const &args) {
			if (m_manipStart) m_manipStart(sender, args);
		}
		virtual void OnManipulationUpdated(Spatial::SpatialGestureRecognizer const & sender,
			Spatial::SpatialManipulationUpdatedEventArgs const &args) {
			if (m_manipUpdate) m_manipUpdate(sender, args);
		}
		virtual void OnManipulationCompleted(Spatial::SpatialGestureRecognizer const & sender,
			Spatial::SpatialManipulationCompletedEventArgs const &args) {
			if (m_manipComplete) m_manipComplete(sender, args);
		}
		virtual void OnManipulationCanceled(Spatial::SpatialGestureRecognizer const & sender,
			Spatial::SpatialManipulationCanceledEventArgs const &args) {
			if (m_manipCanceled) m_manipCanceled(sender, args);
		}
	};
}