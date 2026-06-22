
#include "Combiner.h"

#include <stdlib.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	cout << "Info: Cmd line:" << endl;
	for (int i = 0; i < argc; ++i) {
		cout << "\t'" << argv[i] << "'" << endl;
	}
	cout << "End cmd line." << endl << endl;
	if (argc == 2) {
		cout << "Info: Work in '" << argv[1] << "'." << endl;
		try {
			if (ohms::CombineIn(argv[1])) {
				cout << "Info: Task Succeed." << endl;
			}
			else {
				cout << "Info: Task Failed." << endl;
			}
		}
		catch (std::exception& e) {
			cout << "Exception: '" << e.what() << "'." << endl;
		}
		catch (...) {
			cout << "Exception: Unknown Exception." << endl;
		}
	}
	else {
		cout << "Error: Unsupported Argument." << endl;
		cout << "Just drag one single folder and drop on the icon of this app." << endl;
	}
	system("pause");
	return 0;
}
