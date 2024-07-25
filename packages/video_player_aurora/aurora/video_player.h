// Copyright 2023 Alexander Syrykh. All rights reserved.
// Copyright 2021 Sony Group Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_AURORA_GST_VIDEO_PLAYER_H_
#define PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_AURORA_GST_VIDEO_PLAYER_H_

#include <gst/gst.h>

#ifdef USE_EGL_IMAGE_DMABUF
#include <gst/allocators/gstdmabuf.h>
#include <gst/gl/egl/egl.h>
#include <gst/gl/gl.h>
#include <gst/video/video.h>
#endif  // USE_EGL_IMAGE_DMABUF

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>

#include "video_player_notify_interface.h"

//******************************************************************************
//******************************************************************************
class GstVideoPlayer 
{
public:

    //
    GstVideoPlayer(const std::string& uri,
                    const int64_t textireId,
                    NotifyInterface * handler);
    
    //
    ~GstVideoPlayer();

    //
    static void GstLibraryLoad();
    
    //
    static void GstLibraryUnload();

    //
    bool Play();
    
    //
    bool Pause();
    
    //
    bool Stop();
    
    //
    bool SetVolume(double volume);
    
    //
    bool SetPlaybackRate(double rate);
    
    //
    void SetAutoRepeat(bool auto_repeat);
    
    //
    bool SetSeek(int64_t position);
    
    //
    int64_t GetDuration();
    
    //
    int64_t GetCurrentPosition();
    
    //
    const uint8_t * GetFrameBuffer();

#ifdef USE_EGL_IMAGE_DMABUF
    void* GetEGLImage(void* egl_display, void* egl_context);
#endif  // USE_EGL_IMAGE_DMABUF

    //
    int32_t GetWidth() const;
    
    //
    int32_t GetHeight() const;

private:
    class impl;
    std::shared_ptr<impl> m_p;
};

#endif  // PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_AURORA_GST_VIDEO_PLAYER_H_