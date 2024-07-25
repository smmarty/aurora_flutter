/**
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mobile_scanner/texture_camera.h>
#include <mobile_scanner/yuv.h>
#include <mobile_scanner/yuv_i420.h>
#include <mobile_scanner/yuv_nv12.h>

#include <iostream>

#include "ZXing/ReadBarcode.h"

TextureCamera::TextureCamera(TextureRegistrar* texture_registrar,
                             const CameraErrorHandler& onError,
                             const ChangeQRHandler& onChangeQR)
    : m_textures(texture_registrar),
      m_textureVariant(PixelBufferTexture([this](size_t, size_t) -> FlutterDesktopPixelBuffer* {
          std::lock_guard lock(m_imageDataMutex);
          if (!m_image) {
              return nullptr;
          }

          auto pixelBuffer = std::make_unique<FlutterDesktopPixelBuffer>();

          pixelBuffer->buffer = m_image->bits.data();
          pixelBuffer->width = m_image->width;
          pixelBuffer->height = m_image->height;
          pixelBuffer->release_callback = [](void* context) {
              auto* image = reinterpret_cast<Image*>(context);
              delete image;
          };
          pixelBuffer->release_context = m_image.release();

          return pixelBuffer.release();
      })),
      m_onError(onError),
      m_onChangeQR(onChangeQR),
      m_manager(StreamCameraManager()),
      m_camera(nullptr) {}

void TextureCamera::GetImage(TakeImageHandler&& callback) {
    std::lock_guard lock(m_takeImageMutex);
    m_takeImageHandlers.emplace_back(std::move(callback));
    m_isTakeImage = true;
}

EncodableList TextureCamera::GetAvailableCameras() {
    EncodableList cameras;
    auto count = m_manager->getNumberOfCameras();

    for (int index = 0; index < count; index++) {
        Aurora::StreamCamera::CameraInfo info;
        if (m_manager->getCameraInfo(index, info)) {
            cameras.push_back(EncodableMap{
                {"id", info.id},
                {"name", info.name},
                {"provider", info.provider},
                {"mountAngle", static_cast<std::int32_t>(info.mountAngle)},
                {"facing", static_cast<std::int32_t>(info.facing)},
            });
        }
    }

    return cameras;
}

EncodableList TextureCamera::GetCameraCapabilities(const std::string & cameraId)
{
    std::vector<Aurora::StreamCamera::CameraCapability> caps;
    m_manager->queryCapabilities(cameraId, caps);

    EncodableList result;
    for (const Aurora::StreamCamera::CameraCapability & c : caps)
    {
        result.push_back(EncodableMap{
            {"width",  c.width},
            {"height", c.height},
            {"fps",    c.fps}
        });
    }

    return result;
}

EncodableMap TextureCamera::GetState() {
    if (m_camera) {
        return EncodableMap{
            {"id", m_info.id},
            {"textureId", m_textureId},
            {"width", m_captureWidth},
            {"height", m_captureHeight},
            {"mountAngle", static_cast<std::int32_t>(m_info.mountAngle)},
            {"rotationDisplay", static_cast<std::int32_t>(aurora::GetOrientation())},
            {"error", m_error},
        };
    }

    return EncodableMap{{"error", m_error}};
}

bool TextureCamera::CreateCamera(std::string cameraName) {
    if (m_camera) {
        return true;
    }

    if (m_manager->init()) {
        if (auto count = m_manager->getNumberOfCameras()) {
            for (int index = 0; index < count; index++) {
                if (m_manager->getCameraInfo(index, m_info)) {
                    if (m_info.id == cameraName) {
                        m_camera = m_manager->openCamera(m_info.id);

                        std::vector<Aurora::StreamCamera::CameraCapability> caps;

                        if (m_camera && m_manager->queryCapabilities(m_info.id, caps)) {
                            m_cap = caps.back();
                            return true;
                        } else {
                            SendError("Stream camera error open camera");
                            return false;
                        }
                    }
                }
            }
        }
    }

    return false;
}

void TextureCamera::SendError(std::string error) {
    m_error = error;
    m_onError();
}

EncodableMap TextureCamera::StartCapture(int width, int height) {
    if (m_camera && !m_camera->captureStarted()) {
        m_viewWidth = width;
        m_viewHeight = height;

        ResizeFrame(width, height, m_info, m_cap, m_captureWidth, m_captureHeight);
        std::cout << __func__ << std::endl
                  << "\t" << m_captureWidth << std::endl
                  << "\t" << m_captureHeight << std::endl;

        m_isStart = m_camera->startCapture(m_cap);

        if (!m_isStart) {
            Unregister();
            SendError("Stream camera error start capture");
        } else {
            m_camera->setListener(this);
        }
    }

    return GetState();
}

void TextureCamera::StopCapture() {
    m_isStart = false;

    int index = 0;
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_chromaStep == 1 ? 10 : 500 /* r7 */));
        index++;
    } while (m_isTakeImage && index < 200);

    if (m_camera && m_camera->captureStarted()) {
        m_camera->stopCapture();
        m_camera->setListener(nullptr);
    }
}

