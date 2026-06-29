#ifndef MACROS_H
#define MACROS_H

#define trecmake_begin namespace trecmake {
#define trecmake_end   }
#define trecmake_ns    trecmake::

#define enable_colors \
{                                                           \
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);      \
	DWORD mode = 0;                                         \
	GetConsoleMode(hConsole, &mode);                        \
	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;             \
	SetConsoleMode(hConsole, mode);                         \
}


#endif // !MACROS_H