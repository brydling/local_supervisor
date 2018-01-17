#include "stdafx.h"
#include "ProcessFunctions.h"
#include "global.h"
#include <sstream>

// Declare Callback Enum Functions.
BOOL CALLBACK CloseProcessEnum(HWND hwnd, LPARAM lParam);

void StartProcess(DWORD id, bool startMinimized) {
   RunningProcessInfo runningProcessInfo;

   /* MSDN: The Unicode version of CreateProcess can modify the contents of lpCommandLine.
   * Therefore, this parameter cannot be a pointer to read-only memory (such as a const variable or a
   * literal string). If this parameter is a constant string, the function may cause an access violation.
   */
   STARTUPINFO siStartupInfo;
   memset(&siStartupInfo, 0, sizeof(siStartupInfo));
   siStartupInfo.cb = sizeof(siStartupInfo);
   
   if(startMinimized) {
      siStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
      siStartupInfo.wShowWindow = SW_SHOWMINNOACTIVE;
   } else {
      siStartupInfo.dwFlags = 0;
   }

   PROCESS_INFORMATION piProcessInfo;
   memset(&piProcessInfo, 0, sizeof(piProcessInfo));
   char* commandLine = new char[availableProcesses[id].commandLine.length()+1];
   strncpy(commandLine, availableProcesses[id].commandLine.c_str(), availableProcesses[id].commandLine.length()+1);
   CreateProcess(NULL, commandLine, NULL, NULL, FALSE, NULL, NULL, availableProcesses[id].currentDir.c_str(), &siStartupInfo, &piProcessInfo);
   runningProcessInfo.dwProcessId = piProcessInfo.dwProcessId;
   runningProcessInfo.hProcess = piProcessInfo.hProcess;
   CloseHandle(piProcessInfo.hThread);

   runningProcesses.insert(std::pair<unsigned int, RunningProcessInfo>(id, runningProcessInfo));

   DWORD exitCode;
   DWORD success = GetExitCodeProcess(runningProcessInfo.hProcess, &exitCode);
   DWORD error = GetLastError();

   if(exitCode == STILL_ACTIVE) {
      if(server.ClientConnected()) {
         std::stringstream stringStream;
         stringStream << "running;" << id << ";";
         server.AddToSendQueue(stringStream.str());
      }
   }
}

void StopProcess(DWORD id) {
   DWORD dwPID = runningProcesses[id].dwProcessId;

   // Enumerate all top-level windows belonging to the PID we want to stop.
   // CloseProcessEnum will be called for each window found.
   if (!EnumWindows((WNDENUMPROC)CloseProcessEnum, (LPARAM)dwPID)) {
      DWORD errorCode = GetLastError();
      std::string errMsg = "Error when stopping process #" + std::to_string(id) + "\nGetLastError returned: " + std::to_string(errorCode);
      MessageBoxA(NULL, errMsg.c_str(), "Error", MB_OK);
   }
}

void KillProcess(DWORD id) {
   HANDLE hProc = runningProcesses[id].hProcess;
   DWORD dwTimeout = 1000;
   
   // Give the poor guy a chance
   StopProcess(id);

   // Wait on the handle. If it signals, great. If it times out, kill it.
   if(WaitForSingleObject(hProc, dwTimeout) != WAIT_OBJECT_0)
      TerminateProcess(hProc,0);
}

BOOL CALLBACK CloseProcessEnum(HWND hwnd, LPARAM lParam) {
   DWORD dwID;

   GetWindowThreadProcessId(hwnd, &dwID);

   if(dwID == (DWORD)lParam) {
      PostMessage(hwnd, WM_CLOSE, 0, 0);
   }

   return TRUE;
}

void FindAndEraseStoppedProcesses() {
   std::vector<unsigned int> process_ids_to_erase;
   std::map<unsigned int, RunningProcessInfo>::iterator it;
   for(it = runningProcesses.begin(); it != runningProcesses.end(); it++) {
      RunningProcessInfo runningProcess = it->second;
      unsigned int id = it->first;

      DWORD exitCode;
      if (!GetExitCodeProcess(runningProcess.hProcess, &exitCode)) {
         DWORD error = GetLastError();
         std::string errMsg = "Error calling GetExitCodeProcess. GetLastError returned: " + std::to_string(error);
         MessageBoxA(NULL, errMsg.c_str(), "Error", MB_OK);
      }

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