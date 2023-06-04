#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <pthread.h>
#include <climits>
#include <list>

using namespace std;

#include "process.h"
#include "Scheduler.h"

vector<Process> read_process_from_file(string);
void sleep(int);

class os_kernel
{
private:
    vector<Process> processes0;
    vector<Process> processes1;

    Process Idle_Process;
    

    pthread_mutex_t Lock1;
    pthread_mutex_t Lock2;
    pthread_mutex_t Lock3;
    pthread_mutex_t Lock4;
    pthread_mutex_t Lock5;
    pthread_mutex_t Lock6;

public:
	Scheduler scheduler;
    int Total_Processes;
    os_kernel(string Scheduling_Type, int nCpu)
    {
        pthread_mutex_init(&Lock1, NULL);
        pthread_mutex_init(&Lock2, NULL);
        pthread_mutex_init(&Lock3, NULL);
        pthread_mutex_init(&Lock4, NULL);
        pthread_mutex_init(&Lock5, NULL);
        pthread_mutex_init(&Lock6, NULL);

        vector<Process> processes0 = read_process_from_file(inputFile);
        vector<Process> processes1 = read_process_from_file("Processes2.txt");

        // Initializing the Scheduler
        Scheduler temp_schedueler(Scheduling_Type, processes0, nCpu);
        scheduler = temp_schedueler;

        // Initializing the Idle Process
        Idle_Process.init("Idle", INT_MAX);

        temp_schedueler.print();

        Total_Processes = processes0.size();
    }

