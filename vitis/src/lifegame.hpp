#pragma once

#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <ap_int.h>

typedef ap_axis<32, 0, 0, 0> pkt;
typedef ap_uint<hls::bytewidth<ap_int<32>>> ctrl;

const int SIZE = 128;

extern "C" {
void lifegame(hls::stream<pkt>& in, hls::stream<pkt>& out);
}
