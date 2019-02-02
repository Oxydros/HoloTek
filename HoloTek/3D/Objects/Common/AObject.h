#pragma once

#include "3D/Resources/DeviceResources.h"

///-------------------------------------------------------------------------------------------------
// namespace: HoloTek
//
// summary:	.
///-------------------------------------------------------------------------------------------------
namespace HoloTek
{
	/// <summary>	Define the properties of a simple object in the 3D world. </summary>
	class AObject : public DX::Resource
	{
	public:
		/// <summary>	Defines an alias representing zero-based index of the object pointer. </summary>
		typedef std::unique_ptr<AObject> AObjectPtr;
	public:
		AObject(std::shared_ptr<DX::DeviceResources> deviceResources) : DX::Resource(deviceResources) {}

		/// <summary>	Default destructor of the interface. </summary>
		virtual ~AObject() = default;

		/// <summary>	Render this object. </summary>
		virtual void Render() = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Force the model transform for this update loop. </summary>
		///
		/// <param name="modelTransform">	The model transform. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void SetModelTransform(DirectX::XMMATRIX const &modelTransform) = 0;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Get the BoundingBox of the mesh. </summary>
		///
		/// <param name="boundingBox">	[in,out] The bounding box. </param>
		///-------------------------------------------------------------------------------------------------
		virtual void GetBoundingBox(DirectX::BoundingOrientedBox &boundingBox) = 0;
	};
}