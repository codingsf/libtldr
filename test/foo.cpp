#include "config.h"
#include "foo.hpp"

#include <chrono>
#include <thread>

const int foo_test_data = 0x11223344;

int foo_test_proc()
{
	return 0x11223344;
}

void foo_test_imports()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
}
