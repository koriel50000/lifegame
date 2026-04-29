#include <iostream>
#include "xunit.hpp"
#include "window_buffer_test.hpp"

int main_bak(int argc, char** argv)
{
	std::vector<TestEntry> tests = {
		TEST(WindowBufferTest::test_window),
		TEST(WindowBufferTest::test_linebuffer),
	};

	TestRunner runner;
	runner.runAllTests(tests);

	return 0;
}
