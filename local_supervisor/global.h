#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>
#include "ProcessConfigFile.h"
#include "TCPLineServer.h"

class RunningProcessInfo {
public:
   HANDLE hProcess;
   DWORD dwProcessId; // process id in windows
};

extern std::map<unsigned int, ProcessConfigFile::Process_Type> availableProcesses; // first = id from config file
extern std::map<unsigned int, RunningProcessInfo> runningProcesses; // first = id from config file
extern TCPLineServer server;

#endif