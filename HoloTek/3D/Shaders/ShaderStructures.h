#pragma once

namespace HoloTek
{
	// Constant buffers used to send hologram position transform to the shader pipeline.
	struct CubeModelConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
	};

	// Assert that the constant buffer remains 16-byte aligned (best practice).
	static_assert((sizeof(CubeModelConstantBuffer) % (sizeof(float) * 4)) == 0, "Model constant buffer size must be 16-byte aligned (16 bytes is the length of four floats).");

	struct ColorModelConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4 color;
	};

	// Assert that the constant buffer remains 16-byte aligned (best practice).
	static_assert((sizeof(CubeModelConstantBuffer) % (sizeof(float) * 4)) == 0, "Model constant buffer size must be 16-byte aligned (16 bytes is the length of four floats).");


	struct QuadModelConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT2 texCoordScale;
		DirectX::XMFLOAT2 texCoordOffset;
	};

	// Assert that the constant buffer remains 16-byte aligned (best practice).
	static_assert((sizeof(QuadModelConstantBuffer) % (sizeof(float) * 4)) == 0, "Model constant buffer size must be 16-byte aligned (16 bytes is the length of four floats).");

	struct VertexPosition
	{
		DirectX::XMFLOAT3 pos;
	};


	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct VertexPositionTex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
	};
}