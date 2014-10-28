#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <list>
#include <iomanip>
#include "process.h"
#include "frame.h"

/* Prints the menu for getting the memory size and page size */
void printMenu(int &memorySize, int &pageSize, int &option)
{
  std::cout << "Memory size > ";
  std::cin >> memorySize;
  std::cout << "Page Size (1:100, 2:200, 3:400) > ";
  std::cin >> option;

  /* Error checking for the page size */
  while (option != 1 && option != 2 && option != 3)
  {
    std::cout << "Error: enter only 1, 2, or 3.\n";
    std::cin >> option;
  }
  switch(option) 
  {
    case 1: pageSize = 100; break;
    case 2: pageSize = 200; break;
    case 3: pageSize = 400; break;
  }
}

/*
** Prints the arrival of a process into the input queue, and also prints the
** contents of the input queue.
*/
void printArrival(int pid, int virtualClock, std::vector<Process> &inputQueue, FILE* ofs)
{
  std::cout << "t = " << virtualClock << ": " << "Process " << pid << " arrives\n"
            << "\tInput queue: [ ";
  if (fprintf(ofs, "t = %d: Process %d arrives\n\tInput queue: [ ", virtualClock,
              pid) < 0)
  {
    perror("fprintf");
  }
  size_t size = inputQueue.size();
  for (int i = 0; i < size; i++)
  {
    std::cout << inputQueue[i].pid << " ";
    if (fprintf(ofs, "%d ", inputQueue[i].pid) < 0)
    {
      perror("fprintf");
    }
  }
  std::cout << "]\n";
  if (fprintf(ofs, "]\n") < 0)
  {
    perror("fprintf");
  }
}

/* Prints the contents of the memory map */
void printMemoryMap(Frame *memoryMap, int pageSize, int memorySize, FILE* ofs)
{
  int lowerBound, upperBound;
  if (fprintf(ofs, "\tMemory map:\n") < 0)
  {
    perror("fprintf");
  }
  std::cout << "\tMemory map:\n";
  for (int i = 0; i < memorySize; i+= pageSize)
  {
    lowerBound = i;
    upperBound = i + pageSize - 1;
    if (memoryMap[i].pid == -1)
    { 
      std::cout << "\t\t" << lowerBound << "-" << upperBound 
                << ": Free frame\n";
      if (fprintf(ofs, "\t\t%d-%d: Free frame\n", lowerBound, upperBound) < 0)
      {
        perror("fprintf");
      }
    }
    else
    {
      std::cout << "\t\t" << lowerBound << "-" << upperBound << ": Process " 
                << memoryMap[i].pid << ", Page " << memoryMap[i].pageNumber
                << "\n";
      if (fprintf(ofs, "\t\t%d-%d: Process %d, Page %d\n", lowerBound,
                  upperBound, memoryMap[i].pid, memoryMap[i].pageNumber) < 0)
      {
        perror("fprintf");
      }
    }
  }
}

/*
** Frees the contents of frames that were allocated to the processs with id
** processId within the memory map. -1 represents a free address space.
*/
void freeFromMemory(Frame *memoryMap, int processId, int pageSize, int memorySize)
{
  for (int i = 0; i < memorySize; i++)
  {
    if (memoryMap[i].pid == processId)
      memoryMap[i].pid = -1;
  }
}


