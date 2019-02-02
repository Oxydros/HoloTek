#include <pch.h>

#include "3D\Utility\DirectXHelper.h"
#include "3D\Objects\GUI\Text\TextObject.h"
#include "3D\Utility\CFW1StateSaver.h"

using namespace DirectX;
using namespace winrt::Windows::Foundation::Numerics;

HoloTek::TextObject::TextObject(std::shared_ptr<DX::DeviceResources> deviceResources,
	winrt::Windows::Foundation::Numerics::float2 size,
	winrt::Windows::Foundation::Numerics::float4 color,
	std::wstring const &text)
	: ATextObject(deviceResources), m_text(text), m_color(color), m_size(size)
{
	/*constexpr unsigned int blurTargetWidth = 256;
	_distanceFieldRenderer = std::make_unique<DistanceFieldRenderer>(_deviceResources, blurTargetWidth, blurTargetWidth);*/
}

std::future<void> HoloTek::TextObject::CreateDeviceDependentResourcesAsync()
{
	/*FW1FontWrapper::CFW1StateSaver saver;
	saver.saveCurrentState(_deviceResources->GetD3DDeviceContext());*/
	m_textRenderer = std::make_unique<TextRenderer>(m_deviceResources, m_size.x, m_size.y);

	co_await m_textRenderer->CreateDeviceDependentResourcesAsync();
	//_distanceFieldRenderer->CreateDeviceDependentResources();

	m_textRenderer->RenderTextOffscreen(m_text, m_fontSize);
	m_quadTextureView = m_textRenderer->GetTexture();

	//saver.restoreSavedState();

	m_usingVprtShaders = m_deviceResources->GetDeviceSupportsVprt();

	// If the optional VPRT feature is supported by the graphics device, we
	// can avoid using geometry shaders to set the render target array index.
	std::wstring vertexShaderFileName = m_usingVprtShaders ? L"ms-appx:///VPRTVertexShaderTexture.cso" : L"ms-appx:///VertexShaderTexture.cso";

	std::vector<byte> vertexData = co_await DX::ReadDataAsync(vertexShaderFileName);

	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreateVertexShader(
			vertexData.data(),
			vertexData.size(),
			nullptr,
			&m_vertexShader
		)
	);

	constexpr std::array<D3D11_INPUT_ELEMENT_DESC, 2> vertexDesc =
	{ {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,							0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	} };

	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreateInputLayout(
			vertexDesc.data(),
			vertexDesc.size(),
			vertexData.data(),
			vertexData.size(),
			&m_inputLayout
		)
	);

	std::vector<byte> pixelData = co_await DX::ReadDataAsync(L"ms-appx:///PixelShaderTexture.cso");

	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreatePixelShader(
			pixelData.data(),
			pixelData.size(),
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

	std::vector<byte> geoData;
	if (!m_usingVprtShaders)
	{
		// Load the pass-through geometry shader.
		geoData = co_await DX::ReadDataAsync(L"ms-appx:///GeometryShaderTexture.cso");

		winrt::check_hresult(
			m_deviceResources->GetD3DDevice()->CreateGeometryShader(
				geoData.data(),
				geoData.size(),
				nullptr,
				&m_geometryShader
			)
		);
	}
	float halfWidth = m_size.x * 0.5f;
	float halfHeight = m_size.y * 0.5f;

	m_boundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(halfWidth, halfHeight, 0.001f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	// Load mesh vertices. Each vertex has a position and a color.
	// Note that the quad size has changed from the default DirectX app
	// template. Windows Holographic is scaled in meters, so to draw the
	// quad at a comfortable size we made the quad width 0.2 m (20 cm).
	const std::array<VertexPositionTex, 4> quadVertices =
	{ {
		{ XMFLOAT3(-halfWidth, halfHeight, 0.f), XMFLOAT2(0.f, 0.f) },
		{ XMFLOAT3(halfWidth,  halfHeight, 0.f), XMFLOAT2(1.f, 0.f) },
		{ XMFLOAT3(halfWidth, -halfHeight, 0.f), XMFLOAT2(1.f, 1.f) },
		{ XMFLOAT3(-halfWidth, -halfHeight, 0.f), XMFLOAT2(0.f, 1.f) },
	} };

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = quadVertices.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	const CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPositionTex) * quadVertices.size(), D3D11_BIND_VERTEX_BUFFER);

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
	static constexpr std::array<unsigned short, 12> quadIndices =
	{ {
			// -z
			0,2,3,
			0,1,2,

			// +z
			2,0,3,
			1,0,2,
		} };

	m_indexCount = quadIndices.size();

	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = quadIndices.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	const CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * quadIndices.size(), D3D11_BIND_INDEX_BUFFER);
	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&indexBufferDesc,
			&indexBufferData,
			&m_indexBuffer
		)
	);

	//DX::ThrowIfFailed(CreateDDSTextureFromFile(
	//	_deviceResources->GetD3DDevice(),
	//	L"ms-appx:///Assets//placeholder.dds",
	//	_quadTexture.GetAddressOf(),
	//	_quadTextureView.GetAddressOf()
	//)
	//);

	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));

	desc.Filter = D3D11_FILTER_ANISOTROPIC;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MaxAnisotropy = 3;
	desc.MinLOD = 0;
	desc.MaxLOD = 3;
	desc.MipLODBias = 0.f;
	desc.BorderColor[0] = 0.f;
	desc.BorderColor[1] = 0.f;
	desc.BorderColor[2] = 0.f;
	desc.BorderColor[3] = 0.f;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	winrt::check_hresult(
		m_deviceResources->GetD3DDevice()->CreateSamplerState(
			&desc,
			&m_quadTextureSamplerState
		)
	);

	// After the assets are loaded, the quad is ready to be rendered.
	m_loadingComplete = true;
	co_return;
}

