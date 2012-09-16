#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>
#include "ProcessConfigFile.h"
#include "TCPLineServer.h"

class RunningProcessInfo {
public:
	unsigned int id;
	HANDLE hProcess;
	DWORD dwProcessId;
};

extern std::map<unsigned int, ProcessConfigFile::Process_Type> availableProcesses;
extern std::map<unsigned int, RunningProcessInfo> runningProcesses;
extern TCPLineServer server;

#endif