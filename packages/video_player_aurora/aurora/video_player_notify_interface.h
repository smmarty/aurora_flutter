// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

//******************************************************************************
//******************************************************************************

#ifndef __VIDEO_PLAYER_NOTIFY_INTERFACE_H__
#define __VIDEO_PLAYER_NOTIFY_INTERFACE_H__

//******************************************************************************
//******************************************************************************
class NotifyInterface 
{
public:
    // Notifies the completion of initializing the video player.
    virtual void OnNotifyInitialized(int64_t textureId) = 0;

    // Notifies the completion of decoding a video frame.
    virtual void OnNotifyFrameDecoded(int64_t textureId) = 0;

    // Notifies the completion of playing a video.
    virtual void OnNotifyCompleted(int64_t textureId) = 0;

    // Notifies update of playing or pausing a video.
    virtual void OnNotifyPlaying(int64_t textureId, bool is_playing) = 0;
};

#endif  // __VIDEO_PLAYER_NOTIFY_INTERFACE_H__
