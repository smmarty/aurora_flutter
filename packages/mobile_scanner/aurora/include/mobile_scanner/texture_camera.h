/**
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef TEXTURE_CAMERA_BUFFER_H
#define TEXTURE_CAMERA_BUFFER_H

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>

#include <mobile_scanner/encodable_helper.h>
#include <mobile_scanner/types/image_data.h>

#include <flutter/encodable_value.h>
#include <flutter/flutter_aurora.h>
#include <flutter/texture_registrar.h>

#include <streamcamera/streamcamera.h>

typedef flutter::TextureVariant TextureVariant;
typedef flutter::TextureRegistrar TextureRegistrar;
typedef flutter::PixelBufferTexture PixelBufferTexture;

typedef std::function<void()> CameraErrorHandler;
typedef std::function<void(const std::vector<uint8_t>& data)> TakeImageHandler;
typedef std::function<void(std::string)> ChangeQRHandler;

typedef std::shared_ptr<Aurora::StreamCamera::GraphicBuffer> GraphicBufferPtr;
typedef std::shared_ptr<const Aurora::StreamCamera::YCbCrFrame> YCbCrFrameConstPtr;

class TextureCamera : public Aurora::StreamCamera::CameraListener {
public:
    TextureCamera(flutter::TextureRegistrar* texture_registrar,
                  const CameraErrorHandler& onError,
                  const ChangeQRHandler& onChangeQR);

    void onCameraError(const std::string& errorDescription) override;
    void onCameraFrame(std::shared_ptr<Aurora::StreamCamera::GraphicBuffer> buffer) override;
    void onCameraParameterChanged(Aurora::StreamCamera::CameraParameter, const std::string& value) override;

    EncodableList GetAvailableCameras();
    EncodableList GetCameraCapabilities(const std::string & cameraId);
    EncodableMap Register(const std::string & cameraId);
    EncodableMap Unregister();
    EncodableMap StartCapture(int width, int height);
    void StopCapture();
    EncodableMap GetState();
    void GetImage(TakeImageHandler&& callback);
    EncodableMap ResizeFrame(int width, int height);
    void EnableSearchQr(bool state);

private:
    void SearchQr(std::shared_ptr<const Aurora::StreamCamera::YCbCrFrame> frame);
    bool CreateCamera(std::string cameraName);
    void SendError(std::string error);
    void ResizeFrame(int width,
                     int height,
                     Aurora::StreamCamera::CameraInfo info,
                     Aurora::StreamCamera::CameraCapability cap,
                     int& captureWidth,
                     int& captureHeight);
    YCbCrFrameConstPtr GetFrame(const GraphicBufferPtr& buffer);

    bool TakeImage(const YCbCrFrameConstPtr& frame);
    void SetImage(std::unique_ptr<Image>&& image);

private:
    TextureRegistrar* m_textures;
    TextureVariant m_textureVariant;

    std::vector<TakeImageHandler> m_takeImageHandlers;
    std::mutex m_takeImageMutex;
    std::atomic<bool> m_isTakeImage = false;

    CameraErrorHandler m_onError;
    ChangeQRHandler m_onChangeQR;
    std::string m_error;

    Aurora::StreamCamera::CameraInfo m_info;
    Aurora::StreamCamera::CameraCapability m_cap;

    std::shared_ptr<Aurora::StreamCamera::CameraManager> m_manager;
    std::shared_ptr<Aurora::StreamCamera::Camera> m_camera;
    std::shared_ptr<const Aurora::StreamCamera::YCbCrFrame> m_frame;

    int64_t m_textureId = 0;
    int m_captureWidth = 0;
    int m_captureHeight = 0;
    int m_viewWidth = 0;
    int m_viewHeight = 0;

    std::mutex m_imageDataMutex;
    std::unique_ptr<Image> m_image;
    int m_counter = 0;
    int m_counter_qr = 0;
    int m_chromaStep = 1;
    bool m_isStart = false;
    bool m_enableSearchQr = false;
};

#endif /* TEXTURE_CAMERA_BUFFER_H */
