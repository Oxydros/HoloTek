#include "pch.h"
#include "3D\Utility\Collision.h"
#include "3D\Entities\Common\Entity.h"
#include "3D\Scene\HolographicScene.h"

using namespace HoloTek;
using namespace DirectX;
using namespace winrt::Windows::Foundation::Numerics;

Entity::Entity(std::shared_ptr<HolographicScene> scene)
	: m_parent(nullptr), m_alive(true), m_scene(scene)
{
	SetRealRotation({ 0, 0, 0 });
	SetRealPosition({ 0, 0, 0 });
	m_modelScaling = XMMatrixIdentity();
}

Entity::~Entity()
{
	// Delete all childs
	m_childs.clear();

	if (m_mesh)
		m_mesh->ReleaseDeviceDependentResources();
}

DirectX::XMMATRIX const Entity::GetTransformMatrix() const
{
	DirectX::XMMATRIX localTransform = m_modelScaling * m_modelRotation * m_modelTranslation;
	DirectX::XMMATRIX finalTransform = m_parent != nullptr ? localTransform * m_parent->GetTransformMatrix() : localTransform;

	//TRACE("For " << GetLabel() << " Rotation " << _realRotation << " " <<  _modelRotation << std::endl);
	//TRACE("For " << GetLabel() << " Translation " << _realPosition << " " << _modelTranslation << std::endl);
	//TRACE("For " << GetLabel() << " Local " << localTransform << std::endl);
	//TRACE("For " << GetLabel() << " Final " << finalTransform << std::endl);
	return (finalTransform);
}

void Entity::Update(DX::StepTimer const & timer)
{
	if (m_scene->getPointerPose() != nullptr) {
		float3 positionMotion = m_scene->getPointerPose().Head().Position() - m_previousGazePosition;
		float3 directionMotion = m_scene->getPointerPose().Head().ForwardDirection()- m_previousGazeDirection;
		/*GazeMotion(positionMotion, directionMotion);*/
		m_previousGazePosition = m_scene->getPointerPose().Head().Position();
		m_previousGazeDirection = m_scene->getPointerPose().Head().ForwardDirection();
	}

	//Update position and orient if needed
	if (m_followGazeRotation)
		rotateTowardGaze(m_rotationOffsetFromGaze);
	if (m_followGazePosition)
		positionInFrontOfGaze(m_positionOffsetFromGaze);

	//Update childs
	std::for_each(m_childs.begin(), m_childs.end(),
		[&timer](auto &child)
	{
		/*TRACE("Updating child " << child->GetLabel() << std::endl);*/
		child->Update(timer);
	});

	updateInGaze();
	DoUpdate(timer);

	if (m_mesh)
		m_mesh->SetModelTransform(GetTransformMatrix());

	// If child is delete, remove the unique_ptr from the child list
	// This will delete its instance
	m_childs.erase(
		std::remove_if(m_childs.begin(), m_childs.end(),
			[](auto &child) {
				if (child->isDead())
					TRACE("Killing child " << child->GetLabel().c_str() << std::endl);
				return child->isDead();
		}),
		m_childs.end()
	);

	//Add new entities created at the previous call to update
	std::for_each(m_newChilds.begin(), m_newChilds.end(),
		[this](auto &child)
	{
		TRACE("Adding new child from pending list " << child->GetLabel().c_str() << " " << child.get() << std::endl);
		m_childs.emplace_back(std::move(child));
	});
	m_newChilds.clear();

	/*TRACE("For " << this << " Real position is (" << _realPosition.x << ", " << _realPosition.y << ", " << _realPosition.z
		  << ") Relative is (" << _relativePosition.x << ", " << _relativePosition.y << ", " << _relativePosition.z << ")"
			<< " Bools matrix: " << _useTranslationMatrix << " " << _useRotationMatrix << std::endl);*/
}

std::future<void> Entity::InitializeMeshAsync()
{
	std::for_each(m_childs.begin(), m_childs.end(),
		[](auto &child) -> std::future<void>
	{
		TRACE("Init mesh of " << child->GetLabel().c_str() << std::endl);
		co_await child->InitializeMeshAsync();
		co_return;
	});

	if (m_mesh)
		co_await m_mesh->CreateDeviceDependentResourcesAsync();
	co_return;
}

