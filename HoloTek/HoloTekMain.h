//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"

// Updates, renders, and presents holographic content using Direct3D.
namespace HoloTek
{
	class VideoFrameProcessor;
	class FaceTrackerProcessor;

	class QuadRenderer;
	class SpinningCubeRenderer;
	class TextRenderer;
	class NV12VideoTexture;

	class HoloTekMain : public DX::IDeviceNotify
	{
	public:
		HoloTekMain(std::shared_ptr<DX::DeviceResources> deviceResources);
		~HoloTekMain();

		// Sets the holographic space. This is our closest analogue to setting a new window
		// for the app.
		std::future<void> SetHolographicSpace(winrt::Windows::Graphics::Holographic::HolographicSpace const &holographicSpace);

		// Starts the holographic frame and updates the content.
		winrt::Windows::Graphics::Holographic::HolographicFrame Update();

		// Renders holograms, including world-locked content.
		bool Render(winrt::Windows::Graphics::Holographic::HolographicFrame const &holographicFrame);

		bool IsReadyToRender(void) const { return m_isReadyToRender; }

		// Handle saving and loading of app state owned by AppMain.
		void SaveAppState();
		void LoadAppState();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// Asynchronously creates resources for new holographic cameras.
		void OnCameraAdded(
			winrt::Windows::Graphics::Holographic::HolographicSpace const &sender,
			winrt::Windows::Graphics::Holographic::HolographicSpaceCameraAddedEventArgs const &args);

		// Synchronously releases resources for holographic cameras that are no longer
		// attached to the system.
		void OnCameraRemoved(
			winrt::Windows::Graphics::Holographic::HolographicSpace const &sender,
			winrt::Windows::Graphics::Holographic::HolographicSpaceCameraRemovedEventArgs const &args);

		// Used to notify the app when the positional tracking state changes.
		void OnLocatabilityChanged(
			winrt::Windows::Perception::Spatial::SpatialLocator const &sender,
			winrt::Windows::Foundation::IInspectable  const &args);

		// Clears event registration state. Used when changing to a new HolographicSpace
		// and when tearing down AppMain.
		void UnregisterHolographicEventHandlers();

		void ProcessFaces(
			std::vector<winrt::Windows::Graphics::Imaging::BitmapBounds> const& faces,
			winrt::Windows::Media::Capture::Frames::MediaFrameReference const &frame,
			winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const &worldCoordSystem);

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources>                            m_deviceResources;

		// Render loop timer.
		DX::StepTimer                                                   m_timer;

		// Represents the holographic space around the user.
		winrt::Windows::Graphics::Holographic::HolographicSpace               m_holographicSpace = nullptr;

		// SpatialLocator that is attached to the primary camera.
		winrt::Windows::Perception::Spatial::SpatialLocator                   m_locator = nullptr;

		// A reference frame attached to the holographic camera.
		winrt::Windows::Perception::Spatial::SpatialLocatorAttachedFrameOfReference m_referenceFrame = nullptr;


		// Video and face tracking processors
		std::shared_ptr<VideoFrameProcessor>                            m_videoFrameProcessor;
		std::shared_ptr<FaceTrackerProcessor>                           m_faceTrackerProcessor;

		// Objects related to rendering/3D models
		std::shared_ptr<QuadRenderer>                                   m_quadRenderer;
		std::shared_ptr<SpinningCubeRenderer>                           m_spinningCubeRenderer;
		std::shared_ptr<TextRenderer>                                   m_textRenderer;
		std::shared_ptr<NV12VideoTexture>                               m_videoTexture;

		// Used to avoid redundant copying of frames to our DirectX texture.
		int64_t                                                         m_previousFrameTimestamp = 0;

		// Indicates whether any faces being tracked at the moment.
		bool                                                            m_trackingFaces = false;

		// Indicates whether all resources necessary for rendering are ready.
		bool                                                            m_isReadyToRender = false;

		// Event registration tokens.
		winrt::event_token							                    m_cameraAddedToken;
		winrt::event_token												m_cameraRemovedToken;
		winrt::event_token												m_locatabilityChangedToken;
	};
}
