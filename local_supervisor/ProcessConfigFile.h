#ifndef PROCESSCONFIGFILE_H
#define PROCESSCONFIGFILE_H

#include <string>
#include <map>
#include <vector>

class ProcessConfigFile {
public:

	struct Process_Type {
		unsigned int id;
		std::string currentDir;
		std::string commandLine;
	};

	ProcessConfigFile(std::string filePath) { myFilePath = filePath; }

	bool ReadProcesses(std::map<unsigned int, Process_Type> *processMap);

private:
	std::string myFilePath;
};

#endif