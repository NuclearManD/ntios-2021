
#include <stdint.h>

#include "csvlog.h"
#include "ntios.h"

CSVLogger internal_logger;

void CSVLogger::flush() {
	if (is_open) {
		while (is_flushing)
			yield();
		is_flushing = true;

		file->printf("%f,%.2f,%lu,", (double)seconds_since_open, get_cpu_usage_percent(), (unsigned long)get_cpu_hz());
	}
	for (uint16_t i = 0; i < valcache.size(); i++) {
		if (valcache[i] != NAN) {
			if (is_open)
				file->printf("%f,", valcache[i]);
			valcache[i] = NAN;
		} else if (is_open)
			file->write(',');
	}
	if (is_open) {
		file->write('\n');
		if (flush_timer < millis()) {
			file->flush();
			flush_timer = millis() + 100;
		}
		seconds_since_open += (millis() - cachetime) / 1000.f;
	}

	is_flushing = false;
}

// returns -1 if the valname is not found
int CSVLogger::valindex(const char* valname) {
	for (uint16_t i = 0; i < valnames.size(); i++) {
		if (!strcmp(valname, valnames[i]))
			return i;
	}
	return -1;
}

int CSVLogger::markDouble(const char* name) {
	if (strlen(name) > 32)
		return -1;
	for (uint16_t i = 0; i < valnames.size(); i++) {
		if (!strcmp(name, valnames[i]))
			return i;
	}
	
	if (is_open)
		return -1; // do not allow creation of new val while logging is in progress

	int result = valnames.size();

	char* newname = (char*)malloc(strlen(name) + 1);
	strcpy(newname, name);

	valnames.push_back(newname);
	valcache.push_back(NAN);
	return result;
}

int CSVLogger::markVector(const char* name) {
	if (strlen(name) > 32)
		return -1;

	char* newname = (char*)malloc(strlen(name) + 3);
	strcpy(newname, name);
	strcat(newname, ".x");

	for (uint16_t i = 0; i < valnames.size(); i++) {
		if (!strcmp(name, valnames[i]))
			return i;
	}

	if (is_open) {
		free(newname);
		return -1; // do not allow creation of new val while logging is in progress
	}

	int result = valnames.size();

	valnames.push_back(newname);
	valcache.push_back(NAN);

	newname = (char*)malloc(strlen(name) + 3);
	strcpy(newname, name);
	strcat(newname, ".y");

	valnames.push_back(newname);
	valcache.push_back(NAN);

	newname = (char*)malloc(strlen(name) + 3);
	strcpy(newname, name);
	strcat(newname, ".z");

	valnames.push_back(newname);
	valcache.push_back(NAN);

	return result;
}

bool CSVLogger::openLog() {
	if (is_open) return false;

	if (!fsexists(LOG_DIR))
		if (fsmkdir(LOG_DIR) != 0)
			return false; // failure making logging directory

	memset(logfilename, 0, sizeof(logfilename));
	strcpy(logfilename, LOG_DIR);
	int slen = strlen(logfilename);

	// 4 for ".csv" extension
	size_t maxnamesize = sizeof(logfilename) - slen - 1 - 4;
	snprintf(logfilename + slen, maxnamesize, "l%.8li", micros());
	strcat(logfilename, ".csv");

	file = fsopen(logfilename, NTIOS_WRITE);
	if (file == NULL)
		return false;

	file->printf("stopwatch,cpuload,cpu_hz,");
	for (uint16_t i = 0; i < valnames.size(); i++)
		file->printf("%s,", valnames[i]);
	file->write('\n');

	is_open = true;
	seconds_since_open = false;
	cachetime = millis();
	return true;
}

const char* CSVLogger::closeLog() {
	if (!is_open)
		return NULL;
	flush();
	file->close();
	is_open = false;
	return logfilename;
}

bool CSVLogger::publishDouble(uint16_t index, double val) {
	if (index >= valcache.size())
		return false;

	if (is_flushing) {
		while (is_flushing)
			yield();
	} else {
		uint32_t time = millis();
		if (cachetime != time) {
			flush();
			cachetime = time;
		}
	}

	while (is_flushing)
		yield();
	valcache[index] = val;
	return true;
}

bool CSVLogger::publishVector(uint16_t index, Vector3 val) {
	if (index >= valcache.size() - 2)
		return false;

	if (is_flushing) {
		while (is_flushing)
			yield();
	} else {
		uint32_t time = millis();
		if (cachetime != time) {
			flush();
			cachetime = time;
		}
	}

	valcache[index] = val.x;
	valcache[index + 1] = val.y;
	valcache[index + 2] = val.z;
	return true;
}