void Entity::ReleaseMesh()
{
	std::for_each(m_childs.begin(), m_childs.end(),
		[](auto &child)
	{
		TRACE("Release mesh of " << child->GetLabel().c_str() << std::endl);
		child->ReleaseMesh();
	});

	if (m_mesh)
		m_mesh->ReleaseDeviceDependentResources();
}

void Entity::Render()
{
	std::for_each(m_childs.begin(), m_childs.end(),
		[](auto &child)
	{
		/*TRACE("Render mesh of " << child->GetLabel() << std::endl);*/
		child->Render();
	});

	// Don't render if entity is not supposed to be visible
	if (m_visible && m_mesh)
		m_mesh->Render();
}

void Entity::kill()
{
	m_alive = false;
}

bool Entity::isDead() const
{
	return (!m_alive);
}

void Entity::setVisible(bool visibility)
{
	m_visible = visibility;
	std::for_each(m_childs.begin(), m_childs.end(),
		[&visibility](auto &child)
	{
		child->setVisible(visibility);
	});
}

void Entity::Move(winrt::Windows::Foundation::Numerics::float3 offset)
{
	m_relativePosition += offset;
	m_modelTranslation = XMMatrixTranslationFromVector(XMLoadFloat3(&m_relativePosition));
}

void Entity::Rotate(winrt::Windows::Foundation::Numerics::float3 offset)
{
	m_relativeRotation += offset;
	m_modelRotation = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_relativeRotation));
}

void Entity::Scale(winrt::Windows::Foundation::Numerics::float3 offset)
{
	m_scaling += offset;
	m_modelScaling = XMMatrixScalingFromVector(XMLoadFloat3(&m_scaling));
}

void Entity::SetScale(winrt::Windows::Foundation::Numerics::float3 scale)
{
	m_scaling = scale;
	m_modelScaling = XMMatrixScalingFromVector(XMLoadFloat3(&m_scaling));
}

void Entity::SetScaleX(float scaleX)
{
	m_scaling.x = scaleX;
	m_modelScaling = XMMatrixScalingFromVector(XMLoadFloat3(&m_scaling));
}

void Entity::SetScaleY(float scaleY)
{
	m_scaling.y = scaleY;
	m_modelScaling = XMMatrixScalingFromVector(XMLoadFloat3(&m_scaling));
}

void Entity::SetScaleZ(float scaleZ)
{
	m_scaling.z = scaleZ;
	m_modelScaling = XMMatrixScalingFromVector(XMLoadFloat3(&m_scaling));
}

winrt::Windows::Foundation::Numerics::float3 Entity::GetSize() const
{
	return (winrt::Windows::Foundation::Numerics::float3(
		m_originalSize.x * m_scaling.x,
		m_originalSize.y * m_scaling.y,
		m_originalSize.z * m_scaling.z));
}

void Entity::SetRelativePosition(winrt::Windows::Foundation::Numerics::float3 position)
{
	m_relativePosition = position;
	m_modelTranslation = XMMatrixTranslationFromVector(XMLoadFloat3(&m_relativePosition));
}

void Entity::SetRelativeRotation(winrt::Windows::Foundation::Numerics::float3 rotation)
{
	m_relativeRotation = rotation;
	m_modelRotation = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_relativeRotation));
}

void Entity::SetRealPosition(winrt::Windows::Foundation::Numerics::float3 position)
{
	if (m_parent == nullptr)
		return SetRelativePosition(position);
	m_relativePosition = position - m_parent->GetRealPosition();
	m_modelTranslation = XMMatrixTranslationFromVector(XMLoadFloat3(&m_relativePosition));
}

void Entity::SetRealRotation(winrt::Windows::Foundation::Numerics::float3 rotation)
{
	if (m_parent == nullptr)
		return SetRelativeRotation(rotation);
	m_relativeRotation = rotation - m_parent->GetRealRotation();
	m_modelRotation = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_relativeRotation));
}

