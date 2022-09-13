#include "lifegame.hpp"
#include <fstream>
#include <sstream>

int read_cells(const char* filename, int cells[SIZE * SIZE]) {
	std::ifstream is(filename);

	if (is.fail()) {
		printf("ERROR file open failed.\n");
		return 1;
	}

	int ptr = 0;
	std::string line;
	while (getline(is, line)) {
		std::istringstream iss(line);
		std::string str;
		while (getline(iss, str, ' ')) {
			cells[ptr++] = atoi(str.c_str());
		}
	}
	printf("count=%d\n", ptr);

	return 0;
}

int main() {

	int ini[SIZE * SIZE];

	if (read_cells("../../../src/diehard_2474_0.txt", ini)) {
		printf("ERROR initialize file read failed.\n");
		return 1;
	}

	int exp[SIZE * SIZE];

	if (read_cells("../../../src/diehard_2474_4.txt", exp)) {
		printf("ERROR expected file read failed.\n");
		return 1;
	}

	hls::stream<pkt> in, out;

	for (int i = 0; i < SIZE * SIZE; i++) {
		pkt v;
		v.data = ini[i];
		if (i == SIZE * SIZE - 1) v.last = 1;
		in.write(v);
	}

	for (int c = 0; c < 4; c++) {
		//Call the hardware function
		lifegame(in, out);

		for (int i = 0; i < SIZE * SIZE; i++) {
			pkt v = out.read();
			in.write(v);
		}
	}

	//Compare results
	for (int i = 0; i < SIZE * SIZE; i++) {
		pkt v = in.read();
		int last = (i == SIZE * SIZE - 1) ? 1 : 0;
		if (v.data != exp[i] || v.last != last) {
			printf("ERROR HW and SW results mismatch\n");
			return 1;
		}
	}
	printf("Success HW and SW results match\n");
	return 0;
}
