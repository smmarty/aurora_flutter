/*
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_YUV_I420_H
#define FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_YUV_I420_H

#include <libyuv/libyuv.h>

namespace yuv {

struct ResultI420 {
    uint8_t* y;
    int strideY;
    uint8_t* u;
    int strideU;
    uint8_t* v;
    int strideV;
    int width;
    int height;
    std::shared_ptr<uint8_t> raw;
};

ResultI420 I420Scale(const uint8_t* srcY,
                     const uint8_t* srcU,
                     const uint8_t* srcV,
                     int srcWidth,
                     int srcHeight,
                     int outWidth,
                     int outHeight) {
    auto bufSize = (((outWidth * outHeight) + ((outWidth + 1) / 2) * ((outHeight + 1) / 2))) * 2;
    auto buf = std::shared_ptr<uint8_t>((uint8_t*)malloc(bufSize), free);

    auto y = buf.get();
    auto u = buf.get() + outWidth * outHeight;
    auto v = buf.get() + outWidth * outHeight + (outWidth * outHeight + 3) / 4;

    auto srcStrideY = srcWidth;
    auto srcStrideU = (srcWidth + 1) / 2;
    auto srcStrideV = srcStrideU;

    auto outStrideY = outWidth;
    auto outStrideU = (outWidth + 1) / 2;
    auto outStrideV = outStrideU;

    libyuv::I420Scale(srcY, srcStrideY, srcU, srcStrideU, srcV, srcStrideV, srcWidth, srcHeight, y, outStrideY, u,
                      outStrideU, v, outStrideV, outWidth, outHeight, libyuv::kFilterBilinear);

    return ResultI420{y, outStrideY, u, outStrideU, v, outStrideV, outWidth, outHeight, buf};
}

std::unique_ptr<Image> I420ToARGB(const uint8_t* srcY,
                                  const uint8_t* srcU,
                                  const uint8_t* srcV,
                                  int srcWidth,
                                  int srcHeight) {
    auto srcStrideY = srcWidth;
    auto srcStrideU = (srcWidth + 1) / 2;
    auto srcStrideV = srcStrideU;

    auto result = std::make_unique<Image>();

    result->width = srcWidth;
    result->height = srcHeight;
    result->bits.resize(srcWidth * srcHeight * 4, 0);

    libyuv::I420ToARGB(srcY, srcStrideY, srcU, srcStrideU, srcV, srcStrideV, result->bits.data(), srcWidth * 4,
                       srcWidth, srcHeight);

    return result;
}

}  // namespace yuv

#endif /* FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_YUV_I420_H */
