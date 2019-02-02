#pragma once

#include "3D\Shaders\ShaderStructures.h"
#include "3D\Objects\Mesh\Common\ColoredObject.h"

namespace HoloTek
{
	class ColoredCube : public ColoredObject
	{
	private:
		winrt::Windows::Foundation::Numerics::float3           _size;

	public:
		ColoredCube(std::shared_ptr<DX::DeviceResources> devicesResources,
					winrt::Windows::Foundation::Numerics::float3 size = { 1.0f, 1.0f, 1.0f },
					winrt::Windows::Foundation::Numerics::float4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
		~ColoredCube();
		
	private:
		void CreateMesh() override;
	};
}

