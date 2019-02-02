#pragma once

#include <3D\Input\InteractionListener.h>

namespace HoloTek
{
    // Sample gesture handler.
    // Hooks up events to recognize a tap gesture, and keeps track of input using a boolean value.
    class SpatialInputHandler
    {
    public:
        SpatialInputHandler(InteractionListener *listener);
        ~SpatialInputHandler();

    private:
        // Interaction event handler.
        void OnInteractionDetected(
            winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const& sender,
            winrt::Windows::UI::Input::Spatial::SpatialInteractionDetectedEventArgs const& args);

        // API objects used to process gesture input, and generate gesture events.
        winrt::Windows::UI::Input::Spatial::SpatialInteractionManager       m_interactionManager = nullptr;

        // Event registration token.
        winrt::event_token                                                  m_interactionDetectedEventToken;

		InteractionListener													*m_listener;
    };
}
