#include "pch.h"

#include "3D\Scene\HolographicScene.h"
#include "3D\Utility\DirectXHelper.h"
#include "3D\Objects\Mesh\3DFormes\ColoredCube.h"

using namespace HoloTek;
using namespace DirectX;
using namespace winrt::Windows::Foundation::Numerics;

ColoredCube::ColoredCube(std::shared_ptr<DX::DeviceResources> devicesResources,
	winrt::Windows::Foundation::Numerics::float3 size,
	winrt::Windows::Foundation::Numerics::float4 color
)
	: ColoredObject(devicesResources), _size(size)
{
	SetColor(color);
}

ColoredCube::~ColoredCube()
{
}

void ColoredCube::CreateMesh()
{
	float halfWidth = _size.x * 0.5f;
	float halfHeight = _size.y * 0.5f;
	float halfDepth = _size.z * 0.5f;

	m_boundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(halfWidth, halfHeight, halfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	const std::array<VertexPosition, 8> triangleVertices =
	{
		{
			{ XMFLOAT3(-halfWidth, -halfHeight, -halfDepth) },
			{ XMFLOAT3(-halfWidth, -halfHeight, halfDepth) },
			{ XMFLOAT3(-halfWidth,  halfHeight, -halfDepth) },
			{ XMFLOAT3(-halfWidth,  halfHeight,  halfDepth) },
			{ XMFLOAT3(halfWidth, -halfHeight, -halfDepth) },
			{ XMFLOAT3(halfWidth, -halfHeight,  halfDepth) },
			{ XMFLOAT3(halfWidth,  halfHeight, -halfDepth) },
			{ XMFLOAT3(halfWidth,  halfHeight,  halfDepth) },
		}
	};

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = triangleVertices.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	const CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPosition) * static_cast<UINT>(triangleVertices.size()), D3D11_BIND_VERTEX_BUFFER);
	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&vertexBufferDesc,
			&vertexBufferData,
			&m_vertexBuffer
		)
	);

	// Load mesh indices. Each trio of indices represents
	// a triangle to be rendered on the screen.
	// For example: 2,1,0 means that the vertices with indexes
	// 2, 1, and 0 from the vertex buffer compose the
	// first triangle of this mesh.
	// Note that the winding order is clockwise by default.
	constexpr std::array<unsigned short, 36> triangleIndices =
	{ {
			2,1,0, // -x
			2,3,1,

			6,4,5, // +x
			6,5,7,

			0,1,5, // -y
			0,5,4,

			2,6,7, // +y
			2,7,3,

			0,4,6, // -z
			0,6,2,

			1,3,7, // +z
			1,7,5,
		} };

	m_indexCount = static_cast<unsigned int>(triangleIndices.size());

	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = triangleIndices.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * static_cast<UINT>(triangleIndices.size()), D3D11_BIND_INDEX_BUFFER);
	winrt::check_hresult (
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&indexBufferDesc,
			&indexBufferData,
			&m_indexBuffer
		)
	);
}