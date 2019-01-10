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
    class VideoFrameProcessor;

    // Class to manage the FaceAnalysis::FaceTracker object and process video frames from
    // media capture using the VideoFrameProcessor class
    class FaceTrackerProcessor : public std::enable_shared_from_this<FaceTrackerProcessor>
    {
    public:
        static std::future<std::shared_ptr<FaceTrackerProcessor>> CreateAsync(
            std::shared_ptr<VideoFrameProcessor> const &processor);

        FaceTrackerProcessor(
            winrt::Windows::Media::FaceAnalysis::FaceTracker tracker,
            std::shared_ptr<VideoFrameProcessor> processor);

        ~FaceTrackerProcessor(void);

        bool IsTrackingFaces(void) const;
        std::vector<winrt::Windows::Graphics::Imaging::BitmapBounds> GetLatestFaces(void) const;

    protected:
        void ProcessFrame(void);

		winrt::Windows::Media::FaceAnalysis::FaceTracker      m_faceTracker;
        std::shared_ptr<VideoFrameProcessor>                  m_videoProcessor;

        mutable std::shared_mutex                             m_propertiesLock;
        std::vector<winrt::Windows::Graphics::Imaging::BitmapBounds> m_latestFaces;

        uint32_t                                              m_numFramesWithoutFaces = 0;

        std::thread                                           m_workerThread;
        bool                                                  m_isRunning = false;
    };
}