    void Start(bool CPU_Status, int index)
    {
        while (Total_Processes)
        {
            Schedule(CPU_Status, index);
            terminate(CPU_Status, index);

            // IO(CPU_Status, index);
            // IO_Completion

            while (TT[index] <= TT_Dup[index] && Total_Processes != 0)
            {
                Schedule(CPU_Status, index);
            }
            TT_Dup[index] = TT[index];
        }
    }
    void wakeup() // Transferring Processes from New Queue to Ready Queue
    {
        // Wakeup (New --> Ready)
        if (!scheduler.newqueue.empty())
        {
            while (scheduler.newqueue.top().arrival_time == _time)
            {
                Process temp = scheduler.newqueue.top();
                scheduler.newqueue.pop();
                scheduler.readyQueue.push(temp);

                if (scheduler.newqueue.empty())
                {
                    return;
                }
            }
            std::this_thread::sleep_for(50 * 1ms);
        }

        // Wakeup for (Waiting --> Ready)
        // scheduler.waiting_wakeup();

        return;
    }
    void Schedule(bool &CPU_Status, int index) // Scheduling the Processes on the CPU(Ready Queue ---> Running Queue)
    {
        if (CPU_Status)
        {
            pthread_mutex_lock(&Lock2);
            if (!scheduler.readyQueue.empty())
            {
                // if(scheduler.runningQueue[index].front().Process_ID == INT_MAX)
                while (!scheduler.runningQueue[index].empty())
                {
                    scheduler.runningQueue[index].pop();
                }

                Process temp = scheduler.readyQueue.top();
                scheduler.readyQueue.pop();

                scheduler.runningQueue[index].push(temp);
                CPU_Status = false;
            }
            else
            {
                if (scheduler.runningQueue[index].empty())
                    scheduler.runningQueue[index].push(Idle_Process);
            }
            pthread_mutex_unlock(&Lock2);
        }
    }
    void terminate(bool &CPU_Status, int index) // Terminating by removing the Processes from the Ready queue (Running Queue---> Terminated Queue))
    {
        if (!scheduler.runningQueue[index].empty())
        {
            if (scheduler.runningQueue[index].front().Process_ID != INT_MAX)
            {
                if (scheduler.runningQueue[index].front().Remaining_time == 0)
                {
                    Process temp = scheduler.runningQueue[index].front();
                    scheduler.runningQueue[index].pop();

                    if(temp.process_type == 'I')
                    scheduler.waitingQueue.push(temp);
                    else
                    scheduler.terminatedQueue.push(temp);
                    

                    pthread_mutex_lock(&Lock1);
                    Total_Processes--;
                    pthread_mutex_unlock(&Lock1);
                    CPU_Status = true;
                }
            }
        }
    }
    void Priority_Preempt(int qIndex)
    {
        if (!scheduler.readyQueue.empty())
        {
            pthread_mutex_lock(&Lock4);
            if (scheduler.readyQueue.top().priority > scheduler.runningQueue[qIndex].front().priority)
            {
                Process temp = scheduler.runningQueue[qIndex].front();
                scheduler.runningQueue[qIndex].pop();

                temp.context_switches++; // Incrementing the Context Switches for the Process..
                Total_Context_Switches++;

                Process temp1 = scheduler.readyQueue.top();
                scheduler.readyQueue.pop();
                scheduler.runningQueue[qIndex].push(temp1);

                scheduler.readyQueue.push(temp);
            }
            pthread_mutex_unlock(&Lock4);
        }
    }
    void IO(bool &CPU_Status, int qIndex)
    {
        if (!CPU_Status)
        {
            if (scheduler.runningQueue[qIndex].front().io_time > 0)
            {
                if (rand() % 2)
                {
                    pthread_mutex_lock(&Lock6);
                    // scheduler.runningQueue[qIndex].front().Remaining_IO_time = 0;
                    if (SchedulingType == "f")
                    {
                        Process temp = scheduler.runningQueue[qIndex].front();
                        scheduler.runningQueue[qIndex].pop();

                        temp.io_time--;

                        scheduler.waitingQueue.push(temp);
                        scheduler.runningQueue[qIndex].push(Idle_Process);
                    }
                    else
                    {
                        Process temp = scheduler.runningQueue[qIndex].front();
                        scheduler.runningQueue[qIndex].pop();

                        temp.io_time--;

                        scheduler.waitingQueue.push(temp);
                        CPU_Status = true;
                        Schedule(CPU_Status, qIndex);
                    }
                    pthread_mutex_lock(&Lock6);
                }
            }
        }
    }
    // *--------------------------------------    ROUND ROBIN    --------------------------------------*
    void Round_Robin(int qIndex)
    {
        if (!scheduler.readyQueueRR.empty())
        {
            pthread_mutex_lock(&Lock5);
            if (scheduler.runningQueue[qIndex].front().Remaining_time_Slice == timeslice)
            {
                Process temp = scheduler.runningQueue[qIndex].front();
                scheduler.runningQueue[qIndex].pop();
                temp.Remaining_time_Slice = 0;

                temp.context_switches++; // Incrementing the Context Switches for the Process..
                Total_Context_Switches++;

                Process temp1 = scheduler.readyQueueRR.front();
                scheduler.readyQueueRR.pop();
                scheduler.runningQueue[qIndex].push(temp1);

                scheduler.readyQueueRR.push(temp);
            }
            pthread_mutex_unlock(&Lock5);
        }
    }
    void StartRR(bool CPU_Status, int index)
    {
        while (Total_Processes)
        {
            ScheduleRR(CPU_Status, index);
            terminateRR(CPU_Status, index);
            // IO(CPU_Status, index);

            while (TT[index] <= TT_Dup[index] && Total_Processes != 0)
            {
                ScheduleRR(CPU_Status, index);
            }
            TT_Dup[index] = TT[index];
        }
    }
    void wakeupRR() // Transferring Processes from New Queue to Ready Queue
    {
        if (!scheduler.newqueue.empty())
        {
            while (scheduler.newqueue.top().arrival_time == _time)
            {
                Process temp = scheduler.newqueue.top();
                scheduler.newqueue.pop();
                scheduler.readyQueueRR.push(temp);

                if (scheduler.newqueue.empty())
                {
                    return;
                }
            }
            std::this_thread::sleep_for(50 * 1ms);
        }
        // scheduler.waiting_wakeup();
        return;
    }
    void ScheduleRR(bool &CPU_Status, int index) // Scheduling the Processes on the CPU(Running Queue)
    {
        if (CPU_Status)
        {
            pthread_mutex_lock(&Lock2);
            if (!scheduler.readyQueueRR.empty())
            {
                // if(scheduler.runningQueue[index].front().Process_ID == INT_MAX)
                while (!scheduler.runningQueue[index].empty())
                {
                    scheduler.runningQueue[index].pop();
                }

                Process temp = scheduler.readyQueueRR.front();
                scheduler.readyQueueRR.pop();

                scheduler.runningQueue[index].push(temp);
                CPU_Status = false;
            }
            else
            {
                if (scheduler.runningQueue[index].empty())
                    scheduler.runningQueue[index].push(Idle_Process);
            }
            pthread_mutex_unlock(&Lock2);
        }
    }
    void terminateRR(bool &CPU_Status, int index) // Terminating by removing the Processes from the Ready queue
    {
        if (!scheduler.runningQueue[index].empty())
        {
            if (scheduler.runningQueue[index].front().Process_ID != INT_MAX)
            {
                if (scheduler.runningQueue[index].front().Remaining_time == 0)
                {
                    Process temp = scheduler.runningQueue[index].front();
                    scheduler.runningQueue[index].pop();

                    scheduler.terminatedQueue.push(temp);

                    pthread_mutex_lock(&Lock1);
                    Total_Processes--;
                    pthread_mutex_unlock(&Lock1);
                    CPU_Status = true;
                }
            }
        }
    }
    // *--------------------------------------    ROUND ROBIN    --------------------------------------*
    void UpdateTime()
    {
        scheduler.UpdateTime();
    }
    void Print(int index)
    {
        if (scheduler.runningQueue[index].front().Process_ID == INT_MAX)
        {
            cout << "Cpu # " << index << "\t" << scheduler.runningQueue[index].front().process_name << endl;
        }
        else
            cout << "Cpu # " << index << "\t" << scheduler.runningQueue[index].front().process_name << " : remaining time = " << scheduler.runningQueue[index].front().Remaining_time/10<<'.'<<scheduler.runningQueue[index].front().Remaining_time%10 << endl;
    }
    void Print_Processes()
    {
        for (int i = 0; i < processes0.size(); i++)
        {
            processes0[i].Print();
        }
        cout << "--------------------------\n";
        for (int i = 0; i < processes0.size(); i++)
        {
            processes0[i].Print();
        }
    }
};
