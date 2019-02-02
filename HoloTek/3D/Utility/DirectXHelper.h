#pragma once

namespace DX
{
	// Function that reads from a binary file asynchronously.
	inline std::future<std::vector<byte>> ReadDataAsync(const std::wstring_view& filename)
	{
		using namespace winrt::Windows::Storage;
		using namespace winrt::Windows::Storage::Streams;

		TRACE("Trying to read file " << filename << std::endl)

		IBuffer fileBuffer = co_await PathIO::ReadBufferAsync(filename);

		TRACE("Got response " << fileBuffer.Length() << std::endl)

		std::vector<byte> returnBuffer;
		returnBuffer.resize(fileBuffer.Length());
		DataReader::FromBuffer(fileBuffer).ReadBytes(winrt::array_view<uint8_t>(returnBuffer));
		return returnBuffer;
	}

	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		constexpr float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}

	inline winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface CreateDepthTextureInteropObject(
		const Microsoft::WRL::ComPtr<ID3D11Texture2D> spTexture2D)
	{
		// Direct3D interop APIs are used to provide the buffer to the WinRT API.
		Microsoft::WRL::ComPtr<IDXGIResource1> depthStencilResource;
		winrt::check_hresult(spTexture2D.As(&depthStencilResource));
		Microsoft::WRL::ComPtr<IDXGISurface2> depthDxgiSurface;
		winrt::check_hresult(depthStencilResource->CreateSubresourceSurface(0, &depthDxgiSurface));
		winrt::com_ptr<::IInspectable> inspectableSurface;
		winrt::check_hresult(
			CreateDirect3D11SurfaceFromDXGISurface(
				depthDxgiSurface.Get(),
				winrt::put_abi(inspectableSurface)
			));

		return inspectableSurface.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface>();
	}

	HRESULT CreateWICTextureFromMemoryEx(ID3D11Device* d3dDevice,
		const uint8_t* wicData,
		size_t wicDataSize,
		size_t maxsize,
		D3D11_USAGE usage,
		unsigned int bindFlags,
		unsigned int cpuAccessFlags,
		unsigned int miscFlags,
		bool forceSRGB,
		ID3D11Resource** texture,
		ID3D11ShaderResourceView** textureView);

	HRESULT CreateWICTextureFromMemory(ID3D11Device* d3dDevice,
		const uint8_t* wicData,
		size_t wicDataSize,
		ID3D11Resource** texture,
		ID3D11ShaderResourceView** textureView,
		size_t maxsize = 0);

	// Triangulates a 2D shape, such as the spatial stage movement bounds.
	// This function expects a set of vertices that define the boundaries of a shape, in
	// clockwise order.
	std::vector<unsigned short> TriangulatePoints2DShape(std::vector<winrt::Windows::Foundation::Numerics::float3> const& vertices);

#if defined(_DEBUG)
	// Check for SDK Layer support.
	inline bool SdkLayersAvailable()
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
			0,
			D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
			nullptr,                    // Any feature level will do.
			0,
			D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Runtime apps.
			nullptr,                    // No need to keep the D3D device reference.
			nullptr,                    // No need to know the feature level.
			nullptr                     // No need to keep the D3D device context reference.
		);

		return SUCCEEDED(hr);
	}
#endif
}
