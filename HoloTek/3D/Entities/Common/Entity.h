#pragma once

#include "3D\Resources\DeviceResources.h"
#include "3D\Objects\Common\AObject.h"
#include "3D\Entities\Common\IEntity.h"

using namespace winrt;

namespace HoloTek
{
	class HolographicScene;

	class Entity : public IEntity
	{
	public:
		typedef std::unique_ptr<Entity>	EntityPtr;

	public:
		Entity(std::shared_ptr<HolographicScene> scene);
		virtual ~Entity();

	public:
		void Update(DX::StepTimer const &timer) override final;

		std::future<void> InitializeMeshAsync() override;
		void ReleaseMesh() override;
		void Render() override;
		void kill() override;
		bool isDead() const override;
		void setVisible(bool visibility) override;
		bool isVisible() const override { return (m_visible); }
		bool isRoot() const override { return (m_isRoot); }
		bool IgnoreInGaze() const { return (m_ignoreInGaze); }
		void SetIgnoreInGaze(bool ignore) { m_ignoreInGaze = ignore; }

		void Move(winrt::Windows::Foundation::Numerics::float3 offset) override;
		void Rotate(winrt::Windows::Foundation::Numerics::float3 offset) override;

		winrt::Windows::Foundation::Numerics::float3 GetScale() const override { return m_scaling; }
		void Scale(winrt::Windows::Foundation::Numerics::float3 offset) override;

		void SetScale(winrt::Windows::Foundation::Numerics::float3 scale) override;
		void SetScaleX(float scaleX);
		void SetScaleY(float scaleY);
		void SetScaleZ(float scaleZ);

		winrt::Windows::Foundation::Numerics::float3 GetSize() const override;
		void SetSize(winrt::Windows::Foundation::Numerics::float3 originalSize) override { m_originalSize = originalSize; }

		void SetRelativeX(float x) override { m_relativePosition.x = x; }
		void SetRelativeY(float y) override { m_relativePosition.y = y; }
		void SetRelativeZ(float z) override { m_relativePosition.z = z; }

		void SetRealPosition(winrt::Windows::Foundation::Numerics::float3 position) override;
		void SetRealRotation(winrt::Windows::Foundation::Numerics::float3 rotation) override;

		void SetRelativePosition(winrt::Windows::Foundation::Numerics::float3 position) override;
		void SetRelativeRotation(winrt::Windows::Foundation::Numerics::float3 rotation) override;

		void SetModelPosition(DirectX::XMMATRIX &positionMatrix) override;
		void SetModelRotation(DirectX::XMMATRIX &rotationMatrix) override;

		winrt::Windows::Foundation::Numerics::float3 const GetRealPosition() const override;
		winrt::Windows::Foundation::Numerics::float3 const GetRealRotation() const override;

		winrt::Windows::Foundation::Numerics::float3 const &GetRelativePosition() const override { return m_relativePosition; };
		winrt::Windows::Foundation::Numerics::float3 const &GetRelativeRotation() const override { return m_relativeRotation; };

		DirectX::XMMATRIX const &GetPositionMatrix() const override { return m_modelRotation; };
		DirectX::XMMATRIX const &GetRotationMatrix() const override { return m_modelTranslation; };

		DirectX::XMMATRIX const GetTransformMatrix() const override;

		void AddChild(IEntity::IEntityPtr child) override;
		void RemoveChild(IEntity *child) override;
		void SetParent(IEntity *parent) override;
		IEntity *getParent() const override;

		void setFollowGaze(bool followGazePosition, bool followGazeRotation,
			winrt::Windows::Foundation::Numerics::float3 positionOffsets = { 0.0f, 0.0f, 0.0f },
			winrt::Windows::Foundation::Numerics::float3 rotationOffsets = { 0.0f, 0.0f, 0.0f }) override
		{
			m_followGazePosition = followGazePosition;
			m_followGazeRotation = followGazeRotation;
			m_positionOffsetFromGaze = positionOffsets;
			m_rotationOffsetFromGaze = rotationOffsets;
		}

		void positionInFrontOfGaze(winrt::Windows::Foundation::Numerics::float3 offsets) override;
		void rotateTowardGaze(winrt::Windows::Foundation::Numerics::float3 offsets) override;

		bool isInGaze() const override { return (m_inGaze); };
		bool isFocused() const override { return (m_focused); };

		void setFocus(bool focused) override;

		void getInGazeEntities(std::vector<IEntity*> &entities) override final;
		std::pair<IEntity*, float> getNearestInGazeEntity() override final;

