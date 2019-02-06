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

namespace HoloTek
{
    // Class to manage receiving video frames from Windows::Media::Capture
    class VideoFrameProcessor : std::enable_shared_from_this<VideoFrameProcessor>
    {
    public:
        static std::future<std::shared_ptr<VideoFrameProcessor>> CreateAsync(void);

        VideoFrameProcessor(
            winrt::agile_ref<winrt::Windows::Media::Capture::MediaCapture> mediaCapture,
            winrt::Windows::Media::Capture::Frames::MediaFrameReader const &reader,
            winrt::Windows::Media::Capture::Frames::MediaFrameSource const &source);

        winrt::Windows::Media::Capture::Frames::MediaFrameReference GetLatestFrame(void) const;
        winrt::Windows::Media::Capture::Frames::VideoMediaFrameFormat GetCurrentFormat(void) const;

    protected:
        void OnFrameArrived(
			winrt::Windows::Media::Capture::Frames::MediaFrameReader const &sender,
			winrt::Windows::Media::Capture::Frames::MediaFrameArrivedEventArgs const &args);

		winrt::agile_ref<winrt::Windows::Media::Capture::MediaCapture> m_mediaCapture;
		winrt::Windows::Media::Capture::Frames::MediaFrameReader     m_mediaFrameReader;

        mutable std::shared_mutex                              m_propertiesLock;
		winrt::Windows::Media::Capture::Frames::MediaFrameSource     m_mediaFrameSource;
		winrt::Windows::Media::Capture::Frames::MediaFrameReference  m_latestFrame;
    };
}
