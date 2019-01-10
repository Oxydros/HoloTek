//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "Common\DeviceResources.h"

namespace HoloTek
{
    // Simple class to manage a NV12 texture and use it in Direct3D 11
    class NV12VideoTexture : public DX::Resource
    {
    public:
        NV12VideoTexture(
            std::shared_ptr<DX::DeviceResources> deviceResources,
            uint32_t width,
            uint32_t height);

        void CopyFromVideoMediaFrame(
            winrt::Windows::Media::Capture::Frames::VideoMediaFrame const &source);

        // DX::Resource Interface
        std::future<void> CreateDeviceDependentResourcesAsync() override;
        void ReleaseDeviceDependentResources() override;

        ID3D11Texture2D* GetTexture(void) const { return m_texture.get(); }
        ID3D11ShaderResourceView* GetLuminanceTexture(void) const { return m_luminanceView.get(); }
        ID3D11ShaderResourceView* GetChrominanceTexture(void) const { return m_chrominanceView.get(); }

    protected:
        winrt::com_ptr<ID3D11Texture2D>          m_texture;
		winrt::com_ptr<ID3D11ShaderResourceView> m_luminanceView;
		winrt::com_ptr<ID3D11ShaderResourceView> m_chrominanceView;

        uint32_t const m_width;
        uint32_t const m_height;
    };
}
