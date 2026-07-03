
/* tab = 4 */
#include <iostream>
#include <string>

#include <process.h>
#include <Windows.h>

import WinHelperModule;
import ConsoleModule;
import CmdArgsHelperModule;
import LoggerModule;

const wchar_t* args[] = {
	LR"(C:\Users\Myste\protable_apps\.OHMS\BGM-Maker Release\HandleArknightsMusic.exe)",

	LR"("a b")",           /*a b*/
	LR"("c d\")", LR"(e")",/*c d" e*/  // ->   "c d \"e"
	LR"("f g\\")",         /*f g\*/
	LR"("hi")",            /*hi*/
	LR"(jk)",              /*jk*/
	LR"("l m\"n")",        /*l m"n*/
	LR"(pro\\\"gram.exe)", /*pro\"gram.exe*/
	LR"(pro\"gram64.exe)", /*pro"gram64.exe*/
	LR"(%OHMS_LIB_DIR%)",  /*%OHMS_LIB_DIR%*/
	LR"(" \\\\ ")",        /* \\\\ */
	LR"("\\\\ ")",         /*\\\\ */
	LR"("\\\\")",          /*\\*/
	LR"(X="abc 123")",     /*X=abc 123*/
	LR"(Y=\"def 456\")",   /*Y="def*/
		  /*(* ^ *)*/      /*456"*/
	LR"("Z=ghi 789")",     /*Z=ghi 789*/
	LR"(W=lll 000)",       /*W=lll*/
		/*(* ^ *)*/        /*000*/
	NULL
};

int main() {
	SetConsoleOutputCP(CP_UTF8);
	//setlocale(LC_ALL, ".utf8");
	//std::cout.imbue(std::locale(".utf-8"));
	//std::wcout.imbue(std::locale(".utf-8"));

	Logger::info << "TEST" << logendl << logtab << "info";
	Logger::warning << "TEST" << logendl << logtab << "warning";
	Logger::error << "TEST" << logendl << logtab << "error";
	Logger::exception << "TEST \vexception v";
	Logger::exception << "TEST \n    exception n ....";
	Logger::exception << "TEST" << logendl << logtab << "exception n t";
	Logger::exception << "TEST \v\r\texception v r t";
	Logger::exception << "TEST" << logendl << logtab << "exception n t";
	Logger::exception <<
		"This is an exception message with special characters: \tstdfghedygbn"
		"54e 56w4 6tw5y \n rge3564 6yey6yr56u ey we45y457\n6 356 3456 54trfy w45e565 r5r\"\\ and colors!" << "\r\n";
	Logger::success << "TEST success" << logendl << logtab << "success";

	std::cout << "TestOut" << std::endl;
	std::cerr << "TestErr" << std::endl;

	//logger.reset();

	std::cout << "2TestOut" << std::endl;
	std::cerr << "2TestErr" << std::endl;

	//_wspawnv(_P_WAIT, args[0], args);

	CmdArgBuilderW cab;
	for (size_t i = 1, n = sizeof(args) / sizeof(args[0]); i < n; ++i) {
		if (args[i] == nullptr)
			continue;
		cab += args[i];
	}

	Logger::warning << cab ;
	//logger.reset();

	WinProcRunAndWait(args[0], cab);

	Console::WriteLine("Over 完毕 A 🙂!");
	Console::WriteLine(L"Over 完毕 W 🙂!");
	std::cout << "Over 完毕 A 🙂!" << std::endl;
	std::cout.flush();
	std::wcout << "Over 完毕 A 🙂!" << std::endl;
	std::wcout.flush();
	std::wcout << L"Over 完毕 W 🙂!" << std::endl; // 无法修复 (No way to fix perfectly)
	std::wcout.flush();


	Console::WriteLine("OK 完毕 A 🙂!");
	Console::WriteLine(L"OK 完毕 W 🙂!");
	Logger::success << "OK 完毕 A 🙂!";
	Logger::success << "OK 完毕 A 🙂!";
	Logger::success << L"OK 完毕 W 🙂!";

	return 0;
}
