#ifndef ADDPROCESSESNOTSTARTEDBYSUPERVISOR_H
#define ADDPROCESSESNOTSTARTEDBYSUPERVISOR_H

/* Searches through all Windows processes to catch any of our processes that
   may have been started manually outside of this application. Any processes
   matching the executables that we are configured start and stop will be
   added to the process list.
*/
void AddProcessesNotStartedBySupervisor();

#endif