///-------------------------------------------------------------------------------------------------
// file:	3D\Resources\Texture2D.h
//
// summary:	Declares the texture 2D class
///-------------------------------------------------------------------------------------------------
#pragma once

#include <d3d11.h>
#include <3D\Resources\DeviceResources.h>
#include <3D\Utility\DirectXHelper.h>


///-------------------------------------------------------------------------------------------------
// namespace: HoloLensClient
//
// summary:	.
///-------------------------------------------------------------------------------------------------
namespace HoloTek
{
	/// <summary>	A texture 2d. </summary>
	class Texture2D : public std::enable_shared_from_this<Texture2D>
	{
	private:
		/// <summary>	The texture. </summary>
		winrt::com_ptr<ID3D11Texture2D>				_texture;
		/// <summary>	The texture resource. </summary>
		winrt::com_ptr<ID3D11Resource>				_textureResource;
		/// <summary>	The texture view. </summary>
		winrt::com_ptr<ID3D11ShaderResourceView>	_textureView;
		/// <summary>	The device resources. </summary>
		std::shared_ptr<DX::DeviceResources>				_deviceResources;
		/// <summary>	Filename of the file. </summary>
		std::string											_fileName;

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Constructor. </summary>
		///
		/// <param name="deviceResources">	[in,out] The device resources. </param>
		/// <param name="fileName">		  	Filename of the file. </param>
		///-------------------------------------------------------------------------------------------------
		Texture2D(std::shared_ptr<DX::DeviceResources> &deviceResources, std::string const &fileName);
		/// <summary>	Destructor. </summary>
		~Texture2D();

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets the texture. </summary>
		///
		/// <returns>	The texture. </returns>
		///-------------------------------------------------------------------------------------------------
		ID3D11Texture2D *getTexture() const;

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets texture view. </summary>
		///
		/// <returns>	The texture view. </returns>
		///-------------------------------------------------------------------------------------------------
		ID3D11ShaderResourceView *getTextureView() const;

	public:

		///-------------------------------------------------------------------------------------------------
		/// <summary>	Gets the load. </summary>
		///
		/// <returns>	A Concurrency::task&lt;void&gt; </returns>
		///-------------------------------------------------------------------------------------------------
		std::future<void> Load();
	};
}