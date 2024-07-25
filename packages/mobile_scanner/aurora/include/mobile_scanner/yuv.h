/*
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_YUV_H
#define FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_YUV_H

#include <libyuv/libyuv.h>

#include <QBuffer>
#include <QImage>
#include <QtCore>

namespace yuv {

std::vector<uint8_t> YUVToJPEG(const uint8_t* srcY,
                               const uint8_t* srcU,
                               const uint8_t* srcV,
                               int srcWidth,
                               int srcHeight,
                               int orientationDisplay,
                               int orientationCamera,
                               int direction) {
    auto angle = orientationCamera - orientationDisplay;

    if (direction < 0 && (orientationDisplay == 90 || orientationDisplay == 270)) {
        angle -= 180;
    }

    if (angle < 0) {
        angle = 360 - ((angle * -1) % 360);
    }

    if (angle >= 360) {
        angle = angle % 360;
    }

    QSize size(srcWidth, srcHeight);
    QImage image(size, QImage::Format_RGBA8888);

    if (srcV) {
        auto srcStrideY = srcWidth;
        auto srcStrideU = (srcWidth + 1) / 2;
        auto srcStrideV = srcStrideU;

        libyuv::I420ToARGB(srcY, srcStrideY, srcU, srcStrideU, srcV, srcStrideV,
                           reinterpret_cast<uint8_t*>(image.bits()), srcWidth * 4, srcWidth, srcHeight);
    } else {
        auto srcStrideY = srcWidth;
        auto srcStrideUV = srcWidth;

        libyuv::NV12ToARGB(srcY, srcStrideY,
                           srcU,  // UV
                           srcStrideUV, reinterpret_cast<uint8_t*>(image.bits()), srcWidth * 4, srcWidth, srcHeight);
    }

    QBuffer qbuffer;
    qbuffer.open(QIODevice::WriteOnly);
    image.transformed(QMatrix().rotate(angle)).save(&qbuffer, "JPEG");
    const auto& bytes = qbuffer.data();

    return std::vector<uint8_t>(bytes.constData(), bytes.constData() + bytes.size());
}

}  // namespace yuv

#endif /* FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_YUV_H */
