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
#include "VideoFrameProcessor.h"

using namespace HoloTek;

using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Foundation::Numerics;

using namespace winrt::Windows::Media::Capture;
using namespace winrt::Windows::Media::Capture::Frames;

using namespace std::placeholders;

VideoFrameProcessor::VideoFrameProcessor(winrt::agile_ref<MediaCapture> mediaCapture, MediaFrameReader const &reader, MediaFrameSource const &source)
	: m_mediaCapture(std::move(mediaCapture))
	, m_mediaFrameReader(std::move(reader))
	, m_mediaFrameSource(std::move(source)),
	m_latestFrame(NULL)
{
	// Listen for new frames, so we know when to update our m_latestFrame
	m_mediaFrameReader.FrameArrived(std::bind(&VideoFrameProcessor::OnFrameArrived, this, _1, _2));
}

std::future<std::shared_ptr<VideoFrameProcessor>> VideoFrameProcessor::CreateAsync(void)
{
	IVectorView<MediaFrameSourceGroup> groups = co_await MediaFrameSourceGroup::FindAllAsync();

	MediaFrameSourceGroup selectedGroup = nullptr;
	MediaFrameSourceInfo selectedSourceInfo = nullptr;

	// Pick first color source.
	for (MediaFrameSourceGroup sourceGroup : groups)
	{
		for (MediaFrameSourceInfo sourceInfo : sourceGroup.SourceInfos())
		{
			if (sourceInfo.SourceKind() == MediaFrameSourceKind::Color)
			{
				selectedSourceInfo = sourceInfo;
				break;
			}
		}

		if (selectedSourceInfo != nullptr)
		{
			selectedGroup = sourceGroup;
			break;
		}
	}

	// No valid camera was found. This will happen on the emulator.
	if (selectedGroup == nullptr || selectedSourceInfo == nullptr)
	{
		co_return std::shared_ptr<VideoFrameProcessor>(nullptr);
	}

	MediaCaptureInitializationSettings settings;
	settings.MemoryPreference(MediaCaptureMemoryPreference::Cpu); // Need SoftwareBitmaps for FaceAnalysis
	settings.StreamingCaptureMode(StreamingCaptureMode::Video);   // Only need to stream video
	settings.SourceGroup(selectedGroup);

	MediaCapture nonagile_media;

	co_await nonagile_media.InitializeAsync(settings);

	MediaFrameSource selectedSource = nonagile_media.FrameSources().Lookup(selectedSourceInfo.Id());
	MediaFrameReader reader = co_await nonagile_media.CreateFrameReaderAsync(selectedSource);
	MediaFrameReaderStartStatus status = co_await reader.StartAsync();
	// Only create a VideoFrameProcessor if the reader successfully started
	if (status == MediaFrameReaderStartStatus::Success)
	{
		co_return std::make_shared<VideoFrameProcessor>(winrt::make_agile(std::move(nonagile_media)), reader, selectedSource);
	}
	else
	{
		co_return std::shared_ptr<VideoFrameProcessor>(nullptr);
	}
}

winrt::Windows::Media::Capture::Frames::MediaFrameReference VideoFrameProcessor::GetLatestFrame(void) const
{
	auto lock = std::shared_lock<std::shared_mutex>(m_propertiesLock);
	return m_latestFrame;
}

winrt::Windows::Media::Capture::Frames::VideoMediaFrameFormat VideoFrameProcessor::GetCurrentFormat(void) const
{
	return m_mediaFrameSource.CurrentFormat().VideoFormat();
}

void VideoFrameProcessor::OnFrameArrived(MediaFrameReader const &sender, MediaFrameArrivedEventArgs const &args)
{
	if (MediaFrameReference frame = sender.TryAcquireLatestFrame())
	{
		std::lock_guard<std::shared_mutex> lock(m_propertiesLock);
		m_latestFrame = frame;
	}
}
