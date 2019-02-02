///-------------------------------------------------------------------------------------------------
// file:	3D\Entities\Common\IEntity.h
//
// summary:	Declares the IEntity interface
///-------------------------------------------------------------------------------------------------
#pragma once

#include <3D\Utility\StepTimer.h>

///-------------------------------------------------------------------------------------------------
// namespace: HoloLensClient
//
// summary:	.
///-------------------------------------------------------------------------------------------------
namespace HoloTek
{

	class HolographicScene;

	/// <summary>	An entity. </summary>
	class IEntity
	{
	public:
		typedef std::unique_ptr<IEntity> IEntityPtr;

	public:
		/// <summary>	Destructor. </summary>
		virtual ~IEntity() = default;

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Update the entity based on the elapsed time. </summary>
		///
		/// <param name="timer">	StepTimer object used to get the elapsed time since the last frame. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void Update(DX::StepTimer const &timer) = 0;

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>
		/// 	Entity update function. You need to override this function to implement the logic of this
		/// 	entity.
		/// </summary>
		///
		/// <param name="timer">	StepTimer object used to get the elapsed time since the last frame. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void DoUpdate(DX::StepTimer const &timer) = 0;

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Return the entity name. </summary>
		///
		/// <returns>	The label. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual std::string const GetLabel() const = 0;

		/// <summary>	Initialize the mesh. </summary>
		virtual std::future<void> InitializeMeshAsync() = 0;

		/// <summary>	Release the resources of the mesh. </summary>
		virtual void ReleaseMesh() = 0;

		/// <summary>	Render the mesh. </summary>
		virtual void Render() = 0;

		/// <summary>	Mark this entity for death. </summary>
		virtual void kill() = 0;

		/// <summary>	Return true is this entity is ignored in the gaze calculation. </summary>
		virtual bool IgnoreInGaze() const = 0;

		/// <summary>	Set the ignore in gaze property </summary>
		virtual void SetIgnoreInGaze(bool ignore) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Sets a visible. </summary>
		///
		/// <param name="visibility">	True to visibility. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void setVisible(bool visibility) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Query if this object is visible. </summary>
		///
		/// <returns>	True if visible, false if not. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual bool isVisible() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Query if this object is root. </summary>
		///
		/// <returns>	True if root, false if not. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual bool isRoot() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Check if this entity is marked for death. </summary>
		///
		/// <returns>	True if dead, false if not. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual bool isDead() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Translate of the offset value. </summary>
		///
		/// <param name="offset">	The offset. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void Move(winrt::Windows::Foundation::Numerics::float3 offset) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Rotates using the given offset. </summary>
		///
		/// <param name="offset">	The offset. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void Rotate(winrt::Windows::Foundation::Numerics::float3 offset) = 0;

