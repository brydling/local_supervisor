#ifndef PROCESSFUNCTIONS_H
#define PROCESSFUNCTIONS_H

#include <WinDef.h>

void StartProcess(DWORD id, bool startMinimized);
void StopProcess(DWORD id);
void KillProcess(DWORD id);
void FindAndEraseStoppedProcesses();

#endif