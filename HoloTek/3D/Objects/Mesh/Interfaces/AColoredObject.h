#pragma once

#include <3D\Objects\Common\AObject.h>

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{
	// Define the properties of a colored object
	class AColoredObject : public AObject
	{
	public:
		AColoredObject(std::shared_ptr<DX::DeviceResources> deviceResources) : AObject(deviceResources) {}
		virtual ~AColoredObject() = default;

	public:
		// Set the color of the object
		virtual void SetColor(float4 color) = 0;

		// Get the color of the object
		virtual float4 GetColor() const = 0;
	};
}