		void CaptureInteraction(winrt::Windows::UI::Input::Spatial::SpatialInteraction const &interaction) override final;

		void SetSpatialGestureRecognizer(winrt::Windows::UI::Input::Spatial::SpatialGestureRecognizer const &recognizer) override final;
		winrt::Windows::UI::Input::Spatial::SpatialGestureRecognizer GetSpatialGestureRecognizer() override { return m_spatialGestureRecognizer; };

		std::shared_ptr<HolographicScene>	GetScene() override { return m_scene; }

		//IEntity	*RetrieveEntity(int id) override;

		DirectX::BoundingOrientedBox const GetBoundingBox() override;

	public:
		/// <summary>	Executes the get focus action. </summary>
		virtual bool OnGetFocus() { return false; }

		/// <summary>	Executes the lost focus action. </summary>
		virtual bool OnLostFocus() { return false; }

	protected:

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Adds a mesh. </summary>
		///
		/// <param name="mesh">	The mesh. </param>
		///-------------------------------------------------------------------------------------------------
		std::future<void> addMesh(AObject::AObjectPtr mesh);
		///-------------------------------------------------------------------------------------------------
		/// <summary>	Calculate if this entity is in user gaze. </summary>
		///-------------------------------------------------------------------------------------------------
		void updateInGaze();

	public:
		///-------------------------------------------------------------------------------------------------
		/// <summary>	Update function getting called at this tick. Override it if you need to. </summary>
		///
		/// <param name="timer">	Delta time since last tick. </param>
		///-------------------------------------------------------------------------------------------------
		void DoUpdate(DX::StepTimer const &timer) override {};

	private:
		winrt::Windows::UI::Input::Spatial::SpatialGestureRecognizer	m_spatialGestureRecognizer{ nullptr };

	protected:
		AObject::AObjectPtr										m_mesh{ nullptr };
		std::vector<IEntity::IEntityPtr>						m_newChilds;
		std::vector<IEntity::IEntityPtr>						m_childs;
		IEntity													*m_parent{ nullptr };
		bool													m_alive{ true };
		std::shared_ptr<HolographicScene>						m_scene = nullptr;
		bool													m_visible{ true };
		bool													m_focused{ false };
		bool													m_inGaze{ false };
		bool													m_isRoot{ false };
		bool													m_ignoreInGaze{ false };

		winrt::Windows::Foundation::Numerics::float3			m_previousGazePosition{ -1, -1, -1 };
		winrt::Windows::Foundation::Numerics::float3			m_previousGazeDirection{ -1, -1, -1 };

		bool													m_followGazePosition{ false };
		bool													m_followGazeRotation{ false };
		winrt::Windows::Foundation::Numerics::float3			m_positionOffsetFromGaze{ 0, 0, 0 };
		winrt::Windows::Foundation::Numerics::float3			m_rotationOffsetFromGaze{ 0, 0, 0 };

		winrt::Windows::Foundation::Numerics::float3			m_relativePosition{ 0, 0, 0 };
		winrt::Windows::Foundation::Numerics::float3			m_relativeRotation{ 0, 0, 0 };
		winrt::Windows::Foundation::Numerics::float3			m_scaling{ 1, 1, 1 };
		winrt::Windows::Foundation::Numerics::float3			m_originalSize{ 0, 0, 0 };

		DirectX::XMMATRIX										m_modelTranslation{};
		DirectX::XMMATRIX										m_modelRotation{};
		DirectX::XMMATRIX										m_modelScaling{};
		float													m_distance{ 0 };

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>
		/// 	Required for align of 16B for XMMAtrix
		/// 	https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16.
		/// </summary>
		///
		/// <param name="i">	Zero-based index of the. </param>
		///-------------------------------------------------------------------------------------------------
		void* operator new(size_t i)
		{
			return _mm_malloc(i, 16);
		}

		///-------------------------------------------------------------------------------------------------
		/// <summary>
		/// 	Required for align of 16B for XMMAtrix
		/// 	https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16.
		/// </summary>
		///
		/// <param name="p">	[in,out] If non-null, the p to delete. </param>
		///
		/// <returns>	The result of the operation. </returns>
		///-------------------------------------------------------------------------------------------------
		void operator delete(void* p)
		{
			_mm_free(p);
		}
	};
}

std::ostream& operator<<(std::ostream& stream, const DirectX::XMMATRIX& matrix);
std::ostream& operator<<(std::ostream& stream, const winrt::Windows::Foundation::Numerics::float3 a);