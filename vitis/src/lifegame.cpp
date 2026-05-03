#include "lifegame.hpp"

int live_or_die(win_t& winb) {
	int nb = ((winb[0] + winb[1]) + (winb[2] + winb[3])) + ((winb[5] + winb[6]) + (winb[7] + winb[8]));
	int tt = nb ^ 0x03;
	return (tt == 0 || (tt ^ winb[4]) == 0) ? 1 : 0;
}

void stream_to_array(fifo<axis_data>& ins, block_data_t& outb) {
	stream_to_array_hw: for (int i = 0; i < SIZE * SIZE; i++) {
		axis_data pkt = ins.read();
		outb[i] = pkt.data;
	}
}

void windowize_input(block_data_t& inb, fifo<win_t>& outs) {
	linebuf_t linebuf;
	linebuf.reset(SIZE + 2);

	int ptr = 0;
	windowize_h: for (int y = -1; y < SIZE + 1; y++) {
		windowize_w: for (int x = -1; x < SIZE + 1; x++) {
			// input
			data_t val;
			if (0 <= x && x < SIZE && 0 <= y && y < SIZE) {
				val = inb[ptr++];
			} else {
				val = 0;
			}
			// buffering
			if (y < 1) {
				linebuf.insert_linebuf(val);
			} else {
				linebuf.slide_window(val);
			}
 			// output
			if (1 <= x && 1 <= y) {
				win_t winb = linebuf.get_window();
				outs.write(winb);
			}
		}
	}
}

void compute_output(fifo<win_t>& ins, fifo<axis_data>& outs) {
	compute_hw: for (int i = 0; i < SIZE * SIZE; i++) {
		win_t winb = ins.read();
		axis_data pkt;
		pkt.data = live_or_die(winb);
		pkt.keep = 0xff;
		pkt.strb = 0xff;
		pkt.last = (i == SIZE * SIZE / 2 - 1 || i == SIZE * SIZE - 1) ? 1 : 0;
		outs.write(pkt);
	}
}

void lifegame(fifo<axis_data>& in, fifo<axis_data>& out) {
#pragma HLS interface axis port=in
#pragma HLS interface axis port=out
#pragma HLS interface ap_ctrl_none port=return

	fifo<win_t> pips("pipe_fifo");
#pragma HLS stream variable=pips depth=32

	block_data_t buf;

	stream_to_array(in, buf);

#pragma HLS dataflow
	windowize_input(buf, pips);
	compute_output(pips, out);
}
