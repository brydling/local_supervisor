#ifndef PROCESSCONFIGFILE_H
#define PROCESSCONFIGFILE_H

#include <string>
#include <map>
#include <vector>
#include "process_type.h"

class ProcessConfigFile {
public:
	ProcessConfigFile(std::string filePath) { myFilePath = filePath; }

	bool ReadProcesses(std::map<unsigned int, Process_Type> *processMap);

private:
	std::string myFilePath;
};

#endif