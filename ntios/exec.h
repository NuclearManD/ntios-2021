
#ifndef EXEC_H
#define EXEC_H

#include "drivers.h"

// You opened the file.  You're responsible for closing it.
// _ntios_do_file_exec is complex enough without bothering to close files at the right times.
int _ntios_do_file_exec(StreamDevice* io, NTIOSFile* file, int argc, char** argv);

#endif
