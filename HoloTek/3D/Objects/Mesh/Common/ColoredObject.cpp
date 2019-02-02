#include "pch.h"
#include "3D\Objects\Mesh\Common\ColoredObject.h"

using namespace HoloTek;
using namespace DirectX;

ColoredObject::ColoredObject(std::shared_ptr<DX::DeviceResources> deviceResources,
			std::wstring const &vShader, std::wstring const &pShader, std::wstring const &gShader,
			std::wstring const &vpVShader)
	: AColoredObject(std::move(deviceResources)), m_vertexShaderString(vShader), m_pixelShaderString(pShader),
	m_geometryShaderString(gShader), m_vprtVertexShaderString(vpVShader)
{
}

ColoredObject::~ColoredObject()
{}

std::future<void> ColoredObject::CreateDeviceDependentResourcesAsync()
{
	TRACE("Begin create ressource in colored object" << std::endl)
	co_await InitializeShaders();
	CreateMesh();
	m_loadingComplete = true;
	TRACE("finished create ressource" << std::endl)
	co_return;
}

std::future<void> ColoredObject::InitializeShaders()
{
	m_usingVprtShaders = m_deviceResources->GetDeviceSupportsVprt();

	std::wstring vertexShaderFileName = m_usingVprtShaders ? m_vprtVertexShaderString : m_vertexShaderString;

	TRACE("Init shaders :" << std::endl
		<< "\t" << vertexShaderFileName << std::endl
		<< "\t" << m_pixelShaderString << std::endl
		<< "\t" << m_geometryShaderString << std::endl);

	// Load shaders asynchronously.
	std::vector<byte> vertexShaderFileData = co_await DX::ReadDataAsync(vertexShaderFileName);

	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreateVertexShader(
			vertexShaderFileData.data(),
			vertexShaderFileData.size(),
			nullptr,
			&m_vertexShader
		)
	);

	constexpr std::array<D3D11_INPUT_ELEMENT_DESC, 1> vertexDesc =
	{ {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		} };

	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreateInputLayout(
			vertexDesc.data(),
			static_cast<UINT>(vertexDesc.size()),
			vertexShaderFileData.data(),
			static_cast<UINT>(vertexShaderFileData.size()),
			&m_inputLayout
		)
	);


	std::vector<byte> pixelShaderFileData = co_await DX::ReadDataAsync(m_pixelShaderString);

	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreatePixelShader(
			pixelShaderFileData.data(),
			pixelShaderFileData.size(),
			nullptr,
			&m_pixelShader
		)
	);

	const CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ColorModelConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			&m_modelConstantBuffer
		)
	);

	if (!m_usingVprtShaders)
	{
		// Load the pass-through geometry shader.
		std::vector<byte> geometryShaderFileData = co_await DX::ReadDataAsync(m_geometryShaderString);
		winrt::check_hresult(
			m_deviceResources->GetD3DDevice()->CreateGeometryShader(
				geometryShaderFileData.data(),
				geometryShaderFileData.size(),
				nullptr,
				&m_geometryShader
			)
		);
	}
	co_return;
}

void ColoredObject::GetBoundingBox(DirectX::BoundingOrientedBox & boundingBox)
{
	m_boundingBox.Transform(boundingBox, DirectX::XMLoadFloat4x4(&m_transform));
}

// Renders one frame using the vertex and pixel shaders.
// On devices that do not support the D3D11_FEATURE_D3D11_OPTIONS3::
// VPAndRTArrayIndexFromAnyShaderFeedingRasterizer optional feature,
// a pass-through geometry shader is also used to set the render 
// target array index.
void ColoredObject::Render()
{
	// Only render when vertex and mesh are loaded
	if (!m_loadingComplete)
		return;
	const auto context = m_deviceResources->GetD3DDeviceContext();

	m_modelConstantBufferData.color = XMFLOAT4(m_color.x, m_color.y, m_color.z, m_color.w);

	// Update the model transform buffer for the hologram.
	context->UpdateSubresource(
		m_modelConstantBuffer.Get(),
		0,
		nullptr,
		&m_modelConstantBufferData,
		0,
		0
	);

	// Each vertex is one instance of the VertexPositionColor struct.
	const UINT stride = sizeof(VertexPosition);
	const UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
	);
	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach the vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
	);
	// Apply the model constant buffer to the vertex shader.
	context->VSSetConstantBuffers(
		0,
		1,
		m_modelConstantBuffer.GetAddressOf()
	);

	if (!m_usingVprtShaders)
	{
		// On devices that do not support the D3D11_FEATURE_D3D11_OPTIONS3::
		// VPAndRTArrayIndexFromAnyShaderFeedingRasterizer optional feature,
		// a pass-through geometry shader is used to set the render target 
		// array index.
		context->GSSetShader(
			m_geometryShader.Get(),
			nullptr,
			0
		);
	}

	// Attach the pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
	);

	// Draw the objects.
	// Two instances: one for each eye
	context->DrawIndexedInstanced(
		m_indexCount,   // Index count per instance.
		2,              // Instance count.
		0,              // Start index location.
		0,              // Base vertex location.
		0               // Start instance location.
	);
}

void ColoredObject::SetModelTransform(DirectX::XMMATRIX const &modelTransform)
{
	m_modelTransform = modelTransform;
	XMStoreFloat4x4(&m_transform, m_modelTransform);
	XMStoreFloat4x4(&m_modelConstantBufferData.model, XMMatrixTranspose(m_modelTransform));
	/*_useForcedTransform = true;*/
}

void ColoredObject::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_usingVprtShaders = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_geometryShader.Reset();
	m_modelConstantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}