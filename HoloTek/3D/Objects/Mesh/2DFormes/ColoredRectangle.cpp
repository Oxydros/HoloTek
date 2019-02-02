#include "pch.h"

#include "3D\Utility\DirectXHelper.h"
#include "3D\Objects\Mesh\2DFormes\ColoredRectangle.h"

using namespace HoloTek;
using namespace DirectX;
using namespace winrt::Windows::Foundation::Numerics;

ColoredRectangle::ColoredRectangle(std::shared_ptr<DX::DeviceResources> devicesResources,
								float2 size,
								float4 color)
	: ColoredObject(std::move(devicesResources)), m_size(size)
{
	SetColor(color);
	/*SetPosition({ 0.0f, 0.0f, -2.0f });*/
}

ColoredRectangle::~ColoredRectangle()
{
}

void ColoredRectangle::CreateMesh()
{

	float halfWidth = m_size.x * 0.5f;
	float halfHeight = m_size.y * 0.5f;

	m_boundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(halfWidth, halfHeight, 0.001f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	// Load mesh vertices. Each vertex has a position and a texture UV.
	const std::array<VertexPosition, 4> rectangleVertices =
	{
		{
			{ XMFLOAT3(-halfWidth, -halfHeight,  0.0f) }, // 0 (1)
			{ XMFLOAT3(-halfWidth,  halfHeight,  0.0f) }, // 1 (3)
			{ XMFLOAT3(halfWidth, -halfHeight,  0.0f) }, // 2 (5)
			{ XMFLOAT3(halfWidth,  halfHeight,  0.0f) }, // 3 (7)
		}
	};

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = rectangleVertices.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	const CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPosition) * static_cast<UINT>(rectangleVertices.size()), D3D11_BIND_VERTEX_BUFFER);
	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&vertexBufferDesc,
			&vertexBufferData,
			&m_vertexBuffer
		)
	);

	static constexpr unsigned short rectangleIndices[] =
	{
		0,1,3,
		0,3,2,
	};

	m_indexCount = ARRAYSIZE(rectangleIndices);

	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = rectangleIndices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(rectangleIndices), D3D11_BIND_INDEX_BUFFER);
	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&indexBufferDesc,
			&indexBufferData,
			&m_indexBuffer
		)
	);
}