EncodableMap TextureCamera::Register(const std::string & cameraId) {
    m_textureId = m_textures->RegisterTexture(&m_textureVariant);

    if (CreateCamera(cameraId) && m_viewWidth != 0) {
        StartCapture(m_viewWidth, m_viewHeight);
    }

    return GetState();
}

EncodableMap TextureCamera::Unregister() {
    if (m_camera) {
        m_isStart = false;
        m_camera->stopCapture();
        m_camera->setListener(nullptr);
        m_camera = nullptr;
    }

    m_textures->UnregisterTexture(m_textureId);

    m_error = "";
    m_counter = 0;
    m_counter_qr = 0;
    m_textureId = 0;
    m_captureWidth = 0;
    m_captureHeight = 0;

    return GetState();
}

EncodableMap TextureCamera::ResizeFrame(int width, int height) {
    if (m_isStart && !(width == m_captureWidth || height == m_captureHeight)) {
        ResizeFrame(width, height, m_info, m_cap, m_captureWidth, m_captureHeight);
    }
    return GetState();
}

void TextureCamera::ResizeFrame(int width,
                                int height,
                                Aurora::StreamCamera::CameraInfo info,
                                Aurora::StreamCamera::CameraCapability cap,
                                int& captureWidth,
                                int& captureHeight) {
    auto inWidth = width;
    auto inHeight = height;

    if (inHeight < 0) {
        if (info.mountAngle == 270 || info.mountAngle == 90) {
            inHeight = ((cap.width * inWidth) / cap.height) - 1;
        } else {
            inHeight = ((cap.height * inWidth) / cap.width) - 1;
        }
    }

    auto cw = cap.width;
    auto ch = cap.height;

    auto dw = inWidth < 500 ? 500 : inWidth + 100;
    auto dh = inHeight < 500 ? 500 : inHeight + 100;

    if (info.mountAngle == 270 || info.mountAngle == 90) {
        cw = cap.height;
        ch = cap.width;
    }

    captureHeight = dh;
    captureWidth = (cw * dh) / ch;

    if (captureWidth > dw) {
        captureWidth = dw;
        captureHeight = (ch * dw) / cw;
    }
}

YCbCrFrameConstPtr TextureCamera::GetFrame(const GraphicBufferPtr& buffer) {
    auto frame = buffer->mapYCbCr();

    if (TextureCamera::TakeImage(frame)) {
        return nullptr;
    }

    m_counter += 1;

    if (m_counter < 0) {
        m_counter = 0;
    }

    if (m_counter % 3 == 0) {
        return nullptr;
    }

    return frame;
}

void TextureCamera::onCameraFrame(std::shared_ptr<Aurora::StreamCamera::GraphicBuffer> buffer) {
    if (!m_isStart && !m_isTakeImage) {
        return;
    }

    if (const auto frame = GetFrame(buffer)) {
        m_chromaStep = frame->chromaStep;

        if (m_enableSearchQr) {
            SearchQr(frame);
        }

        if (frame->chromaStep == 1 /* I420 */) {
            auto result = yuv::I420Scale(frame->y, frame->cr, frame->cb, frame->width, frame->height, m_captureWidth,
                                         m_captureHeight);

            SetImage(yuv::I420ToARGB(result.y, result.u, result.v, result.width, result.height));
        } else if (frame->chromaStep == 2 /* NV12 */) {
            auto result =
                yuv::NV12Scale(frame->y, frame->cr, frame->width, frame->height, m_captureWidth, m_captureHeight);
            SetImage(yuv::NV12ToARGB(result.y, result.uv, result.width, result.height));
        }
    }
}

