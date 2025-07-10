#include<bits/stdc++.h>
using namespace std;

// Process class
class Process {
public:
    int pid;
    int arrival;
    int burst;
    int burst_remain;
    int priority;
    int start_time = -1;
    int completion = 0;
    int turnaround = 0;
    int waiting = 0;
    int response = -1;
    int queue_level = 0; // For MLQ/MLFQ

    Process(int id, int a, int b, int p){
        pid=id;
        arrival=a;
        burst=b;
        burst_remain=b;
        priority=p;
    }
};

struct GanttEntry {
    int pid;
    int start;
    int end;
    bool context_switch;
};

class Scheduler {
    int context_switch_overhead;
    vector<Process> processes;
    vector<GanttEntry> gantt;

public:
    Scheduler(vector<Process> plist, int cs_overhead) {
        processes=plist; 
        context_switch_overhead=cs_overhead;
    }

    void resetProcesses() {
        for (auto& p : processes) {
            p.burst_remain = p.burst;
            p.start_time = -1;
            p.completion = 0;
            p.turnaround = 0;
            p.waiting = 0;
            p.response = -1;
            p.queue_level = 0;
        }
        gantt.clear();
    }

    // FCFS
    void runFCFS() {
        resetProcesses();
        vector<Process> plist = processes;
        sort(plist.begin(), plist.end(), [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

        int time = 0, prev_pid = -1;
        for (auto& p : plist) {
            if (time < p.arrival) 
                time = p.arrival;
            if (prev_pid != -1 && prev_pid != p.pid)
                time += context_switch_overhead;
            p.start_time = time;
            p.response = time - p.arrival;
            time += p.burst;
            p.completion = time;
            p.turnaround = p.completion - p.arrival;
            p.waiting = p.turnaround - p.burst;
            gantt.push_back({p.pid, p.start_time, p.completion, prev_pid != -1 && prev_pid != p.pid});
            prev_pid = p.pid;
        }
        printResults("FCFS", plist);
    }

    // SJF Non-Preemptive
    // Time Complexity: O(n log n)
    void runSJF_NP() {
        resetProcesses(); // Resets all process stats
        vector<Process> plist = processes;
        int n = plist.size();
        int completed = 0, time = 0, prev_pid = -1;

        // Sort processes by arrival time
        sort(plist.begin(), plist.end(), [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

        // Min-heap to pick process with shortest burst: {burst_time, index_in_plist}
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;

        int i = 0; // Pointer to track processes that have arrived

        while (completed < n) {
            // Push all newly arrived processes into the heap
            while (i < n && plist[i].arrival <= time) {
                pq.push({plist[i].burst, i});
                ++i;
            }

            if (pq.empty()) {
                time++; // No process ready, advance time
                continue;
            }

            auto [burst, idx] = pq.top(); pq.pop();

            // Apply context switch overhead if needed
            if (prev_pid != -1 && prev_pid != plist[idx].pid)
                time += context_switch_overhead;

            // Start and complete the selected process
            plist[idx].start_time = time;
            plist[idx].response = time - plist[idx].arrival;
            time += plist[idx].burst;
            plist[idx].completion = time;
            plist[idx].turnaround = plist[idx].completion - plist[idx].arrival;
            plist[idx].waiting = plist[idx].turnaround - plist[idx].burst;

            // Add to Gantt chart with context switch info
            gantt.push_back({
                plist[idx].pid,
                plist[idx].start_time,
                plist[idx].completion,
                (prev_pid != -1 && prev_pid != plist[idx].pid)
            });

            prev_pid = plist[idx].pid;
            completed++;
        }

    printResults("SJF (NP)", plist);
}



    // SJF Preemptive
    // Time Complexity: O(B log n) where B = total burst time
    void runSJF_P() {
        resetProcesses();
        vector<Process> plist = processes;
        int n = plist.size(), completed = 0, time = 0, prev_pid = -1;
        vector<bool> in_heap(n, false);

        // Min-heap: {burst_remain, arrival, index}
        priority_queue<tuple<int, int, int>, vector<tuple<int, int, int>>, greater<tuple<int, int, int>>> pq;

        while (completed < n) {
            for (int i = 0; i < n; ++i)
                if (!in_heap[i] && plist[i].arrival <= time && plist[i].burst_remain > 0)
                    pq.push({plist[i].burst_remain, plist[i].arrival, i}), in_heap[i] = true;

            if (pq.empty()) { time++; continue; }

            auto [burst_remain, arrival, idx] = pq.top(); pq.pop();
            if (prev_pid != -1 && prev_pid != plist[idx].pid)
                time += context_switch_overhead;
            if (plist[idx].start_time == -1)
                plist[idx].start_time = time, plist[idx].response = time - plist[idx].arrival;

            // Run for one unit (preemptive)
            gantt.push_back({plist[idx].pid, time, time + 1, prev_pid != -1 && prev_pid != plist[idx].pid});
            plist[idx].burst_remain--;
            time++;
            if (plist[idx].burst_remain == 0) {
                plist[idx].completion = time;
                plist[idx].turnaround = plist[idx].completion - plist[idx].arrival;
                plist[idx].waiting = plist[idx].turnaround - plist[idx].burst;
                completed++;
            } else {
                pq.push({plist[idx].burst_remain, plist[idx].arrival, idx});
            }
            prev_pid = plist[idx].pid;
        }
        printResults("SJF (P)", plist);
    }


    // Priority Non-Preemptive
    // Time Complexity: O(n log n)
    void runPriority_NP() {
        resetProcesses();
        vector<Process> plist = processes;
        int n = plist.size(), completed = 0, time = 0, prev_pid = -1;

        // Sort by arrival time
        sort(plist.begin(), plist.end(), [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

        // Min-heap: {priority, arrival_time, index}
        priority_queue<tuple<int, int, int>, vector<tuple<int, int, int>>, greater<>> pq;
        int i = 0; // Pointer for arrived processes

        while (completed < n) {
            // Push all processes that have arrived by current time
            while (i < n && plist[i].arrival <= time) {
                pq.push({plist[i].priority, plist[i].arrival, i});
                ++i;
            }

            if (pq.empty()) {
                time++; // No process ready, advance time
                continue;
            }

            auto [priority, arrival, idx] = pq.top(); pq.pop();

            // Apply context switch if changing process
            if (prev_pid != -1 && prev_pid != plist[idx].pid)
                time += context_switch_overhead;

            // Set process timing details
            plist[idx].start_time = time;
            plist[idx].response = time - plist[idx].arrival;
            time += plist[idx].burst;
            plist[idx].completion = time;
            plist[idx].turnaround = plist[idx].completion - plist[idx].arrival;
            plist[idx].waiting = plist[idx].turnaround - plist[idx].burst;

            // Log to Gantt chart
            gantt.push_back({
                plist[idx].pid,
                plist[idx].start_time,
                plist[idx].completion,
                (prev_pid != -1 && prev_pid != plist[idx].pid)
            });

            prev_pid = plist[idx].pid;
            completed++;
        }

        printResults("Priority (NP)", plist);
    }


// lower number-->higher priority

    // Priority Preemptive 
// Time Complexity: O(B log n) where B = total burst time
    void runPriority_P() {
        resetProcesses();
        vector<Process> plist = processes;
        int n = plist.size(), completed = 0, time = 0, prev_pid = -1;

        // Sort by arrival time
        sort(plist.begin(), plist.end(), [](const Process &a, const Process &b) {
            return a.arrival < b.arrival;
        });

        // Min-heap: {priority, arrival, index}
        priority_queue<tuple<int, int, int>, vector<tuple<int, int, int>>, greater<>> pq;
        int ind = 0; // Pointer to track arriving processes

        while (completed < n) {
            // Push all newly arrived processes into the heap
            while (ind < n && plist[ind].arrival <= time) {
                if (plist[ind].burst_remain > 0) {
                    pq.push({plist[ind].priority, plist[ind].arrival, ind});
                }
                ++ind;
            }

            if (pq.empty()) {
                time++; // Idle time
                continue;
            }

            auto [priority, arrival, idx] = pq.top(); pq.pop();

            // Apply context switch overhead
            if (prev_pid != -1 && prev_pid != plist[idx].pid)
                time += context_switch_overhead;

            // Record start/response time
            if (plist[idx].start_time == -1) {
                plist[idx].start_time = time;
                plist[idx].response = time - plist[idx].arrival;
            }

            // Run for 1 time unit
            gantt.push_back({plist[idx].pid, time, time + 1, (prev_pid != -1 && prev_pid != plist[idx].pid)});
            plist[idx].burst_remain--;
            time++;

            // If finished, calculate completion stats
            if (plist[idx].burst_remain == 0) {
                plist[idx].completion = time;
                plist[idx].turnaround = plist[idx].completion - plist[idx].arrival;
                plist[idx].waiting = plist[idx].turnaround - plist[idx].burst;
                completed++;
            } else {
                pq.push({plist[idx].priority, plist[idx].arrival, idx}); // Reinsert for next time
            }

            prev_pid = plist[idx].pid;
        }

        printResults("Priority (P)", plist);
    }

    // Round Robin O(n+B)+O(nlog n)
    void runRR(int quantum) {
        resetProcesses();
        vector<Process> plist = processes;
        int n = plist.size(), completed = 0, time = 0, prev_pid = -1;

        queue<int> ready;
        vector<bool> in_queue(n, false);

        // Sort by arrival time
        sort(plist.begin(), plist.end(), [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

        int ind = 0; // Pointer to next process to check for arrival

        // Initial load of ready queue (arrival time = 0)
        while (ind < n && plist[ind].arrival <= time) {
            ready.push(ind);
            in_queue[ind] = true;
            ind++;
        }

        while (completed < n) {
            if (ready.empty()) {
                time++; // Advance time if nothing is ready
                while (ind < n && plist[ind].arrival <= time) {
                    ready.push(ind);
                    in_queue[ind] = true;
                    ind++;
                }
                continue;
            }

            int idx = ready.front(); ready.pop();

            if (prev_pid != -1 && prev_pid != plist[idx].pid)
                time += context_switch_overhead;

            if (plist[idx].start_time == -1) {
                plist[idx].start_time = time;
                plist[idx].response = time - plist[idx].arrival;
            }

            int exec = min(quantum, plist[idx].burst_remain);
            gantt.push_back({plist[idx].pid, time, time + exec, prev_pid != -1 && prev_pid != plist[idx].pid});
            time += exec;
            plist[idx].burst_remain -= exec;

            // Push any processes that arrived during execution
            while (ind < n && plist[ind].arrival <= time) {
                ready.push(ind);
                in_queue[ind] = true;
                ind++;
            }

            if (plist[idx].burst_remain == 0) {
                plist[idx].completion = time;
                plist[idx].turnaround = plist[idx].completion - plist[idx].arrival;
                plist[idx].waiting = plist[idx].turnaround - plist[idx].burst;
                completed++;
            } else {
                ready.push(idx); // Put it back in queue
            }

            prev_pid = plist[idx].pid;
        }

        printResults("RR", plist);
    }

    // MLQ
    void runMLQ() {
        resetProcesses();
        vector<Process> q1, q2, q3;
        for (auto p : processes) {
            if (p.priority >= 7) q1.push_back(p);      // Highest  round robin with 4 quanta
            else if (p.priority >= 4) q2.push_back(p); // Medium   priority np
            else q3.push_back(p);                      // Lowest   fcfs
        }

        int time = 0, prev_pid = -1;
        vector<GanttEntry> mlq_gantt;
        int total = q1.size() + q2.size() + q3.size();

        int completed = runQueueRR(q1, 4, time, prev_pid, mlq_gantt);
        completed += runQueuePriorityNP(q2, time, prev_pid, mlq_gantt);
        completed += runQueueFCFS(q3, time, prev_pid, mlq_gantt);

        vector<Process> all;   // merge meine process nhi use kiya kyunki humko grouped process chahiye with time and all ki values
        all.insert(all.end(), q1.begin(), q1.end());
        all.insert(all.end(), q2.begin(), q2.end());
        all.insert(all.end(), q3.begin(), q3.end());

        gantt = mlq_gantt;
        printResults("MLQ", all);
    }

    // MLFQ
    void runMLFQ() {
        resetProcesses(); 
        vector<Process> plist = processes;
        int n = plist.size(), completed = 0, time = 0, prev_pid = -1;

        // Sort 
        sort(plist.begin(), plist.end(), [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

        // 3 priority queues: Q1 (highest), Q2 (medium), Q3 (lowest)
        queue<int> q1, q2, q3;
        int next = 0;

        const int q1_quantum = 8, q2_quantum = 16;
        while (completed < n) {
            // Add all processes that have arrived by current time to Q1
            while (next < n && plist[next].arrival <= time)
                q1.push(next++);

            int idx = -1, exec = 0, level = 0;   // every process starts with highest priority

            // Select next process from the highest non-empty queue
            if (!q1.empty()) {
                idx = q1.front(); q1.pop();
                exec = min(q1_quantum, plist[idx].burst_remain);
                level = 1;
            } else if (!q2.empty()) {
                idx = q2.front(); q2.pop();
                exec = min(q2_quantum, plist[idx].burst_remain);
                level = 2;
            } else if (!q3.empty()) {
                idx = q3.front(); q3.pop();
                exec = plist[idx].burst_remain;
                level = 3;
            } else {
                // If all queues are empty, increase time by one
                time++;
                continue;
            }

            if (prev_pid != -1 && prev_pid != plist[idx].pid)
                time += context_switch_overhead;

            if (plist[idx].start_time == -1) // first time scheduling of this process
                plist[idx].start_time = time, plist[idx].response = time - plist[idx].arrival;

            gantt.push_back({plist[idx].pid, time, time + exec, prev_pid != -1 && prev_pid != plist[idx].pid});
            time += exec;
            plist[idx].burst_remain -= exec;

            while (next < n && plist[next].arrival <= time)
                q1.push(next++);

            if (plist[idx].burst_remain == 0) {
                plist[idx].completion = time;
                plist[idx].turnaround = time - plist[idx].arrival;
                plist[idx].waiting = plist[idx].turnaround - plist[idx].burst;
                completed++;
            } else {    // concpet of aging process is demotes to lower priority queue after one burst so prevent starvation of lower queues
                if (level == 1)
                    q2.push(idx);  // From Q1 → Q2
                else
                    q3.push(idx);  // From Q2 or Q3 → Q3 (FCFS)
            }

            prev_pid = plist[idx].pid;
        }

        // Output results
        printResults("MLFQ", plist);
    }

    // Helpers for MLQ
    int runQueueRR(vector<Process>& q, int quantum, int& time, int& prev_pid, vector<GanttEntry>& gantt) {
        sort(q.begin(), q.end(), [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

        int n = q.size();
        int completed = 0;
        vector<int> ready;
        int next = 0;

        while (completed < n) {
            // Load all processes that have arrived
            while (next < n && q[next].arrival <= time)
                ready.push_back(next++);

            if (ready.empty()) {
                time++;
                continue;
            }

            int idx = ready.front();
            ready.erase(ready.begin());

            if (prev_pid != -1 && prev_pid != q[idx].pid)
                time += context_switch_overhead;

            if (q[idx].start_time == -1)
                q[idx].start_time = time, q[idx].response = time - q[idx].arrival;

            int exec = min(quantum, q[idx].burst_remain);
            gantt.push_back({q[idx].pid, time, time + exec, prev_pid != -1 && prev_pid != q[idx].pid});

            time += exec;
            q[idx].burst_remain -= exec;

            // Load newly arrived processes during this execution
            while (next < n && q[next].arrival <= time)
                ready.push_back(next++);

            if (q[idx].burst_remain == 0) {
                q[idx].completion = time;
                q[idx].turnaround = q[idx].completion - q[idx].arrival;
                q[idx].waiting = q[idx].turnaround - q[idx].burst;
                completed++;
            } else {
                ready.push_back(idx); // push back for next round
            }

            prev_pid = q[idx].pid;
        }
        return completed;
    }

    int runQueuePriorityNP(vector<Process>& q, int& time, int& prev_pid, vector<GanttEntry>& gantt) {
        sort(q.begin(), q.end(), [](const Process& a, const Process& b) {
            return a.arrival < b.arrival;
        });

        int n = q.size(), completed = 0;
        vector<bool> done(n, false);

        while (completed < n) {
            int idx = -1, best_priority = INT_MAX;

            for (int i = 0; i < n; ++i) {
                if (!done[i] && q[i].arrival <= time && q[i].priority < best_priority) {
                    best_priority = q[i].priority;
                    idx = i;
                }
            }

            if (idx == -1) {
                time++;
                continue;
            }

            if (prev_pid != -1 && prev_pid != q[idx].pid)
                time += context_switch_overhead;

            q[idx].start_time = time;
            q[idx].response = time - q[idx].arrival;

            gantt.push_back({q[idx].pid, time, time + q[idx].burst, prev_pid != -1 && prev_pid != q[idx].pid});

            time += q[idx].burst;
            q[idx].completion = time;
            q[idx].turnaround = q[idx].completion - q[idx].arrival;
            q[idx].waiting = q[idx].turnaround - q[idx].burst;

            done[idx] = true;
            completed++;
            prev_pid = q[idx].pid;
        }
        return completed;
    }

    int runQueueFCFS(vector<Process>& q, int& time, int& prev_pid, vector<GanttEntry>& local_gantt) {
        int finished = 0;
        sort(q.begin(), q.end(), [](const Process& a, const Process& b) { return a.arrival < b.arrival; });
        for (auto& p : q) {
            if (time < p.arrival) time = p.arrival;
            if (prev_pid != -1 && prev_pid != p.pid)
                time += context_switch_overhead;
            p.start_time = time;
            p.response = time - p.arrival;
            local_gantt.push_back({p.pid, time, time + p.burst, prev_pid != -1 && prev_pid != p.pid});
            time += p.burst;
            p.completion = time;
            p.turnaround = p.completion - p.arrival;
            p.waiting = p.turnaround - p.burst;
            finished++;
            prev_pid = p.pid;
        }
        return finished;
    }

    // Print Gantt chart and process table
    void printResults(const string& algo, const vector<Process>& plist) {
        cout << "\n===== " << algo << " =====\n";
        cout << "Gantt Chart:\n|";
        for (auto& g : gantt)
            cout << " P" << g.pid << " |";
        cout << "\n0";
        for (auto& g : gantt)
            cout << setw(5) << g.end;
        cout << "\n";
        cout << "\nPID\tAT\tBT\tPRI\tCT\tTAT\tWT\tRT\n";
        double sum_tat = 0, sum_wt = 0, sum_rt = 0;
        for (const auto& p : plist) {
            cout << "P" << p.pid << "\t" << p.arrival << "\t" << p.burst << "\t"
                 << p.priority << "\t" << p.completion << "\t" << p.turnaround << "\t"
                 << p.waiting << "\t" << p.response << "\n";
            sum_tat += p.turnaround;
            sum_wt += p.waiting;
            sum_rt += p.response;
        }
        int n = plist.size();
        cout << fixed << setprecision(2);
        cout << "Avg TAT: " << sum_tat / n << ", Avg WT: " << sum_wt / n << ", Avg RT: " << sum_rt / n << "\n";
        gantt.clear();
    }
};

void generateRandomProcesses(vector<Process>& plist, int n, int seed = 42) {
    mt19937 gen(seed);
    uniform_int_distribution<> arrival_dist(0, 10), burst_dist(2, 20), pri_dist(1, 9);
    for (int i = 0; i < n; ++i) {
        int at = arrival_dist(gen);
        int bt = burst_dist(gen);
        int pr = pri_dist(gen);
        plist.emplace_back(i, at, bt, pr);
    }
}

void userInputProcesses(vector<Process>& plist, int n) {
    for (int i = 0; i < n; ++i) {
        int at, bt, pr;
        cout << "Enter arrival, burst, priority for P" << i << ": ";
        cin >> at >> bt >> pr;
        plist.emplace_back(i, at, bt, pr);
    }
}

int main() {
    int n, cs_overhead = 1, input_mode;
    cout << "Number of processes: "; cin >> n;
    cout << "Context switch overhead (default 1): "; cin >> cs_overhead;
    cout << "Enter 1 for random input, 2 for manual: "; cin >> input_mode;
    vector<Process> plist;
    if (input_mode == 1) generateRandomProcesses(plist, n);
    else userInputProcesses(plist, n);

    cout << "\nProcess List:\nPID\tAT\tBT\tPRI\n";
    for (const auto& p : plist)
        cout << "P" << p.pid << "\t" << p.arrival << "\t" << p.burst << "\t" << p.priority << "\n";

    Scheduler sched(plist, cs_overhead);
    sched.runFCFS();
    sched.runSJF_NP();
    sched.runSJF_P();
    sched.runPriority_NP();
    sched.runPriority_P();
    sched.runRR(4); // Example quantum
    sched.runMLQ();
    sched.runMLFQ();

    return 0;
}
