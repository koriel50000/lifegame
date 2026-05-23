#pragma once

#include <assert.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <hls_vector.h>
#include <ap_int.h>
#include "window_buffer.hpp"

// @see ug1399, HLS Programmers Guide > Customizing-AXI4-Stream-Interfaces
using axis_data = ap_axis<64, 0, 0, 0, (AXIS_ENABLE_DATA | AXIS_ENABLE_LAST), true>;

const int SIZE = 128;
const int KERNEL = 3;

using data_t = uint64_t;
using block_data_t = data_t[SIZE * SIZE];
using win_t = hls::vector<data_t, KERNEL * KERNEL>;
using linebuf_t = LineBuffer<256, KERNEL, data_t, win_t>; // 128 < SIZE + 2 < 256
template <typename T>
using fifo = hls::stream<T>;

extern "C" {
void lifegame(fifo<axis_data>& in, fifo<axis_data>& out);
}