void Entity::SetModelPosition(DirectX::XMMATRIX &positionMatrix)
{
	/*TRACE("WARNING: Use of setRealPosition Matrix for "  << std::endl);*/
	/*if (_parent != nullptr && !_parent->isRoot()) throw std::runtime_error("Can't update real position because it is a Child entity");*/
	m_modelTranslation = positionMatrix;
}

void Entity::SetModelRotation(DirectX::XMMATRIX &rotationMatrix)
{
	/*TRACE("WARNING: Use of setRealRotation Matrix for "  << std::endl);*/
	/*if (_parent != nullptr && !_parent->isRoot()) throw std::runtime_error("Can't update real position because it is a Child entity");*/
	m_modelRotation = rotationMatrix;
}

winrt::Windows::Foundation::Numerics::float3 const Entity::GetRealPosition() const
{
	float3 parentReal = { 0, 0, 0 };

	//Real position is the sum of all relative positions starting from the root node
	if (m_parent)
		parentReal = m_parent->GetRealPosition();
	return (parentReal + m_relativePosition);
}

winrt::Windows::Foundation::Numerics::float3 const Entity::GetRealRotation() const
{
	float3 parentReal = { 0, 0, 0 };

	//Real rotation is the sum of all relative rotations starting from the root node
	if (m_parent)
		parentReal = m_parent->GetRealRotation();
	return (parentReal + m_relativeRotation);
}

void Entity::SetParent(IEntity *parent)
{
	//Remove this entity from the childs entity of the previous parent
	if (m_parent != nullptr) m_parent->RemoveChild(this);
	//Set the new parent
	m_parent = parent;
}

void Entity::AddChild(IEntity::IEntityPtr child)
{
	//auto found = std::find(_childs.begin(), _childs.end(), child);

	//if (found != _childs.end())
	//	std::runtime_error("This entity is already a child of this entity");
	child->SetParent(this);
	//Child has same visibility as parent
	child->setVisible(m_visible);
	m_newChilds.push_back(std::move(child));
}

void Entity::RemoveChild(IEntity *child)
{
	m_childs.erase(
		std::remove_if(m_childs.begin(), m_childs.end(),
			[&child](auto &c) {
				if (c.get() == child)
					TRACE("Removing child " << child->GetLabel().c_str() << std::endl);
				return (c.get() == child);
			}),
		m_childs.end());
}

IEntity *Entity::getParent() const
{
	return m_parent;
}

std::future<void> Entity::addMesh(AObject::AObjectPtr mesh)
{
	m_mesh = std::move(mesh);
	TRACE("Create device for mesh, waiting..." << std::endl);
	co_await m_mesh->CreateDeviceDependentResourcesAsync();
	TRACE("Done waiting mesh creation" << std::endl);
	co_return;
}

void Entity::updateInGaze()
{
	//Don't calcul gaze if no mesh in entity, or not visible,
	// or if entity is the cursor
	if (m_mesh == nullptr || !m_visible || this == &(m_scene->getCursor()) || IgnoreInGaze())
	{
		m_inGaze = false;
		m_distance = -1;
		return;
	}

	auto pointerPose = m_scene->getPointerPose();

	if (pointerPose != nullptr)
	{
		// Get the gaze direction relative to the given coordinate system.
		const float3 position = pointerPose.Head().Position();
		const float3 direction = pointerPose.Head().ForwardDirection();

		XMFLOAT3 headPosition = XMFLOAT3(position.x, position.y, position.z);
		XMFLOAT3 headDirection = XMFLOAT3(direction.x, direction.y, direction.z);
		float distance = 0.0f;

		DirectX::BoundingOrientedBox currentBoundingBox;
		m_mesh->GetBoundingBox(currentBoundingBox);

		float3 extents{ currentBoundingBox.Extents.x, currentBoundingBox.Extents.y, currentBoundingBox.Extents.z };
		float3 B1 = (GetRealPosition() - extents);
		float3 B2 = (GetRealPosition() + extents);

		float3 Hit;
		float3 L1{ headPosition.x, headPosition.y, headPosition.z };
		float3 L2{ headDirection.x, headDirection.y, headDirection.z };

		bool check = CheckLineBox(B1, B2, L1, (L1 + (L2 * 6.0f)), Hit, 0.1f);

		XMVECTOR originVec = DirectX::XMLoadFloat3(&(headPosition));
		XMVECTOR hitVec = DirectX::XMLoadFloat3(&XMFLOAT3(Hit.x, Hit.y, Hit.z));
		XMVECTOR distanceV = XMVector3Length(XMVectorSubtract(originVec, hitVec));
		DirectX::XMStoreFloat(&m_distance, distanceV);

		/*TRACE("In Gaze " << GetLabel() << " " << _inGaze << " " << _distance << std::endl);*/
		m_inGaze = currentBoundingBox.Intersects(DirectX::XMLoadFloat3(&headPosition), DirectX::XMLoadFloat3(&headDirection), distance);

		//if (_inGaze)
		//{
		//	TRACE(this->GetLabel() << " in gaze " << _inGaze << " dist " << _distance << " hit " << Hit << " " << check << std::endl);
		//	TRACE("User position " << L1 << " direction " << L2 << std::endl);
		//	TRACE("Box extend is " << extents << std::endl);
		//	TRACE("Real position is " << GetRealPosition() << std::endl);
		//}

		//check is false when its colliding sometime because the extents dont reflex the orientation of the box
		/*if (check != _inGaze)
			_inGaze = false;*/
	}
	else
	{
		m_inGaze = false;
		m_distance = -1;
	}
}

