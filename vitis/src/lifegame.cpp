#include  "lifegame.hpp"
#include <multimediaIps/xf_video_mem.hpp>

typedef hls::stream<pkt> pkt_s;
//typedef hls::stream<int> fifo_t;

typedef xf::cv::LineBuffer<2, SIZE, int> linebuf_t;
typedef xf::cv::Window<3, 3, int> window_t;

int life_or_death(int w[9]) {
	int nb =  w[0] + w[1] + w[2] + w[3] + w[5] + w[6] + w[7] + w[8];
	int tt = nb ^ 0x03;
	return (tt == 0 || (tt ^ w[4]) == 0) ? 1 : 0;
}

int read_first_pkt(pkt_s& in, pkt& val) {
	in.read(val);
	return val.data;
}

int read_pkt(pkt_s& in) {
	pkt val;
	in.read(val);
	return val.data;
}

void write_pkt(pkt_s& out, pkt& val, const int v) {
	val.data = v;
	out.write(val);
}

void write_last_pkt(pkt_s& out, pkt& val, const int v) {
	val.data = v;
	val.last = 1;
	out.write(val);
}

void slide_window(linebuf_t& linebuf, window_t& window, const int x, pkt_s& ins, pkt& val) {
	int rows[3];

	linebuf.get_col(rows, x);
	rows[2] = read_pkt(ins);
	linebuf.shift_pixels_up(x);
	linebuf.insert_bottom_row(rows[2], x);

	window.shift_pixels_left();
	window.insert_right_col(rows);
}

void slide_window(linebuf_t& linebuf, window_t& window, const int x) {
	window.shift_pixels_left();
	window.insert_pixel(linebuf(0, x), 1, 2);
	window.insert_pixel(linebuf(1, x), 2, 2);
}

void top(linebuf_t& linebuf, window_t& window, pkt_s& ins, pkt_s& outs, pkt& val) {
	for (int x = 0; x < SIZE + 1; x++) {
		if (x < SIZE) {
			slide_window(linebuf, window, x, ins, val);
		}

		if (x == 1) {
			int tl[] = {
					0, 0, 0,
					0, window(1, 1), window(1, 2),
					0, window(2, 1), window(2, 2)
			};
			write_pkt(outs, val, life_or_death(tl));
		} else if (x == SIZE) {
			int tr[] = {
					0, 0, 0,
					window(1, 1), window(1, 2), 0,
					window(2, 1), window(2, 2), 0
			};
			write_pkt(outs, val, life_or_death(tr));
		} else if (x > 0) {
			int tc[] = {
					0, 0, 0,
					window(1, 0), window(1, 1), window(1, 2),
					window(2, 0), window(2, 1), window(2, 2)
			};
			write_pkt(outs, val, life_or_death(tc));
		}
	}
}

void middle(linebuf_t& linebuf, window_t& window, pkt_s& ins, pkt_s& outs, pkt& val) {
	for (int x = 0; x < SIZE + 1; x++) {
		if (x < SIZE) {
			slide_window(linebuf, window, x, ins, val);
		}

		if (x == 1) {
			int ml[] = {
					0, window(0, 1), window(0, 2),
					0, window(1, 1), window(1, 2),
					0, window(2, 1), window(2, 2)
			};
			write_pkt(outs, val, life_or_death(ml));
		} else if (x == SIZE) {
			int mr[] = {
					window(0, 1), window(0, 2), 0,
					window(1, 1), window(1, 2), 0,
					window(2, 1), window(2, 2), 0
			};
			write_pkt(outs, val, life_or_death(mr));
		} else if (x > 0) {
			int mc[] = {
				window(0, 0), window(0, 1), window(0, 2),
				window(1, 0), window(1, 1), window(1, 2),
				window(2, 0), window(2, 1), window(2, 2)
			};
			write_pkt(outs, val, life_or_death(mc));
		}
	}
}

void bottom(linebuf_t& linebuf, window_t& window, pkt_s& outs, pkt& val) {
	for (int x = 0; x < SIZE + 1; x++) {
		if (x < SIZE) {
			slide_window(linebuf, window, x);
		}

		if (x == 1) {
			int bl[] = {
					0, window(1, 1), window(1, 2),
					0, window(2, 1), window(2, 2),
					0, 0, 0
			};
			write_pkt(outs, val, life_or_death(bl));
		} else if (x == SIZE) {
			int br[] = {
					window(1, 1), window(1, 2), 0,
					window(2, 1), window(2, 2), 0,
					0, 0, 0
			};
			write_last_pkt(outs, val, life_or_death(br));
		} else if (x > 0) {
			int bc[] = {
					window(1, 0), window(1, 1), window(1, 2),
					window(2, 0), window(2, 1), window(2, 2),
					0, 0, 0,
			};
			write_pkt(outs, val, life_or_death(bc));
		}
	}
}

void preload(linebuf_t& linebuf, pkt_s& ins, pkt& val) {
	int v = read_first_pkt(ins, val);
	linebuf.insert_bottom_row(v, 0);
	for (int x = 1; x < SIZE; x++) {
		v = read_pkt(ins);
		linebuf.insert_bottom_row(v, x);
	}
}

//void read_input(hls::stream<pkt>& in, fifo_t& ins, int val) {
//	ins << val;
//	for (int i = 1; i < SIZE * SIZE; i++) {
//		pkt inval;
//		in.read(inval);
//		int v = inval.data;
//		ins << v;
//	}
//}

void compute_nextstep(hls::stream<pkt>& ins, hls::stream<pkt>& outs) {
	linebuf_t linebuf;
	window_t window;

	pkt val;

	preload(linebuf, ins, val);
	// line 0, line 1
	top(linebuf, window, ins, outs, val);

	for (int y = 2; y < SIZE; y++) {
		// line y - 1, line y, line y + 1
		middle(linebuf, window, ins, outs, val);
	}

	// line SIZE - 2, line SIZE - 1
	bottom(linebuf, window, outs, val);
}

//void write_result(hls::stream<pkt>& out, fifo_t& outs, const ctrl keep, const ctrl strb) {
//	pkt outval;
//	outval.keep = keep;
//	outval.strb = strb;
//	outval.last = 0;
//	for (int i = 0; i < SIZE * SIZE; i++) {
//		int v;
//		outs >> v;
//		outval.data = v;
//		if (i == SIZE * SIZE - 1) {
//			outval.last = 1;
//		}
//		out.write(outval);
//	}
//}

void lifegame(hls::stream<pkt>& in, hls::stream<pkt>& out) {
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE ap_ctrl_none port=return

	//fifo_t ins("input_fifo");
	//fifo_t outs("output_fifo");

//#pragma HLS dataflow
	//read_input(in, ins, val);
	compute_nextstep(in, out);
	//write_result(out, outs, keep, strb);
}
