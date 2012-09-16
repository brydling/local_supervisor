#include "stdafx.h"
#include "AddProcessesNotStartedBySupervisor.h"
#include "global.h"
#include <WinDef.h>
#include <Psapi.h>
#include <string>
#include <sstream>
#include <map>

void AddProcessesNotStartedBySupervisor() {
	DWORD pids[256];
	DWORD bytesreturned;
	EnumProcesses(pids, sizeof(pids), &bytesreturned);
	DWORD processes = bytesreturned/sizeof(DWORD);
	for(int i=0; i<processes; i++) {
		bool addedProcessToRunningList = false;
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pids[i]);
		char filePath[512];
		DWORD size = sizeof(filePath);
		if(QueryFullProcessImageName(hProcess, 0, filePath, &size) != 0) {
			for(int j=0; j<size; j++) {
				filePath[j] = tolower(filePath[j]);
			}
			std::string sFilePath = filePath;

			std::map<unsigned int, ProcessConfigFile::Process_Type>::iterator it;
			for(it=availableProcesses.begin(); it != availableProcesses.end(); it++) {
				if(runningProcesses.count(it->first) == 0 && it->second.commandLine == sFilePath) {
					RunningProcessInfo runningProcessInfo;
					runningProcessInfo.id = it->first;
					runningProcessInfo.dwProcessId = pids[i];
					runningProcessInfo.hProcess = hProcess;
					runningProcesses.insert(std::pair<unsigned int, RunningProcessInfo>(runningProcessInfo.id, runningProcessInfo));
					addedProcessToRunningList = true;

					if(server.ClientConnected()) {
						unsigned int id = it->first;
						std::stringstream stringStream;
						stringStream << "running;" << id << ";";
						server.AddToSendQueue(stringStream.str());
					}

					break;
				}
			}
		}
		if(!addedProcessToRunningList) {
			CloseHandle(hProcess);
		}
	}
}