		virtual  winrt::Windows::Foundation::Numerics::float3 GetScale() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Scale using the given offset. Add up the previous scale and this one </summary>
		///
		/// <param name="offset">	The offset. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void Scale(winrt::Windows::Foundation::Numerics::float3 offset) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the scale to the specified vector, erase the previous one </summary>
		///
		/// <param name="scale">	The scaling vector. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetScale(winrt::Windows::Foundation::Numerics::float3 scale) = 0;
		virtual void SetScaleX(float scaleX) = 0;
		virtual void SetScaleY(float scaleY) = 0;
		virtual void SetScaleZ(float scaleZ) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Get the real size of the object. </summary>
		///-------------------------------------------------------------------------------------------------
		virtual winrt::Windows::Foundation::Numerics::float3 GetSize() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the real size of the object. </summary>
		///-------------------------------------------------------------------------------------------------
		virtual void SetSize(winrt::Windows::Foundation::Numerics::float3 originalSize) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the relative position. </summary>
		///
		/// <param name="position">	The position. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetRelativePosition(winrt::Windows::Foundation::Numerics::float3 position) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the relative X position. </summary>
		///
		/// <param name="position">	The position. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetRelativeX(float x) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the relative Y position. </summary>
		///
		/// <param name="position">	The position. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetRelativeY(float y) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the relative Z position. </summary>
		///
		/// <param name="position">	The position. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetRelativeZ(float z) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the relative rotation. </summary>
		///
		/// <param name="rotation">	The rotation. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetRelativeRotation(winrt::Windows::Foundation::Numerics::float3 rotation) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the real position. </summary>
		///
		/// <param name="position">	The position. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetRealPosition(winrt::Windows::Foundation::Numerics::float3 position) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the real rotation. </summary>
		///
		/// <param name="rotation">	The rotation. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetRealRotation(winrt::Windows::Foundation::Numerics::float3 rotation) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the real position matrix. </summary>
		///
		/// <param name="positionMatrix">	[in,out] The position matrix. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetModelPosition(DirectX::XMMATRIX &positionMatrix) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the real rotation matrix. </summary>
		///
		/// <param name="rotationMatrix">	[in,out] The rotation matrix. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetModelRotation(DirectX::XMMATRIX &rotationMatrix) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Return the real position. </summary>
		///
		/// <returns>	The position. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual winrt::Windows::Foundation::Numerics::float3 const GetRealPosition() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Return the real rotation. </summary>
		///
		/// <returns>	The rotation. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual winrt::Windows::Foundation::Numerics::float3 const GetRealRotation() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Return the relative position to its parent. </summary>
		///
		/// <returns>	The relative position. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual winrt::Windows::Foundation::Numerics::float3 const &GetRelativePosition() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Return the relative rotation to its parent. </summary>
		///
		/// <returns>	The relative rotation. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual winrt::Windows::Foundation::Numerics::float3 const &GetRelativeRotation() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Return the real position matrix. </summary>
		///
		/// <returns>	The position matrix. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual DirectX::XMMATRIX const &GetPositionMatrix() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Return the real rotation matrix. </summary>
		///
		/// <returns>	The rotation matrix. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual DirectX::XMMATRIX const &GetRotationMatrix() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets transform matrix. </summary>
		///
		/// <returns>	The transform matrix. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual DirectX::XMMATRIX const GetTransformMatrix() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Add a child entity. </summary>
		///
		/// <param name="child">	[in,out] If non-null, the child. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void AddChild(IEntity::IEntityPtr child) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Remove a child entity. </summary>
		///
		/// <param name="child">	[in,out] If non-null, the child. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void RemoveChild(IEntity *child) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set the parent of this entity. </summary>
		///
		/// <param name="parent">	[in,out] If non-null, the parent. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetParent(IEntity *parent) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Get the parent of this entity. </summary>
		///
		/// <returns>	Null if it fails, else the parent. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual IEntity *getParent() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Define if the entity must follow the user gaze at each frame. </summary>
		///
		/// <param name="followGaze">	  	True to follow gaze. </param>
		/// <param name="followOrient">   	True to follow orient. </param>
		/// <param name="positionOffsets">	The position offsets. </param>
		/// <param name="rotationOffsets">	The rotation offsets. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void setFollowGaze(bool followGaze, bool followOrient,
			winrt::Windows::Foundation::Numerics::float3 positionOffsets,
			winrt::Windows::Foundation::Numerics::float3 rotationOffsets) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Place the entity in front of the user gaze. </summary>
		///
		/// <param name="offsets">	The offsets. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void positionInFrontOfGaze(winrt::Windows::Foundation::Numerics::float3 offsets) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>
		/// 	Place the entity in front of the user gaze No use of offsets for now (vector Matrix)
		/// </summary>
		///
		/// <param name="offsets">	The offsets. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void rotateTowardGaze(winrt::Windows::Foundation::Numerics::float3 offsets) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Query if this object is in gaze. </summary>
		///
		/// <returns>	True if in gaze, false if not. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual bool isInGaze() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Query if this object is focused. </summary>
		///
		/// <returns>	True if focused, false if not. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual bool isFocused() const = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Sets the focus. </summary>
		///
		/// <param name="focused">	True if focused. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void setFocus(bool focused) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets in gaze entities. </summary>
		///
		/// <param name="entities">	[in,out] [in,out] If non-null, the entities. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void getInGazeEntities(std::vector<IEntity*> &entities) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets nearest in gaze entity. </summary>
		///
		/// <returns>	Null if it fails, else the nearest in gaze entity. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual std::pair<IEntity*, float> getNearestInGazeEntity() = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Start capturing interaction using the SpatialGestureRecognizer. </summary>
		///
		/// <param name="interaction">	[in,out] [in,out] If non-null, the spatial interaction forwarded to the recognizer. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void CaptureInteraction(winrt::Windows::UI::Input::Spatial::SpatialInteraction const &interaction) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Set a custom SpatialGestureRecognizer </summary>
		///
		/// <param name="recognizer"> SpatialGestureRecognizer </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetSpatialGestureRecognizer(winrt::Windows::UI::Input::Spatial::SpatialGestureRecognizer const &recognizer) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Return the binded SpatialGestureRecognizer </summary>
		///
		/// <returns>	ref to SpatialGestureRecognizer </returns>
		///-------------------------------------------------------------------------------------------------
		virtual winrt::Windows::UI::Input::Spatial::SpatialGestureRecognizer GetSpatialGestureRecognizer() = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Return the scene in which this entity is stored </summary>
		///
		/// <returns>	shared_ptr to the HolographicScene </returns>
		///-------------------------------------------------------------------------------------------------
		virtual std::shared_ptr<HolographicScene>	GetScene() = 0;

		/////-------------------------------------------------------------------------------------------------
		///// <summary>	Force set the network ID </summary>
		/////
		///// <param name="id">	The entity id. </param>
		/////-------------------------------------------------------------------------------------------------
		//virtual void SetID(int id) = 0;

		/////-------------------------------------------------------------------------------------------------
		///// <summary>	Return the network ID </summary>
		/////
		///// <returns>	The ID in int. (int32 in network protocol) </returns>
		/////-------------------------------------------------------------------------------------------------
		//virtual int GetID() const = 0;

		/////-------------------------------------------------------------------------------------------------
		///// <summary>	Try to retrieve an entity with the given network ID in the entity tree </summary>
		/////
		///// <returns>	nullptr if not found, or else a ptr to the entity </returns>
		/////-------------------------------------------------------------------------------------------------
		//virtual IEntity	*RetrieveEntity(int id) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Get the BoundingBox of the entity. </summary>
		///
		/// <returns>	The bounding box. </returns>
		///-------------------------------------------------------------------------------------------------
		virtual DirectX::BoundingOrientedBox const GetBoundingBox() = 0;
	};
}