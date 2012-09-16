#include "stdafx.h"
#include "ProcessFunctions.h"
#include "global.h"
#include <sstream>

void StartProcess(DWORD id) {
	RunningProcessInfo runningProcessInfo;
	runningProcessInfo.id = id;

	/* MSDN: The Unicode version of CreateProcess can modify the contents of lpCommandLine.
	* Therefore, this parameter cannot be a pointer to read-only memory (such as a const variable or a
	* literal string). If this parameter is a constant string, the function may cause an access violation.
	*/
	STARTUPINFO siStartupInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);
	PROCESS_INFORMATION piProcessInfo;
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	char* commandLine = new char[availableProcesses[id].commandLine.length()+1];
	strncpy(commandLine, availableProcesses[id].commandLine.c_str(), availableProcesses[id].commandLine.length()+1);
	CreateProcess(NULL, commandLine, NULL, NULL, FALSE, NULL, NULL, availableProcesses[id].currentDir.c_str(), &siStartupInfo, &piProcessInfo);
	runningProcessInfo.dwProcessId = piProcessInfo.dwProcessId;
	runningProcessInfo.hProcess = piProcessInfo.hProcess;
	CloseHandle(piProcessInfo.hThread);

	runningProcesses.insert(std::pair<unsigned int, RunningProcessInfo>(id, runningProcessInfo));
}

void StopProcess(DWORD id) {
	HANDLE hProcess = runningProcesses[id].hProcess;
	TerminateProcess(hProcess, 1);
	CloseHandle(hProcess);
	runningProcesses.erase(id);
}

void FindAndEraseStoppedProcesses() {
	std::vector<unsigned int> process_ids_to_erase;
	std::map<unsigned int, RunningProcessInfo>::iterator it;
	for(it=runningProcesses.begin() ; it != runningProcesses.end(); it++) {
		RunningProcessInfo runningProcess = it->second;
		unsigned int id = it->first;

		DWORD exitCode;
		DWORD success = GetExitCodeProcess(runningProcess.hProcess, &exitCode);
		DWORD error = GetLastError();

		if(exitCode != STILL_ACTIVE) {
			if(server.ClientConnected()) {
				std::stringstream stringStream;
				stringStream << "stopped;" << id << ";";
				server.AddToSendQueue(stringStream.str());
			}

			CloseHandle(runningProcess.hProcess);
			process_ids_to_erase.push_back(id);
		}
	}

	std::vector<unsigned int>::iterator vect_it;
	for(vect_it=process_ids_to_erase.begin(); vect_it != process_ids_to_erase.end(); vect_it++) {
		runningProcesses.erase(*vect_it);
	}
}