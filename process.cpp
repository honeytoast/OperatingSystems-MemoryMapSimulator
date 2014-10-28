#include "process.h"
#include <iostream>

/* Construct a process object with a process id and its turnaround time.
** This is used for the vectors of completed processes because the only
** information we need to retain is the process id and turnaround time.
*/
Process::Process(int processId, int _turnaroundTime)
{
  pid = processId;
  turnaroundTime = _turnaroundTime;
}

/* Construct a process object with a process id, arrival(time), lifetime(in 
** memory), and memory requirements.
*/
Process::Process(int processId, int arrival, int lifetime, int memoryReq)
{
  pid = processId;
  arrivalTime = arrival;
  lifetimeInMemory = lifetime;
  memoryRequirements = memoryReq;
}

/* Print info of a process, used for documenting */
void Process::printInfo()
{
  std::cout << "Pid is " << pid << std::endl;
  std::cout << "Arrival time " << arrivalTime << std::endl;
  std::cout << "Lifetime " << lifetimeInMemory << std::endl;
  std::cout << "Mem requirements " << memoryRequirements << std::endl;
}

/* Set the admitted time of a process */
void Process::setAdmittedTime(int virtualClock)
{
  admittedTime = virtualClock;
}

/* Set the turnaround time of a process after calculating the time of its
** completion.
*/
void Process::setTurnaroundTime()
{
  completedTime = admittedTime + lifetimeInMemory;
  turnaroundTime = completedTime - arrivalTime;
}
