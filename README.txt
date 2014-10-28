/* Name: Grace Hadiyanto 
** 
** Included files: README.txt
**                 in1.txt (sample workload input file)
**                 Out2.txt (sample output that corresponds to choosing page size option 2 with the sample input file)
**                 makefile
**                 frame.h
**                 process.h
**                 process.cpp
**                 main.cpp
*/

How to Compile: cd to the directory of the project
                type 'make' in the terminal
                type 'make clean' to remove object files afterwards
                the compiled executable will be called memorysimulator

How to Run: type './memorysimulator' and the program will ask for the
            memory size and page size option. Then it will ask for a workload
            input file.

            The program will print results to the terminal and also to a file
            called Out{option}.txt where option is the option number for the
            page size selected.

            for example: selecting a page size option of 3: 400 will set the
            page size to 400 and the output file will be called Out3.txt

Details about implementation:

              To implement the memory simulator, I first created two structs.
              The Frame data structure holds details about a frame in memory.
              The Process data structure holds information about a process.
              These structs are used within vectors and lists to keep track
              of the memoryMap and processes in the system.

              The virtual clock is implemented with a long and will be
              incremented with each iteration of the main loop in the main
              function.

              The memory map is a dynamically allocated array of Frames and
              each Frame in the memory map that has a pid of -1 will represent
              a free frame.
              
              Each time a process arrives, there will be a print statement
              announcing the time and its arrival, then the current memory map
              will be printed. Each time a process completes, there will be a
              statement announcing its completion and the newly updated
              memory map after its deallocation from memory.

              Throughout its lifetime, the process moves through 3 vectors and
              1 list.
              I chose vectors as my main data structure to work with so I can
              have fast and safe access to the container contents.
              Sequentially, a process would go through the processesList after
              it is read in from the file. It would then be removed from the
              process list, and be put into the input queue after it has
              "arrived". Once it has been allocated memory by the memory manager,
              it will be put into the processes in memory vector, and removed
              from the input queue. Lastly, after its completion, it would be
              removed from the processes in memory vector and put into the
              process completed vector. Each time a process or processes have
              arrived, the memory manager will be invoked. The memory manager
              will also be invoked each time a process or processes have
              completed.

              The memory manager will first check the contents of the pid
              completed list. If any processes pid was on the list, it must
              have completed, and the memory manager will deallocate memory 
              from that process, and remove that process from processes in memory,
              and add that process into the processes completed vector. Then 
              the memory manager will check for any free frames and try to
              allocate frames to a process from the input queue. If the head
              of the input queue's memory requirements can't be satisfied, it
              will check the next process in the queue, and so on, until no
              processes in the input queue can be satisfied.

              All printed statements in the terminal will also be printed to the
              output file.              

Compiled and tested with: Mac OSx g++ and clang++ compiler
                          Ubuntu 12.04LTS g++ compiler
