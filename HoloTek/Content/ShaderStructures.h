#pragma once

namespace HoloTek
{
    //// Constant buffer used to send hologram position transform to the shader pipeline.
    //struct ModelConstantBuffer
    //{
    //    DirectX::XMFLOAT4X4 model;
    //};

    //// Assert that the constant buffer remains 16-byte aligned (best practice).
    //static_assert((sizeof(ModelConstantBuffer) % (sizeof(float) * 4)) == 0, "Model constant buffer size must be 16-byte aligned (16 bytes is the length of four floats).");


    //// Used to send per-vertex data to the vertex shader.
    //struct VertexPositionColor
    //{
    //    DirectX::XMFLOAT3 pos;
    //    DirectX::XMFLOAT3 color;
    //};

		// Constant buffers used to send hologram position transform to the shader pipeline.
	struct CubeModelConstantBuffer
	{
		winrt::Windows::Foundation::Numerics::float4x4 model;
	};

	// Assert that the constant buffer remains 16-byte aligned (best practice).
	static_assert((sizeof(CubeModelConstantBuffer) % (sizeof(float) * 4)) == 0, "Model constant buffer size must be 16-byte aligned (16 bytes is the length of four floats).");

	struct QuadModelConstantBuffer
	{
		winrt::Windows::Foundation::Numerics::float4x4 model;
		winrt::Windows::Foundation::Numerics::float2 texCoordScale;
		winrt::Windows::Foundation::Numerics::float2 texCoordOffset;
	};

	// Assert that the constant buffer remains 16-byte aligned (best practice).
	static_assert((sizeof(QuadModelConstantBuffer) % (sizeof(float) * 4)) == 0, "Model constant buffer size must be 16-byte aligned (16 bytes is the length of four floats).");

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		winrt::Windows::Foundation::Numerics::float3 pos;
		winrt::Windows::Foundation::Numerics::float3 color;
	};

	struct VertexPositionTex
	{
		winrt::Windows::Foundation::Numerics::float3 pos;
		winrt::Windows::Foundation::Numerics::float2 tex;
	};
}