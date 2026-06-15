#include "UCresseliaApplication.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
	UCresseliaApplication app;

	if (!app.Setup()) {
		return 0;
	}

	app.Run();

	if (!app.Teardown()) {
		return 0;
	}
}