/*
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_YUV_NV12_H
#define FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_YUV_NV12_H

#include <libyuv/libyuv.h>

namespace yuv {

struct ResultNV12 {
    uint8_t* y;
    int strideY;
    uint8_t* uv;
    int strideUV;
    int width;
    int height;
    std::shared_ptr<uint8_t> raw;
};

ResultNV12 NV12Scale(const uint8_t* srcY,
                     const uint8_t* srcUV,
                     int srcWidth,
                     int srcHeight,
                     int outWidth,
                     int outHeight) {
    auto bufSize = (((outWidth * outHeight) + ((outWidth + 1) / 2) * ((outHeight + 1) / 2))) * 2;
    auto buf = std::shared_ptr<uint8_t>((uint8_t*)malloc(bufSize), free);

    auto y = buf.get();
    auto uv = buf.get() + outWidth * outHeight;

    auto srcStrideY = srcWidth;
    auto srcStrideUV = srcWidth;

    auto outStrideY = outWidth;
    auto outStrideUV = outWidth;

    libyuv::NV12Scale(srcY, srcStrideY, srcUV, srcStrideUV, srcWidth, srcHeight, y, outStrideY, uv, outStrideUV,
                      outWidth, outHeight, libyuv::kFilterBilinear);

    return ResultNV12{y, outStrideY, uv, outStrideUV, outWidth, outHeight, buf};
}

std::unique_ptr<Image> NV12ToARGB(const uint8_t* srcY, const uint8_t* srcUV, int srcWidth, int srcHeight) {
    auto srcStrideY = srcWidth;
    auto srcStrideUV = srcWidth;

    auto result = std::make_unique<Image>();

    result->width = srcWidth;
    result->height = srcHeight;
    result->bits.resize(srcWidth * srcHeight * 4, 0);

    libyuv::NV12ToARGB(srcY, srcStrideY, srcUV, srcStrideUV, result->bits.data(), srcWidth * 4, srcWidth, srcHeight);

    return result;
}

}  // namespace yuv

#endif /* FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_YUV_NV12_H */