std::pair<IEntity*, float> Entity::getNearestInGazeEntity()
{
	auto pair = std::make_pair(static_cast<IEntity*>(this), m_distance);

	std::for_each(m_childs.begin(), m_childs.end(),
		[&pair](auto &child)
	{
		auto newDistance = child->getNearestInGazeEntity();
		if (newDistance.first->isInGaze() && (pair.second < 0 || newDistance.second < pair.second))
		{
			pair = newDistance;
		}
	});

	return (pair);
}

void Entity::CaptureInteraction(winrt::Windows::UI::Input::Spatial::SpatialInteraction const &interaction)
{
	if (m_spatialGestureRecognizer)
		m_spatialGestureRecognizer.CaptureInteraction(interaction);
}

void Entity::SetSpatialGestureRecognizer(winrt::Windows::UI::Input::Spatial::SpatialGestureRecognizer const &recognizer)
{
	m_spatialGestureRecognizer = recognizer;
}
//
//IEntity  *Entity::RetrieveEntity(int id)
//{
//	if (this->GetID() == id && !isRoot())
//		return (this);
//	for (auto it = m_childs.begin(); it != m_childs.end(); ++it)
//	{
//		if ((*it)->GetID() == id && !(*it)->isRoot())
//			return ((*it).get());
//	}
//	return (nullptr);
//}

DirectX::BoundingOrientedBox const Entity::GetBoundingBox()
{
	DirectX::BoundingOrientedBox box{};

	//Note: Trivial right now. If child entity, should construct a bounding box surrounding all childs
	if (m_mesh)
		m_mesh->GetBoundingBox(box);
	return (box);
}

void Entity::positionInFrontOfGaze(winrt::Windows::Foundation::Numerics::float3 offsets)
{
	auto pointerPose = m_scene->getPointerPose();

	if (pointerPose != nullptr)
	{
		// Get the gaze direction relative to the given coordinate system.
		winrt::Windows::Foundation::Numerics::float3 headPosition = pointerPose.Head().Position();

		// Add offset x, y
		headPosition.x += offsets.x;
		headPosition.y += offsets.y;

		const winrt::Windows::Foundation::Numerics::float3 headDirection = pointerPose.Head().ForwardDirection();

		// The tag-along hologram follows a point 2.0m in front of the user's gaze direction.
		const winrt::Windows::Foundation::Numerics::float3 gazeAtTwoMeters = headPosition + (offsets.z * headDirection);

		//TRACE("For " << this << " translation of (" << gazeAtTwoMeters.x << ", " << gazeAtTwoMeters.y << ", " << gazeAtTwoMeters.z
		//		<< ") "<< std::endl);

		SetRealPosition(gazeAtTwoMeters);
	}
}

