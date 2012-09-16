#ifndef PROCESSFUNCTIONS_H
#define PROCESSFUNCTIONS_H

#include <WinDef.h>

void StartProcess(DWORD id);
void StopProcess(DWORD id);
void FindAndEraseStoppedProcesses();

#endif