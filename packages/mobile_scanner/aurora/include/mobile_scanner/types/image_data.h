/**
 * SPDX-FileCopyrightText: Copyright 2024 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <vector>

struct Image {
    std::size_t width = 0;
    std::size_t height = 0;
    std::vector<std::uint8_t> bits;
};

#endif
