OS Process Scheduling Simulator
A comprehensive C++ simulator for major CPU scheduling algorithms, including:

First Come First Serve (FCFS)

Shortest Job First (SJF) – Non-Preemptive & Preemptive (SRTF)

Priority Scheduling – Non-Preemptive & Preemptive

Round Robin (RR)

Multilevel Queue (MLQ)

Multilevel Feedback Queue (MLFQ)

Features
Random or manual process input

Context switch overhead simulation

Automatic execution of all algorithms

Gantt chart and tabular performance metrics for each algorithm

Modular, well-commented code using C++ classes

Usage
Clone the repository:

bash
git clone https://github.com/yourusername/os-scheduler-simulator.git
cd os-scheduler-simulator
Compile the code:

bash
g++ -std=c++11 src/main.cpp -o scheduler
Run the program:

bash
./scheduler
Enter the number of processes, context switch overhead, and input mode (random/manual).

View the results for each scheduling algorithm.

Example Output
text
===== FCFS =====
Gantt Chart:
| P0 | P1 | P2 |
0    5    12   18

PID   AT   BT   PRI   CT   TAT   WT   RT
P0    0    5    3     5    5     0    0
...
Algorithms Implemented
Algorithm	Preemptive	Non-Preemptive	Notes
FCFS		✓	Simple FIFO
SJF	✓	✓	SRTF for preemptive
Priority	✓	✓	Lower value = higher priority
RR	✓		Quantum configurable
MLQ			Multiple fixed queues
MLFQ	✓		Queues with feedback
File Structure
File/Folder	Description
src/main.cpp	Main C++ source code
README.md	Project documentation
.gitignore	Ignore build files, binaries
Customization
Adjust quantum or context switch overhead in the code or at runtime.

Modify process assignment logic for MLQ/MLFQ as desired.

License
MIT License (add a LICENSE file if you want to open-source it).

Contributing
Pull requests and suggestions are welcome!