void Entity::rotateTowardGaze(winrt::Windows::Foundation::Numerics::float3 offsets)
{
	auto pointerPose = m_scene->getPointerPose();

	if (pointerPose != nullptr)
	{
		// Get the gaze direction relative to the given coordinate system.
		const float3 headPosition = pointerPose.Head().Position();
		const float3 realPos = GetRealPosition();
		const float3 headDirection = realPos - headPosition;
		/*const float3 headDirection = pointerPose->Head->ForwardDirection;*/

		/*TRACE(GetLabel() << " GAZE ROT POS " << headPosition << " DIR " << headDirection << " " << pointerPose->Head->ForwardDirection << std::endl);

		const float3 gazeAtTwoMeters = headPosition + (1.0f * headDirection);*/

		// Lerp the position, to keep the hologram comfortably stable.
		//auto lerpedPosition = lerp(getPosition(), gazeAtTwoMeters, dtime * c_lerpRate);

		// Create a direction normal from the hologram's position to the origin of person space.
		// This is the z-axis rotation.
		XMVECTOR facingNormal = XMVector3Normalize(-XMLoadFloat3(&headDirection));

		// Rotate the x-axis around the y-axis.
		// This is a 90-degree angle from the normal, in the xz-plane.
		// This is the x-axis rotation.
		XMVECTOR xAxisRotation = XMVector3Normalize(XMVectorSet(XMVectorGetZ(facingNormal), 0.f, -XMVectorGetX(facingNormal), 0.f));

		// Create a third normal to satisfy the conditions of a rotation matrix.
		// The cross product  of the other two normals is at a 90-degree angle to
		// both normals. (Normalize the cross product to avoid floating-point math
		// errors.)
		// Note how the cross product will never be a zero-matrix because the two normals
		// are always at a 90-degree angle from one another.
		XMVECTOR yAxisRotation = XMVector3Normalize(XMVector3Cross(facingNormal, xAxisRotation));

		float3 x, y, z;
		XMStoreFloat3(&x, xAxisRotation);
		XMStoreFloat3(&y, yAxisRotation);
		XMStoreFloat3(&z, facingNormal);

		// Construct the 4x4 rotation matrix.
		//TRACE("For " << this << " rotation of "
		//	<< "("<< x.x << ", " << x.y << ", " << x.z << ") "
		//	<< "(" << y.x << ", " << y.y << ", " << y.z << ") "
		//	<< "(" << z.x << ", " << z.y << ", " << z.z << ") "
		//	<< std::endl);

		// Rotate the quad to face the user.
		auto rotation = XMMATRIX(
			xAxisRotation,
			yAxisRotation,
			facingNormal,
			XMVectorSet(0.f, 0.f, 0.f, 1.f)
		);
		SetModelRotation(rotation);
	}
}

void Entity::setFocus(bool newFocus)
{
	auto oldFocus = m_focused;

	m_focused = newFocus;
	if (m_focused != oldFocus)
	{
		Concurrency::task<void> callbackTask = Concurrency::create_task([this]()
		{
			if (m_focused) {
				/*TRACE("Got Focus on " << GetLabel() << std::endl);*/
				OnGetFocus();
			}
			else {
				/*TRACE("Lost Focus on " << GetLabel() << std::endl);*/
				OnLostFocus();
			}
		});
	}
}

void Entity::getInGazeEntities(std::vector<IEntity*>& entities)
{
	std::for_each(m_childs.begin(), m_childs.end(),
		[&entities](auto &child)
	{
		child->getInGazeEntities(entities);
	});
	if (isInGaze())
		entities.push_back(this);
}

// UTILITIES DISPLAY FUNCTIONS

std::ostream& operator<<(std::ostream& stream, const DirectX::XMMATRIX& matrix) {
	DirectX::XMFLOAT4X4 fView;
	DirectX::XMStoreFloat4x4(&fView, matrix);
	for (int i = 0; i < 4; i++)
	{
		stream << "[";
		for (int j = 0; j < 4; j++)
		{
			stream << " " << fView.m[i][j];
		}
		stream << "]";
	}
	return (stream);
}

std::ostream& operator<<(std::ostream& stream, const winrt::Windows::Foundation::Numerics::float3 a)
{
	stream << "(" << a.x << ", " << a.y << ", " << a.z << ")";
	return (stream);
}

// -- END OF UTILITIES FUNCTIONS