void HoloTek::TextObject::ReleaseDeviceDependentResources()
{
	m_textRenderer->ReleaseDeviceDependentResources();
	/*_distanceFieldRenderer->ReleaseDeviceDependentResources();*/
	m_loadingComplete = false;
	m_usingVprtShaders = false;

	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_geometryShader.Reset();

	m_modelConstantBuffer.Reset();

	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();

	//_quadTexture.Reset();
	m_quadTextureView.Reset();
	m_quadTextureSamplerState.Reset();
}


void HoloTek::TextObject::Render()
{
	// Loading is asynchronous. Resources must be created before drawing can occur.
	if (!m_loadingComplete)
	{
		return;
	}

	if (m_updateText)
	{
		m_textRenderer->RenderTextOffscreen(m_text, m_fontSize);
		m_quadTextureView = m_textRenderer->GetTexture();
		m_updateText = false;
	}

	const auto context = m_deviceResources->GetD3DDeviceContext();

	m_modelConstantBufferData.color = XMFLOAT4(m_color.x, m_color.y, m_color.z, 0);

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
	const UINT stride = sizeof(VertexPositionTex);
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
		// a pass-through geometry shader sets the render target ID.
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
	context->PSSetShaderResources(
		0,
		1,
		//_texture->getTextureView().GetAddressOf()
		m_quadTextureView.GetAddressOf()
	);
	context->PSSetSamplers(
		0,
		1,
		m_quadTextureSamplerState.GetAddressOf()
	);

	// Draw the objects.
	context->DrawIndexedInstanced(
		m_indexCount,   // Index count per instance.
		2,              // Instance count.
		0,              // Start index location.
		0,              // Base vertex location.
		0               // Start instance location.
	);
}

void HoloTek::TextObject::SetModelTransform(DirectX::XMMATRIX const & modelTransform)
{
	m_modelTransform = modelTransform;
	XMStoreFloat4x4(&m_transform, m_modelTransform);
	XMStoreFloat4x4(&m_modelConstantBufferData.model, XMMatrixTranspose(m_modelTransform));
}

void HoloTek::TextObject::GetBoundingBox(DirectX::BoundingOrientedBox & boundingBox)
{
	m_boundingBox.Transform(boundingBox, DirectX::XMLoadFloat4x4(&m_transform));
}