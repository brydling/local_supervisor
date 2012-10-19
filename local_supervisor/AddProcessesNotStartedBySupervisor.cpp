#include "stdafx.h"
#include "AddProcessesNotStartedBySupervisor.h"
#include "global.h"
#include <WinDef.h>
#include <Psapi.h>
#include <string>
#include <sstream>
#include <map>

typedef std::basic_string<TCHAR> tstring;

struct HardDisk_Type
{
	tstring devicePath;
	tstring driveLetter;
};

typedef std::vector<HardDisk_Type> HardDiskCollection;

HardDiskCollection _hardDiskCollection;
bool hardDiskCollectionInitialized = 0;

void InitializeHardDiskCollection( HardDiskCollection &_hardDiskCollection )
{
    TCHAR tszLinkName[MAX_PATH] = { 0 };
    TCHAR tszDevName[MAX_PATH] = { 0 };
    TCHAR tcDrive = 0;

    _tcscpy_s( tszLinkName, MAX_PATH, _T("a:") );
    for ( tcDrive = _T('a'); tcDrive < _T('z'); ++tcDrive )
    {
        tszLinkName[0] = tcDrive;
        if ( QueryDosDevice( tszLinkName, tszDevName, MAX_PATH ) )
        {
			HardDisk_Type hardDisk;
			for(int j=0; j<strlen(tszDevName); j++) {
				tszDevName[j] = tolower(tszDevName[j]);
			}
			hardDisk.devicePath = tszDevName;
			hardDisk.driveLetter = tszLinkName;

			_hardDiskCollection.push_back( hardDisk );
        }
    }
}

void AddProcessesNotStartedBySupervisor() {
	DWORD pids[512];
	DWORD bytesreturned;
	EnumProcesses(pids, sizeof(pids), &bytesreturned);
	DWORD processes = bytesreturned/sizeof(DWORD);

	if(!hardDiskCollectionInitialized)
	{
		InitializeHardDiskCollection(_hardDiskCollection);
	}

	for(int i=0; i<processes; i++) {
		bool addedProcessToRunningList = false;
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pids[i]);
		char filePath[512];
		DWORD size = sizeof(filePath);
		if(GetProcessImageFileName(hProcess, filePath, size) != 0) {
			for(int j=0; j<size; j++) {
				filePath[j] = tolower(filePath[j]);
			}

			std::string sFilePath = filePath;

			// Replace the device path with a drive letter
			for(int hdIndex = 0; hdIndex < _hardDiskCollection.size(); hdIndex++)
			{
				if(sFilePath.find(_hardDiskCollection[hdIndex].devicePath) != std::string::npos)
				{
					sFilePath.replace(0, _hardDiskCollection[hdIndex].devicePath.length(), _hardDiskCollection[hdIndex].driveLetter);
					break;
				}
			}

			std::map<unsigned int, ProcessConfigFile::Process_Type>::iterator it;
			for(it=availableProcesses.begin(); it != availableProcesses.end(); it++) {
				if(runningProcesses.count(it->first) == 0 && it->second.commandLine == sFilePath) {
					DWORD exitCode;
					DWORD success = GetExitCodeProcess(hProcess, &exitCode);
					DWORD error = GetLastError();

					if(exitCode == STILL_ACTIVE) {
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
					}

					break;
				}
			}
		} else {
			/*DWORD error = GetLastError();

			std::stringstream str;
			str << "GetModuleFileNameEx failed for PID " << pids[i] << " with error code " << error << ".";

			MessageBoxA(0, str.str().c_str(), "Error", MB_OK);*/
		}

		if(!addedProcessToRunningList) {
			CloseHandle(hProcess);
		}
	}

	//MessageBoxA(0, strProcesses.c_str(), "Processes", MB_OK);
}