/*
** The memory manager unit whose purpose is to:
** 1) free memory space upon completion of a process
** 2) check for free frames in the memory map
** 3) allocate memory to a process from the input queue whose memory requirements
** can be satisfied.
*/
void memoryManager(Frame *memoryMap, std::vector<Process> &inputQueue, 
                   std::vector<Process> &processesInMemory, int pageSize,
                   int memorySize, int virtualClock, std::list<int> &pidCompleted,
                   std::vector<Process> &processesCompleted, FILE* ofs)
{
  int processId;
  size_t length;

  /* Traverse through a vector of the id's of completed processes */
  while (!pidCompleted.empty())
  {
    processId = pidCompleted.front();
    pidCompleted.pop_front();
    
    /* Free the memory previously occupied by the process with id processId */
    freeFromMemory(memoryMap, processId, pageSize, memorySize);

    /* Traverse through the vector of processes currently in memory */
    length = processesInMemory.size();
    for (int i = 0; i < length; i++)
    {
      /* If the pid of the completed process matches the pid of the process in
      ** memory, then announce its completion, and print the current memory map.
      ** Also, add it to the vector of completed processes, and erase it from
      ** the vector of processes currently in memory.
      */
      if (processesInMemory[i].pid == processId)
      {
        Process temp = processesInMemory[i];
        std::cout << "t = " << virtualClock << ": Process " << temp.pid
                  << " completes\n";
        if (fprintf(ofs, "t = %d: Process %d completes\n", virtualClock,
                    temp.pid) < 0)
        {
          perror("fprintf");
        }
        printMemoryMap(memoryMap, pageSize, memorySize, ofs);
        processesCompleted.push_back(Process(temp.pid, temp.turnaroundTime));
        processesInMemory.erase(processesInMemory.begin() + i);
      }
    }
  }        

  /* Create a list of Frames to keep track of the current free frames in memory */
  std::list<Frame> freeFrames;
  int freeLowerBound, freeUpperBound;
  for (int i = 0; i < memorySize; i+= pageSize)
  {
    /* A -1 in the memory map's pid value means that the address spaces within
    ** the bounds of the page size is free. 
    */
    if (memoryMap[i].pid == -1)
    {
      freeLowerBound = i;
      freeUpperBound = i + pageSize - 1;

      /* Add the frame to the list of free frames */
      freeFrames.push_back(Frame(freeLowerBound, freeUpperBound));
    }
  }
  
  int pageNeeds;
  size_t size;
  bool noMoreCanBeSatisfied = false;

  /* Main loop within the memory manager function. This loop's job is to:
  ** 1) Calculate page needs of a process inside the input queue
  ** 2) Assign the process memory space if its memory requirements can be met
  ** 3) Erase the process from the input queue if it has been allocated memory
  ** 4) Go through the input queue to see if any other processes can be
  **    allocated memory space
  */
  while (true && !inputQueue.empty())
  {
    size = inputQueue.size();
    for(int i = 0; i < size; i++)
    {
      /* Round up to get pages needed by the process if its the page size cannot
      ** be evenly divided by its memory requirements.
      */
      if ((inputQueue[i].memoryRequirements % pageSize) != 0)
        pageNeeds = (inputQueue[i].memoryRequirements / pageSize) + 1;
      else
        pageNeeds = inputQueue[i].memoryRequirements / pageSize;

      /* If the page needs of a process is less than the amount of free frames */
      if (pageNeeds <= freeFrames.size())
      {
        /* Allocate each needed frame to the process */
        for(int j = 0; j < pageNeeds; j++)
        {
          freeLowerBound = freeFrames.front().lowerBound;
          freeUpperBound = freeFrames.front().upperBound;
          for (; freeLowerBound <= freeUpperBound; freeLowerBound++)
          {
            memoryMap[freeLowerBound].pid = inputQueue[i].pid;
            memoryMap[freeLowerBound].pageNumber = j + 1;
          }
          /* Remove the frame from the free frame list */
          freeFrames.pop_front();
        }
        Process temp = inputQueue[i];

        /* Add the process into the vector of processes currently in memory */
        processesInMemory.push_back(Process(temp.pid, temp.arrivalTime,
                                            temp.lifetimeInMemory, 
                                            temp.memoryRequirements));

        /* Set the admitted time and turnaround time of the process */
        processesInMemory.back().setAdmittedTime(virtualClock);
        processesInMemory.back().setTurnaroundTime();

        /* Announce that the process has been moved to memory */
        std::cout << "\tMM moves Process " << inputQueue[i].pid << " to memory.\n";
        if (fprintf(ofs, "\tMM moves Process %d to memory.\n", 
                    inputQueue[i].pid) < 0)
        {
          perror("fprintf");
        }

        /* Print the current memory map */
        printMemoryMap(memoryMap, pageSize, memorySize, ofs);

        /* Erase the process from the input queue */
        inputQueue.erase(inputQueue.begin() + i);
        break;
      }
      /* If i == size - 1, it means that there are no more processes currently
      ** in the input queue whose memory requirements can be satisfied, so we
      ** should break out of the loop
      */
      else if (i == size - 1)
      {
        noMoreCanBeSatisfied = true;
        break;
      }
    }
    /* If no more processes can be satisfied for the moment, break */
    if (noMoreCanBeSatisfied)
      break;
  }
  /* Clear the list of free frames */
  freeFrames.clear();
}

