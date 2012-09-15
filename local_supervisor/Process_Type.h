#ifndef PROCESS_TYPE_H
#define PROCESS_TYPE_H

#include <string>

struct Process_Type {
	unsigned int id;
	std::string currentDir;
	std::string commandLine;
};

#endif