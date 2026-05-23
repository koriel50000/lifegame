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

int main(int argc, char** argv) {

	int ini[SIZE * SIZE];

	if (read_cells("./diehard_2474_0.txt", ini)) {
		printf("ERROR initialize file read failed.\n");
		return 1;
	}

	int exp[SIZE * SIZE];

	if (read_cells("./diehard_2474_4.txt", exp)) {
		printf("ERROR expected file read failed.\n");
		return 1;
	}

	fifo<axis_data> in, out;

	for (int i = 0; i < SIZE * SIZE; i++) {
		axis_data pkt;
		pkt.data = ini[i];
		pkt.last = (i == SIZE * SIZE / 2 - 1 || i == SIZE * SIZE - 1) ? 1 : 0;
		in.write(pkt);
	}

	for (int c = 0; c < 4; c++) {
		//Call the hardware function
		lifegame(in, out);

		for (int i = 0; i < SIZE * SIZE; i++) {
			axis_data pkt = out.read();
			in.write(pkt);
		}
	}

	//Compare results
	for (int i = 0; i < SIZE * SIZE; i++) {
		axis_data pkt = in.read();
		//int last = (i == SIZE * SIZE / 2 - 1 || i == SIZE * SIZE - 1) ? 1 : 0;
		if (pkt.data != exp[i]) { // || pkt.last != last) {
			printf("ERROR HW and SW results mismatch [%d] expected=%d actual=%d\n", i, exp[i], pkt.data);
			return 1;
		}
	}
	printf("Success HW and SW results match\n");
	return 0;
}