int main()
{
  /* Our virtual clock */
  int virtualClock = 0;

  /* Create a dynamic array of Frames representing the memory map */
  Frame *memoryMap;

  /* The option variable, if 1 means the page size is 100, 2: 200, and 3: 400 */
  int option;

  /* Declare and get memory and page size */
  int memorySize, pageSize;
  printMenu(memorySize, pageSize, option);

  /* Initialize the memoryMap dynamically, and each element of the array to -1 
  ** to symbolize that the memory is free for allocation to a process
  */
  memoryMap = new Frame[memorySize];
  for (int i = 0; i < memorySize; i++)
    memoryMap[i].pid = -1;
  
  
  /* Ask user for workload file */
  char fileName[256];
  std::cout << "Enter the name of the workload file > ";
  std::cin >> fileName;

  /* Create input file stream and open file for reading */
  FILE *ifs;
  ifs = fopen(fileName, "r");

  /* Error checks the stream */
  if (!ifs)
  {
    perror("fopen");
    exit(-1);
  }

  /* Scan the first integer in the file to get number of processes */
  int numberProcesses;
  fscanf(ifs, "%d", &numberProcesses);

  /* Create counter variables to count how many processes have completed
  ** execution and how many processes information have been parsed from the
  ** file
  */
  int processesParsed;
  processesParsed = 0;

  /* A vector representing the input queue */
  std::vector<Process> inputQueue;

  /* A vector representing processes currently in memory */
  std::vector<Process> processesInMemory;

  /* A vector representing processes that have already finished execution */
  std::vector<Process> processesCompleted;

  /* List of all incoming processes, even ones that haven't arrived */
  std::list<Process> processesList;

  /* Parse through the input file for process information and add all of the
  ** processes to a processesList
  */
  while (processesParsed != numberProcesses)
  {
    int numberOfPieces, pid, arrivalTime, lifetimeInMemory;
    fscanf(ifs, "%d %d %d", &pid, &arrivalTime, &lifetimeInMemory);
    fscanf(ifs, "%d", &numberOfPieces);
    int totalMemoryReq = 0;
    int temp = 0;
    for (int i = 0; i < numberOfPieces; i++)
    {
      fscanf(ifs, "%d", &temp);
      totalMemoryReq += temp;
    }
    processesList.push_back(Process(pid, arrivalTime, lifetimeInMemory, 
                                    totalMemoryReq));
    processesParsed++;
  }

  /* Error checks for closing the stream */
  if (fclose(ifs) != 0)
  {
    perror("fclose");
    exit(-1);
  }

  /* Create a list to store the process id of completed processes */
  std::list<int> pidCompleted;

  /* Create a file stream for our output */
  FILE* ofs;
  char outFileName[9];
  if (sprintf(outFileName, "Out%d.txt", option) < 0)
  {
    perror("sprintf");
  }
  ofs = fopen(outFileName, "w");
  if (!ofs)
  {
    perror("fopen");
  }

  /* The main loop, each loop iteration will increment the virtual clock by 1.
  ** Exit condition is when the amount of processes completed == number of
  ** processes.
  */
  while (processesCompleted.size() != numberProcesses)
  {
    /* While the list of processes is not empty, if the virtual clock is greater 
    ** than or equal to the processes' arrival time, they will be put into the 
    ** input queue
    */
    while (!processesList.empty())
    {
      if (virtualClock >= processesList.front().arrivalTime)
      {
        Process temp = processesList.front();
        inputQueue.push_back(Process(temp.pid, temp.arrivalTime,
                                     temp.lifetimeInMemory, 
                                     temp.memoryRequirements));

        /* Remove the process from the processesList after it has been put into
        ** the input queue
        */
        processesList.pop_front();

        /* Print a statement announcing the arrival of a processes to the input
        ** queue and show the current memory map, then invoke the memory
        ** manager
        */
        printArrival(temp.pid, virtualClock, inputQueue, ofs);
        printMemoryMap(memoryMap, pageSize, memorySize, ofs);
        memoryManager(memoryMap, inputQueue, processesInMemory, pageSize,
                      memorySize, virtualClock, pidCompleted, processesCompleted, ofs);
      }
      else
        break;
    }

    /* Check for completion of any of the processes in memory by comparing their
    ** turnaround time with the current virtual clock time.
    */
    for(int i = 0; i < processesInMemory.size(); i++)
    {
      if (processesInMemory[i].completedTime == virtualClock)
      {
        pidCompleted.push_back(processesInMemory[i].pid);
      }
    }
    
    /* Invoke the memory manager */
    memoryManager(memoryMap, inputQueue, processesInMemory, pageSize,
                  memorySize, virtualClock, pidCompleted, processesCompleted, ofs);

    /* Increment the virtual clock */
    virtualClock++;

    /* If the virtual clock is 100000, break as instructed by the assignment */
    if (virtualClock == 100000)
      break;
  }
  
  /* Calculate the average turnaround time by summing the total turnaround
  ** time from the processes completed vector and dividing it by the number of
  ** processes completed.
  */
  int processesCompletedSize = processesCompleted.size();
  int totalTurnaroundTime = 0;
  float averageTurnaroundTime;

  for (int i = 0; i < processesCompletedSize; i++)
    totalTurnaroundTime += processesCompleted[i].turnaroundTime;

  averageTurnaroundTime = (float)totalTurnaroundTime / processesCompletedSize;
  std::cout << "Average turnaround: " << std::fixed << std::setprecision(2)
            << averageTurnaroundTime << " (" << totalTurnaroundTime << "/" 
            << processesCompletedSize << ")\n";

  if (fprintf(ofs, "Average turnaround: %.2f (%d/%d)\n", averageTurnaroundTime,
              totalTurnaroundTime, processesCompletedSize) < 0)
  {
    perror("fprintf");
  }

  /* Close the file */
  if (fclose(ofs) != 0)
  {
    perror("fclose");
  }

  /* Deallocate the memory map */
  delete[] memoryMap;

  return 0;
}
