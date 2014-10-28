#ifndef PROCESS_H
#define PROCESS_H

/* Structure of a process */
struct Process {
  int pid;
  int arrivalTime;
  int lifetimeInMemory;
  int memoryRequirements;
  int admittedTime;
  int completedTime;
  int turnaroundTime;

  Process(int processId, int _turnaroundTime);
  Process(int processId, int arrival, int lifetime, int memoryReq);
  void printInfo();
  void setAdmittedTime(int virtualClock);
  void setTurnaroundTime();
};

#endif
