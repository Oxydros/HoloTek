#pragma once

#include "3D\Shaders\ShaderStructures.h"
#include "3D\Objects\Mesh\Common\ColoredObject.h"

using namespace winrt::Windows::Foundation::Numerics;

namespace HoloTek
{
	class ColoredCircle : public ColoredObject
	{
	public:
		ColoredCircle(std::shared_ptr<DX::DeviceResources> devicesResources,
			float radius = 0.25f,
			float radians = DirectX::XM_2PI,
			float4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
		~ColoredCircle();

	private:
		void CreateMesh() override;
		std::vector<float3> CreateCircle(float3 center, float radius, int divisions, float radians);

	private:
		float											m_radius;
		int												m_precision;
		float											m_radians;
	};
}

