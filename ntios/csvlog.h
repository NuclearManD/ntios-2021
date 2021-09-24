
#ifndef CSVLOG_H
#define CSVLOG_H


// For vector
#undef min
#undef max

#include <vector>

#include "ntios.h"
#include "ntios_math.h"

#define LOG_DIR "/logs/"

class CSVLogger {
private:

	// In the future we want to avoid using these C++ libraries,
	// as they aren't always portable.
	// Consider implementing them in this codebase or making an
	// equivelent.
	// For now we leave them, as they haven't caused any problems *yet*
	std::vector<char*> valnames;
	std::vector<double> valcache;

	unsigned long cachetime;
	unsigned long flush_timer;

	double seconds_since_open;

	bool is_open;

	NTIOSFile* file;

	char logfilename[20];

	bool is_flushing = false;
public:

	void flush();

	// returns -1 if the valname is not found
	int valindex(const char* valname);
	int markDouble(const char* name);
	int markVector(const char* name);

	bool isOpen() { return is_open; }
	bool openLog();
	const char* closeLog();

	bool publishDouble(uint16_t index, double val);
	bool publishVector(uint16_t index, Vector3 val);
};

extern CSVLogger internal_logger;

#endif
