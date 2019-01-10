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

#include "pch.h"
#include "FaceTrackerProcessor.h"
#include "VideoFrameProcessor.h"

using namespace HoloTek;

using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Media;
using namespace winrt::Windows::Media::Capture;
using namespace winrt::Windows::Media::Capture::Frames;
using namespace winrt::Windows::Media::FaceAnalysis;

std::future<std::shared_ptr<FaceTrackerProcessor>> FaceTrackerProcessor::CreateAsync(std::shared_ptr<VideoFrameProcessor> const & processor)
{
	FaceTracker tracker = FaceTracker::CreateAsync().get();

	tracker.MinDetectableFaceSize(winrt::Windows::Graphics::Imaging::BitmapSize{ 64u, 64u });
	tracker.MaxDetectableFaceSize(winrt::Windows::Graphics::Imaging::BitmapSize{ 512u, 512u });
	co_return std::make_shared<FaceTrackerProcessor>(std::move(tracker), std::move(processor));
}

FaceTrackerProcessor::FaceTrackerProcessor(winrt::Windows::Media::FaceAnalysis::FaceTracker tracker, std::shared_ptr<VideoFrameProcessor> processor)
	: m_faceTracker(std::move(tracker)), m_videoProcessor(std::move(processor))
{
	if (m_videoProcessor)
	{
		// Create explicit background thread for FaceTrackingProcessing, as it is an expensive operation.
		m_workerThread = std::thread([this]
		{
			m_isRunning = true;

			while (m_isRunning)
			{
				ProcessFrame();
			}
		});
	}
}

FaceTrackerProcessor::~FaceTrackerProcessor(void)
{
	m_isRunning = false;
}

bool HoloTek::FaceTrackerProcessor::IsTrackingFaces(void) const
{
	auto lock = std::shared_lock<std::shared_mutex>(m_propertiesLock);
	return !m_latestFaces.empty();
}

std::vector<winrt::Windows::Graphics::Imaging::BitmapBounds> FaceTrackerProcessor::GetLatestFaces(void) const
{
	auto lock = std::shared_lock<std::shared_mutex>(m_propertiesLock);
	return m_latestFaces;
}

void FaceTrackerProcessor::ProcessFrame(void)
{
	if (MediaFrameReference frame = m_videoProcessor->GetLatestFrame())
	{
		if (VideoMediaFrame videoMediaFrame = frame.VideoMediaFrame())
		{
			// Validate that the incoming frame format is compatible with the FaceTracker
			if (videoMediaFrame.SoftwareBitmap() && FaceTracker::IsBitmapPixelFormatSupported(videoMediaFrame.SoftwareBitmap().BitmapPixelFormat()))
			{
				// Ask the FaceTracker to process this frame asynchronously
				winrt::Windows::Foundation::IAsyncOperation<IVector<DetectedFace>> processFrameTask = m_faceTracker.ProcessNextFrameAsync(videoMediaFrame.GetVideoFrame());

				IVector<DetectedFace> faces = processFrameTask.get();

				std::lock_guard<std::shared_mutex> lock(m_propertiesLock);

				if (faces.Size() == 0u)
				{
					++m_numFramesWithoutFaces;

					// The FaceTracker might lose track of faces for a few frames, for example,
					// if the person momentarily turns their head away from the camera. To smooth out
					// the tracking, we allow 30 video frames (~1 second) without faces before
					// we say that we're no longer tracking any faces.
					if (m_numFramesWithoutFaces > 30u && !m_latestFaces.empty())
					{
						m_latestFaces.resize(0u);
					}
				}
				else
				{
					m_numFramesWithoutFaces = 0u;
					m_latestFaces.resize(faces.Size());

					for (uint32_t i = 0u; i < faces.Size(); ++i)
					{
						m_latestFaces[i] = faces.GetAt(i).FaceBox();
					}
				}
			}
		}
	}
}
