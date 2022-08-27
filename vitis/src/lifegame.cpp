#include  "lifegame.hpp"

typedef hls::stream<pkt> pkt_s;
typedef hls::stream<int> fifo_t;

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

void top(const int mline[SIZE], int lline[SIZE], pkt_s& ins, pkt_s& outs, pkt& val) {
	lline[0] = read_pkt(ins);
	lline[1] = read_pkt(ins);;
	int tl[] = {
			0, 0, 0,
			0, mline[0], mline[1],
			0, lline[0], lline[1],
	};
	write_pkt(outs, val, life_or_death(tl));
	for (int x = 1; x < SIZE - 1; x++) {
		lline[x + 1] = read_pkt(ins);;
		int tc[] = {
				0, 0, 0,
				mline[x - 1], mline[x], mline[x + 1],
				lline[x - 1], lline[x], lline[x + 1]
		};
		write_pkt(outs, val, life_or_death(tc));
	}
	int tr[] = {
			0, 0, 0,
			mline[SIZE - 2], mline[SIZE - 1], 0,
			lline[SIZE - 2], lline[SIZE - 1], 0
	};
	write_pkt(outs, val, life_or_death(tr));
}

void middle(const int uline[SIZE], const int mline[SIZE], int lline[SIZE], pkt_s& ins, pkt_s& outs, pkt& val) {
	lline[0] = read_pkt(ins);;
	lline[1] = read_pkt(ins);;
	int ml[] = {
			0, uline[0], uline[1],
			0, mline[0], mline[1],
			0, lline[0], lline[1]
	};
	write_pkt(outs, val, life_or_death(ml));
	for (int x = 1; x < SIZE - 1; x++) {
		lline[x + 1] = read_pkt(ins);;
		int mc[] = {
				uline[x - 1], uline[x], uline[x + 1],
				mline[x - 1], mline[x], mline[x + 1],
				lline[x - 1], lline[x], lline[x + 1]
		};
		write_pkt(outs, val, life_or_death(mc));
	}
	int mr[] = {
			uline[SIZE - 2], uline[SIZE - 1], 0,
			mline[SIZE - 2], mline[SIZE - 1], 0,
			lline[SIZE - 2], lline[SIZE - 1], 0
	};
	write_pkt(outs, val, life_or_death(mr));
}

void bottom(const int uline[SIZE], const int mline[SIZE], pkt_s& outs, pkt& val) {
	int bl[] = {
			0, uline[0], uline[1],
			0, mline[0], mline[1],
			0, 0, 0
	};
	write_pkt(outs, val, life_or_death(bl));
	for (int x = 1; x < SIZE - 1; x++) {
		int bc[] = {
				uline[x - 1], uline[x], uline[x + 1],
				mline[x - 1], mline[x], mline[x + 1],
				0, 0, 0
		};
		write_pkt(outs, val, life_or_death(bc));
	}
	int br[] = {
			uline[SIZE - 2], uline[SIZE - 1], 0,
			mline[SIZE - 2], mline[SIZE - 1], 0,
			0, 0, 0
	};
	write_last_pkt(outs, val, life_or_death(br));
}

void load(int line[SIZE], pkt_s& ins, pkt& val) {
	line[0] = read_first_pkt(ins, val);
	for (int x = 1; x < SIZE; x++) {
		line[x] = read_pkt(ins);
	}
}

void shift_down(int uline[SIZE], int mline[SIZE], const int lline[SIZE]) {
	for (int x = 0; x < SIZE; x++) {
#pragma HLS unroll
		uline[x] = mline[x];
		mline[x] = lline[x];
	}
}

void read_input(hls::stream<pkt>& in, fifo_t& ins, int val) {
	ins << val;
	for (int i = 1; i < SIZE * SIZE; i++) {
		pkt inval;
		in.read(inval);
		int v = inval.data;
		ins << v;
	}
}

void compute_nextstep(hls::stream<pkt>& ins, hls::stream<pkt>& outs) {
	int uline[SIZE], mline[SIZE], lline[SIZE];

	pkt val;

    load(uline, ins, val);
    // line 0, line 1
    top(uline, mline, ins, outs, val);

    for (int y = 1; y < SIZE - 1; y++) {
            // line y - 1, line y, line y + 1
            middle(uline, mline, lline, ins, outs, val);
            shift_down(uline, mline, lline);
    }

    // line SIZE - 2, line SIZE - 1
    bottom(uline, mline, outs, val);
}

void write_result(hls::stream<pkt>& out, fifo_t& outs, const ctrl keep, const ctrl strb) {
	pkt outval;
	outval.keep = keep;
	outval.strb = strb;
	outval.last = 0;
	for (int i = 0; i < SIZE * SIZE; i++) {
		int v;
		outs >> v;
		outval.data = v;
		if (i == SIZE * SIZE - 1) {
			outval.last = 1;
		}
		out.write(outval);
	}
}

void lifegame(hls::stream<pkt>& in, hls::stream<pkt>& out) {
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE ap_ctrl_none port=return

	fifo_t ins("input_fifo");
	fifo_t outs("output_fifo");

//#pragma HLS dataflow
	//read_input(in, ins, val);
	compute_nextstep(in, out);
	//write_result(out, outs, keep, strb);
}
