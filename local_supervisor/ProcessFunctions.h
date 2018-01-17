#ifndef PROCESSFUNCTIONS_H
#define PROCESSFUNCTIONS_H

#include <WinDef.h>

/* Function for starting one of the processes.
   ARGS:
      id (in):
         The ID of the process (in the process config file).
      startMinimized (in):
         Set to true to start the process minimized.
*/
void StartProcess(DWORD id, bool startMinimized);

/* Function for stopping one of the processes.
   ARGS:
      id (in):
         The ID of the process (in the process config file).
*/
void StopProcess(DWORD id);

/* Function for killing one of the processes.
   ARGS:
      id (in):
         The ID of the process (in the process config file).
*/
void KillProcess(DWORD id);

/* Function for finding if any of our started processes have been stopped
   and erasing them from our list of started processes.
*/
void FindAndEraseStoppedProcesses();

#endif