void TextureCamera::onCameraError(const std::string& errorDescription) {
    Unregister();
    SendError(errorDescription);
}

void TextureCamera::onCameraParameterChanged([[maybe_unused]] Aurora::StreamCamera::CameraParameter parameter,
                                             const std::string& value) {
    std::cout << "onCameraParameterChanged: " << value << std::endl;
}

void TextureCamera::EnableSearchQr(bool state) {
    m_enableSearchQr = state;
    m_counter_qr = 0;
}

void TextureCamera::SearchQr(std::shared_ptr<const Aurora::StreamCamera::YCbCrFrame> frame) {
    int size = frame->chromaStep == 1 ? 15 : 30;

    m_counter_qr += 1;

    if (m_counter_qr < 0) {
        m_counter_qr = 0;
    }

    if (m_counter_qr % size != 0) {
        return;
    }

    // 720p
    int width = 1280;
    int height = 720;

    std::shared_ptr<Image> image;

    if (frame->chromaStep == 1 /* I420 */) {
        auto result = yuv::I420Scale(frame->y, frame->cr, frame->cb, frame->width, frame->height, width, height);

        image = yuv::I420ToARGB(result.y, result.u, result.v, result.width, result.height);
    } else if (frame->chromaStep == 2 /* NV12 */) {
        auto result = yuv::NV12Scale(frame->y, frame->cr, frame->width, frame->height, width, height);
        image = yuv::NV12ToARGB(result.y, result.uv, result.width, result.height);
    }

    if (image) {
        auto image_view = ZXing::ImageView(image->bits.data(), width, height, ZXing::ImageFormat::RGBX);

#if PSDK_MAJOR == 5
        auto hints = ZXing::DecodeHints().setFormats(ZXing::BarcodeFormat::Any);
#else
        auto hints = ZXing::DecodeHints().setFormats(ZXing::BarcodeFormat::QR_CODE);
#endif

        auto result = ZXing::ReadBarcode(image_view, hints);
        if (result.isValid()) {
            auto ws = result.text();
            std::string text(ws.begin(), ws.end());
            m_onChangeQR(text);
        } else {
            m_onChangeQR("");
        }
    }
}

bool TextureCamera::TakeImage(const YCbCrFrameConstPtr& frame) {
    bool expected = true;
    if (!m_isTakeImage.compare_exchange_strong(expected, false)) {
        return false;
    }

    std::vector<TakeImageHandler> handlers;
    {
        std::lock_guard lock(m_takeImageMutex);
        if (m_takeImageHandlers.empty()) {
            return false;
        }
        std::swap(handlers, m_takeImageHandlers);
    }

    std::vector<uint8_t> image;
    if (frame->chromaStep == 1 /* I420 */) {
        image = yuv::YUVToJPEG(frame->y, frame->cr, frame->cb, frame->width, frame->height,
                               static_cast<int>(aurora::GetOrientation()), m_info.mountAngle,
                               m_info.id.find("front") != std::string::npos ? -1 : 1);
    } else if (frame->chromaStep == 2 /* NV12 */) {
        image = yuv::YUVToJPEG(frame->y, frame->cr, nullptr, frame->width, frame->height,
                               static_cast<int>(aurora::GetOrientation()), m_info.mountAngle,
                               m_info.id.find("front") != std::string::npos ? -1 : 1);
    }

    for (auto& it : handlers) {
        it(image);
    }

    return true;
}

void TextureCamera::SetImage(std::unique_ptr<Image>&& image) {
    std::lock_guard lock(m_imageDataMutex);
    m_image = std::move(image);
    if (m_textureId != 0) {
        m_textures->MarkTextureFrameAvailable(m_textureId);
    }
}
