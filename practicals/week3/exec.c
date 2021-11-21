#include <unistd.h>

int main(int argc, char** argv) {
	return execv("/usr/bin/ls", argv);
}
