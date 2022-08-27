#include "lifegame.hpp"

int main() {

	int ini[SIZE * SIZE];
	std::fill_n(ini, SIZE * SIZE, 0);
//	int ini[64] = { 0, 0, 0, 0, 0, 0, 0, 0,
//					0, 0, 1, 0, 0, 0, 0, 0,
//					0, 0, 0, 1, 0, 0, 0, 0,
//					0, 1, 1, 1, 0, 0, 0, 0,
//					0, 0, 0, 0, 0, 0, 0, 0,
//					0, 0, 0, 0, 0, 0, 0, 0,
//					0, 0, 0, 0, 0, 0, 0, 0,
//					0, 0, 0, 0, 0, 0, 0, 0 };

	hls::stream<pkt> in, out;

	for (int i = 0; i < SIZE * SIZE; i++) {
		pkt v;
		v.data = ini[i];
		if (i == SIZE * SIZE - 1) v.last = 1;
		in.write(v);
	}

	//Call the hardware function
	lifegame(in, out);

	int exp[SIZE * SIZE];
	std::fill_n(exp, SIZE * SIZE, 0);
//	int exp[64] = { 0, 0, 0, 0, 0, 0, 0, 0,
//					0, 0, 0, 0, 0, 0, 0, 0,
//					0, 1, 0, 1, 0, 0, 0, 0,
//					0, 0, 1, 1, 0, 0, 0, 0,
//					0, 0, 1, 0, 0, 0, 0, 0,
//					0, 0, 0, 0, 0, 0, 0, 0,
//					0, 0, 0, 0, 0, 0, 0, 0,
//					0, 0, 0, 0, 0, 0, 0, 0 };

	//Compare results
	for (int i = 0; i < SIZE * SIZE; i++) {
		pkt v;
		out.read(v);
		int last = (i == SIZE * SIZE - 1) ? 1 : 0;
		if (v.data != exp[i] || v.last != last) {
			printf("ERROR HW and SW results mismatch\n");
			return 1;
		}
	}
	printf("Success HW and SW results match\n");
	return 0;
}
