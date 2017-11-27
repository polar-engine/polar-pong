#include <pong/app.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine,
                   int nCmdShow) {
	int argc    = __argc;
	char **argv = __argv;
#else
int main(int argc, char **argv) {
#endif
	auto engine = polar::core::polar({argv, argv + argc});
	pong::app{engine};
	return 0;
}
