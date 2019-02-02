#include <pch.h>
#include "InteractableEntity.h"

HoloTek::InteractableEntity::InteractableEntity(std::shared_ptr<HolographicScene> scene, Spatial::SpatialGestureSettings settings)
	: Entity(scene)
{
	Spatial::SpatialGestureRecognizer spatial(settings);

	m_tappedToken = spatial.Tapped(std::bind(&InteractableEntity::OnAirTap, this, std::placeholders::_1, std::placeholders::_2));
	m_manipStartedToken = spatial.ManipulationStarted(std::bind(&InteractableEntity::OnManipulationStarted, this, std::placeholders::_1, std::placeholders::_2));
	m_manipUpdatedToken = spatial.ManipulationUpdated(std::bind(&InteractableEntity::OnManipulationUpdated, this, std::placeholders::_1, std::placeholders::_2));
	m_manipCompletedToken = spatial.ManipulationCompleted(std::bind(&InteractableEntity::OnManipulationCompleted, this, std::placeholders::_1, std::placeholders::_2));
	m_manipCanceledToken = spatial.ManipulationCanceled(std::bind(&InteractableEntity::OnManipulationCanceled, this, std::placeholders::_1, std::placeholders::_2));

	SetSpatialGestureRecognizer(spatial);
}

HoloTek::InteractableEntity::~InteractableEntity()
{
	auto spatial = GetSpatialGestureRecognizer();
	if (spatial)
	{
		spatial.Tapped(m_tappedToken);
		spatial.ManipulationStarted(m_manipStartedToken);
		spatial.ManipulationUpdated(m_manipUpdatedToken);
		spatial.ManipulationCompleted(m_manipCompletedToken);
		spatial.ManipulationCanceled(m_manipCanceledToken);
	}	
}
