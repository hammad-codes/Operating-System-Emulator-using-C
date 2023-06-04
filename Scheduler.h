#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <climits>
#include <math.h>

#include "process.h"
using namespace std;

string Scheduling_type;
struct ComparatorA
{
    bool operator()(Process P1, Process P2)
    {
        if (Scheduling_type == "f")
            return P1.arrival_time > P2.arrival_time;
        else
            return P1.priority < P2.priority;
    }
};
struct ComparatorB
{
    bool operator()(Process P1, Process P2)
    {
        return P1.arrival_time > P2.arrival_time;
    }
};
class Scheduler
{
public:
    priority_queue<Process, vector<Process>, ComparatorB> newqueue;
    priority_queue<Process, vector<Process>, ComparatorA> readyQueue;

    queue<Process> readyQueueRR;

    queue<Process> *runningQueue;
    queue<Process> waitingQueue;
    queue<Process> terminatedQueue;
    pthread_mutex_t Lock1;

    int NCpus;

    int newqueuesize()
    {
        return newqueue.size();
    }
    int running_size()
    {
        return runningQueue->size();
    }
    int waiting_size()
    {
        return waitingQueue.size();
    }
    int tersize()
    {
        return terminatedQueue.size();
    }
    int rrsize()
    {
        return readyQueueRR.size();
    }
    int readysize()
    {
        return readyQueue.size();
    }
    
    Scheduler() {}
    Scheduler(string Scheduling_Type, vector<Process> processes0, int nCPus) // Filling
    {
        pthread_mutex_init(&Lock1, NULL);

        Scheduling_type = Scheduling_Type;
        NCpus = nCPus;
        runningQueue = new queue<Process>[nCPus];
        for (int i = 0; i < processes0.size(); i++)
        {
            newqueue.push(processes0[i]);
        }
    }
    void print()
    {
        while (!newqueue.empty())
        {
            cout << newqueue.top().process_name << endl;
            newqueue.pop();
        }
    }
    void UpdateTime()
    {
        for (int i = 0; i < NCpus; i++)
        {
            if (!runningQueue[i].empty())
            {
                runningQueue[i].front().Remaining_time--;
                runningQueue[i].front().Remaining_time_Slice++;
            }
        }

        queue<Process> tempqueue;
        while(!waitingQueue.empty())
        {
            Process temp = waitingQueue.front();
            waitingQueue.pop();

            temp.Remaining_IO_time--;
        
            if(temp.Remaining_IO_time == 0)
            {
                temp.io_time--;
                if(temp.io_time !=0)
                {
                    temp.Remaining_IO_time = 20;
                    tempqueue.push(temp);
                }
                else
                {
                    terminatedQueue.push(temp);
                }
            }
            else
            {
                tempqueue.push(temp);
            }

        }
        while(!tempqueue.empty())
        {
            Process temp = tempqueue.front();
            tempqueue.pop();

            waitingQueue.push(temp);
        }

    }
};