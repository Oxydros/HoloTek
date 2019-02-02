#pragma once

#include "3D\Shaders\ShaderStructures.h"
#include "3D\Objects\Mesh\Common\ColoredObject.h"

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{
	class ColoredRectangle : public ColoredObject
	{
	public:
		ColoredRectangle(std::shared_ptr<DX::DeviceResources> devicesResources,
			winrt::Windows::Foundation::Numerics::float2 size,
			winrt::Windows::Foundation::Numerics::float4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
		~ColoredRectangle();

	private:
		void CreateMesh() override;

	private:
		winrt::Windows::Foundation::Numerics::float2	m_size;
	};
}