#include "stdafx.h"
#include "AddProcessesNotStartedBySupervisor.h"
#include "global.h"
#include <WinDef.h>
#include <strsafe.h>
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
         for(unsigned int j = 0; j < strlen(tszDevName); j++) {
            tszDevName[j] = tolower(tszDevName[j]);
         }
         hardDisk.devicePath = tszDevName;
         hardDisk.driveLetter = tszLinkName;

         _hardDiskCollection.push_back( hardDisk );
      }
   }
}

void ErrorMessage(LPTSTR lpszFunction)
{ 
   // Retrieve the system error message for the last-error code

   LPVOID lpMsgBuf;
   LPVOID lpDisplayBuf;
   DWORD dw = GetLastError(); 

   FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | 
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      dw,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &lpMsgBuf,
      0, NULL );

   // Display the error message and exit the process

   lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
   StringCchPrintf((LPTSTR)lpDisplayBuf, 
      LocalSize(lpDisplayBuf) / sizeof(TCHAR),
      TEXT("%s failed with error %d: %s"), 
      lpszFunction, dw, lpMsgBuf); 
   MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

   LocalFree(lpMsgBuf);
   LocalFree(lpDisplayBuf);
}

void AddProcessesNotStartedBySupervisor() {
   DWORD pids[512];
   DWORD bytesreturned;
   EnumProcesses(pids, sizeof(pids), &bytesreturned);
   DWORD processes = bytesreturned/sizeof(DWORD);

   if(!hardDiskCollectionInitialized)
   {
      InitializeHardDiskCollection(_hardDiskCollection);
      hardDiskCollectionInitialized = true;
   }

#ifdef DEBUG_TEXT
   MessageBoxA(0, "Begin looping over started processes", "Debug", MB_OK);
#endif

   for(unsigned int i = 0; i < processes; i++) {
      bool addedProcessToRunningList = false;
#ifdef DEBUG_TEXT
      char buf[20];
      sprintf(buf, "PID: %d", pids[i]);
      MessageBoxA(0, buf, "Debug", MB_OK);
#endif

      HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE | SYNCHRONIZE/*PROCESS_ALL_ACCESS*/, 0, pids[i]);
      if(hProcess == NULL) {
#ifdef DEBUG_TEXT
         ErrorMessage("OpenProcess");
#endif
         continue;
      }

      char filePath[512];
      DWORD size = sizeof(filePath);

      if(GetProcessImageFileName(hProcess, filePath, size) != 0) {
         for(unsigned int j = 0; j < strlen(filePath); j++) {
            filePath[j] = tolower(filePath[j]);
         }

         std::string sFilePath = filePath;

         // Replace the device path with a drive letter
         for(unsigned int hdIndex = 0; hdIndex < _hardDiskCollection.size(); hdIndex++)
         {
            if(sFilePath.find(_hardDiskCollection[hdIndex].devicePath) != std::string::npos)
            {
               sFilePath.replace(0, _hardDiskCollection[hdIndex].devicePath.length(), _hardDiskCollection[hdIndex].driveLetter);
               break;
            }
         }

         std::map<unsigned int, ProcessConfigFile::Process_Type>::iterator it;
#ifdef DEBUG_TEXT
         MessageBoxA(0, sFilePath.c_str(), "Debug", MB_OK);
#endif
         for(it=availableProcesses.begin(); it != availableProcesses.end(); it++) {
            if(runningProcesses.count(it->first) == 0 && it->second.commandLine == sFilePath) {
               DWORD exitCode;
               DWORD success = GetExitCodeProcess(hProcess, &exitCode);
               DWORD error = GetLastError();

               if(exitCode == STILL_ACTIVE) {
                  RunningProcessInfo runningProcessInfo;
                  unsigned int id = it->first;
                  runningProcessInfo.dwProcessId = pids[i];
                  runningProcessInfo.hProcess = hProcess;
                  runningProcesses.insert(std::pair<unsigned int, RunningProcessInfo>(id, runningProcessInfo));
                  addedProcessToRunningList = true;

                  if(server.ClientConnected()) {
                     std::stringstream stringStream;
                     stringStream << "running;" << id << ";";
                     server.AddToSendQueue(stringStream.str());
                  }
               }

               break;
            }
         }
      } else {
#ifdef DEBUG_TEXT
         ErrorMessage("GetProcessImageFileName");
#endif
      }

      if(!addedProcessToRunningList) {
         CloseHandle(hProcess);
      }
   }
}