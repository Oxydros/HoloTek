///-------------------------------------------------------------------------------------------------
// file:	3D\Scene\HolographicScene.h
//
// summary:	Declares the holographic scene class
///-------------------------------------------------------------------------------------------------
#pragma once

#include "3D\Resources\DeviceResources.h"
#include "3D\Entities\Common\EmptyEntity.h"
#include "3D\Entities\GUI\Menus\MainMenu.h"
#include "3D\Entities\GUI\Menus\ActivityMenu.h"
#include "3D\Input\InteractionListener.h"

#include "Recognition\VideoFrameProcessor.h"
#include "Recognition\FacesBuffer.h"

#include "API/IntraAPI.h"

///-------------------------------------------------------------------------------------------------
// namespace: HoloLensClient
//
// summary:	.
///-------------------------------------------------------------------------------------------------
namespace HoloTek
{
	/// <summary>	A holographic scene. </summary>
	class HolographicScene : public std::enable_shared_from_this<HolographicScene>, public InteractionListener
	{
	public:
		/// <summary>	Defines an alias representing the shared pointer. </summary>
		typedef std::shared_ptr<HolographicScene>				SharedPtr;

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Constructor. </summary>
		///
		/// <param name="deviceResources">	[in,out] The device resources. </param>
		///-------------------------------------------------------------------------------------------------
		HolographicScene(std::shared_ptr<DX::DeviceResources> deviceResources);
		/// <summary>	Destructor. </summary>
		~HolographicScene();

	public:
		/// <summary>	Initializes this object. </summary>
		std::future<void> InitializeAsync();

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Updates the given timer. </summary>
		///
		/// <param name="timer">	The timer. </param>
		///-------------------------------------------------------------------------------------------------
		void Update(DX::StepTimer const& timer);
		/// <summary>	Renders this object. </summary>
		void Render();

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Updates the coordinate system described by coordinateSystem. </summary>
		///
		/// <param name="coordinateSystem">	The coordinate system. </param>
		///-------------------------------------------------------------------------------------------------
		void UpdateCoordinateSystem(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const &coordinateSystem);

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Updates the pointer pose described by pointerPose. </summary>
		///
		/// <param name="pointerPose">	The pointer pose. </param>
		///-------------------------------------------------------------------------------------------------
		void UpdatePointerPose(winrt::Windows::UI::Input::Spatial::SpatialPointerPose const &pointerPose);

		/// <summary>	Executes the device lost action. </summary>
		void OnDeviceLost();
		/// <summary>	Executes the device restored action. </summary>
		void OnDeviceRestored();

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Query if this object is alive. </summary>
		///
		/// <returns>	True if alive, false if not. </returns>
		///-------------------------------------------------------------------------------------------------
		bool isAlive() const { return m_alive; };
		/// <summary>	Kills this object. </summary>
		void kill() { m_alive = false; }
	public:
		inline void GoToMainMenu() {
			m_mainMenu->setVisible(true);
			m_activityMenu->setVisible(false);
		};
		inline void GoToActivityMenu() {
			m_mainMenu->setVisible(false);
			m_activityMenu->setVisible(true);
		};

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets device resources. </summary>
		///
		/// <returns>	The device resources. </returns>
		///-------------------------------------------------------------------------------------------------
		std::shared_ptr<DX::DeviceResources> getDeviceResources() const { return (m_deviceResources); };

	public:
		void InteractionDetectedEvent(
			winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const &sender,
			winrt::Windows::UI::Input::Spatial::SpatialInteractionDetectedEventArgs const &args) override;

	public:
		MainMenu &getMainMenu() const { return *m_mainMenu; }
		ActivityMenu &getActivityMenu() const { return *m_activityMenu; }
		IEntity	 &getCursor() const { return *m_cursor; }

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Adds an entity. </summary>
		///
		/// <param name="e">	An IEntity::IEntityPtr to process. </param>
		///-------------------------------------------------------------------------------------------------
		void addEntity(IEntity::IEntityPtr e);

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Adds an entity in front to 'dist'. </summary>
		///
		/// <param name="e">   	An IEntity::IEntityPtr to process. </param>
		/// <param name="dist">	(Optional) The distance. </param>
		///-------------------------------------------------------------------------------------------------
		void addEntityInFront(IEntity::IEntityPtr e, float dist = 2.0f);

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets coordinate system. </summary>
		///
		/// <returns>	Nullptr if it fails, else the coordinate system. </returns>
		///-------------------------------------------------------------------------------------------------
		winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const &getCoordinateSystem() const;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets pointer pose. </summary>
		///
		/// <returns>	Nullptr if it fails, else the pointer pose. </returns>
		///-------------------------------------------------------------------------------------------------
		winrt::Windows::UI::Input::Spatial::SpatialPointerPose const &getPointerPose() const;

	private:
		//Scene status
		bool															m_alive{ true };
		/// <summary>	The coordinate system. </summary>
		winrt::Windows::Perception::Spatial::SpatialCoordinateSystem	m_coordinateSystem{ nullptr };
		/// <summary>	The pointer pose. </summary>
		winrt::Windows::UI::Input::Spatial::SpatialPointerPose			m_pointerPose{ nullptr };
		/// <summary>	The device resources. </summary>
		std::shared_ptr<DX::DeviceResources>							m_deviceResources;
		/// <summary>	Root node. </summary>
		std::unique_ptr<EmptyEntity>									m_root;

		std::shared_ptr<VideoFrameProcessor>                            m_videoFrameProcessor;
		std::shared_ptr<FacesBuffer>									m_facesBuffer;

		/// <summary>	The main menu. </summary>
		MainMenu														*m_mainMenu{ nullptr };
		ActivityMenu													*m_activityMenu{ nullptr };
		/// <summary>	User cursor (head direction). </summary>
		IEntity															*m_cursor{ nullptr };
		/// <summary>	Entity currently focused (nearest) </summary>
		IEntity															*m_focusedEntity{ nullptr };

		IntraAPI														m_api;
	};
}

