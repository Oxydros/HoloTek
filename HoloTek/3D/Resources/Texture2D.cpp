#include <pch.h>
#include <wrl\client.h>

#include <3D\Resources\Texture2D.h>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace winrt::Windows::Foundation;

HoloTek::Texture2D::Texture2D(std::shared_ptr<DX::DeviceResources> &deviceResources,
	std::string const &fileName)
	: _deviceResources(deviceResources), _fileName(fileName)
{
}

HoloTek::Texture2D::~Texture2D()
{

}

ID3D11Texture2D *HoloTek::Texture2D::getTexture() const
{
	return _texture.get();
}

ID3D11ShaderResourceView *HoloTek::Texture2D::getTextureView() const
{
	return _textureView.get();
}

std::future<void> HoloTek::Texture2D::Load()
{
	std::vector<byte> fileData = co_await DX::ReadDataAsync(std::wstring(_fileName.begin(), _fileName.end()));

	winrt::check_hresult(
		DX::CreateWICTextureFromMemory(
			_deviceResources->GetD3DDevice(),
			&fileData[0],
			fileData.size(),
			_textureResource.put(),
			nullptr));	
	winrt::check_hresult(_texture.as<ID3D11Texture2D>()->QueryInterface(IID_PPV_ARGS(_texture.put())));
	D3D11_TEXTURE2D_DESC texDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	_texture->GetDesc(&texDesc);
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MostDetailedMip = 0;
	viewDesc.Texture2D.MipLevels = 1;

	winrt::check_hresult(_deviceResources->GetD3DDevice()->CreateShaderResourceView(_textureResource.get(), &viewDesc, _textureView.put()));
}
