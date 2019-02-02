#include "pch.h"
#include "SpatialInputHandler.h"

using namespace HoloTek;

using namespace std::placeholders;
using namespace winrt::Windows::UI::Input::Spatial;

// Creates and initializes a GestureRecognizer that listens to a Person.
SpatialInputHandler::SpatialInputHandler(InteractionListener *listener) :
	m_listener(listener)
{
    // The interaction manager provides an event that informs the app when
    // spatial interactions are detected.
    m_interactionManager = SpatialInteractionManager::GetForCurrentView();

    // Bind a handler to the SourcePressed event.
	m_interactionDetectedEventToken = m_interactionManager.InteractionDetected(bind(&SpatialInputHandler::OnInteractionDetected, this, _1, _2));
}

SpatialInputHandler::~SpatialInputHandler()
{
    // Unregister our handler for the OnSourcePressed event.
    m_interactionManager.InteractionDetected(m_interactionDetectedEventToken);
}

void SpatialInputHandler::OnInteractionDetected(SpatialInteractionManager const& sender, SpatialInteractionDetectedEventArgs const& args)
{
	if (m_listener)
		m_listener->InteractionDetectedEvent(sender, args);
}
