#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <pthread.h>
#include <thread>
#include <array>
#include <list>


using namespace std;

int nCpus = 0;
double _time = 0.0;
double *TT;
double *TT_Dup;
int timeslice = 0;
int Total_Context_Switches = 0;
string SchedulingType;
string inputFile , outputFile;
#include "./os_kernel.h"
#include "./process.h"

//  Functions Declarations
void GetCommand(string *, string *, string *, int *, int *);
vector<Process> read_process_from_file(string);
void Round_Robin(os_kernel *);
void FCFS(os_kernel *);
void Priority(os_kernel *);
void sleep(int, os_kernel *);
int main()
{
    srand(time(NULL));
    //  Variables Declarations

    /*-----------------------*/

    GetCommand(&inputFile, &outputFile, &SchedulingType, &nCpus, &timeslice);
    cout << inputFile << "\t" << nCpus << "\t" << SchedulingType << "\t" << timeslice << "\t" << outputFile << endl<<endl;

    /*-----------------------*/

    // cout << "Enter scheduling type : ";
    // string tempschedule;
    // cin >> tempschedule;
    // cout << tempschedule << endl;
    // SchedulingType = tempschedule;

    TT = new double[nCpus]{0.0};
    TT_Dup = new double[nCpus]{0.0};

    os_kernel kernel(SchedulingType, nCpus);

    // Creating the Threads
    std::thread *threads[nCpus];

    if (SchedulingType == "r")
    {

        for (int i = 0; i < nCpus; ++i)
        {
            threads[i] = new thread(&os_kernel::StartRR, &kernel, true, i);
        }
        Round_Robin(&kernel);
    }
    else if (SchedulingType == "f")
    {
        for (int i = 0; i < nCpus; ++i)
        {
            threads[i] = new thread(&os_kernel::Start, &kernel, true, i);
        }
        FCFS(&kernel);
    }
    else if (SchedulingType == "p")
    {
        for (int i = 0; i < nCpus; ++i)
        {
            threads[i] = new thread(&os_kernel::Start, &kernel, true, i);
        }
        Priority(&kernel);
    }

    cout << endl
         << "Context Switches : " << Total_Context_Switches << endl;
    for (int i = 0; i < nCpus; i++)
    {
        threads[i]->join();
        threads[i] = NULL;
        delete threads[i];
    }
    return 0;
}
void GetCommand(string *inputFile, string *outputFile, string *SchedulingType, int *nCpu, int *timeslice)
{
    string Command;

    cout << "Please enter the Command : ";
    getline(cin, Command);

    if (Command.find(" "))
    {
        Command = Command.substr(Command.find(" ") + 1, Command.size() - 1);

        if (Command.find(" ") != -1)
        {
            *inputFile = Command.substr(0, Command.find(" ")); // Reading Input File
            Command = Command.substr(Command.find(" ") + 1, Command.size() - 1);

            if (Command.find(" ") != -1)
            {
                string cpus = Command.substr(0, Command.find(" ")); // Reading No of Cpu's
                *nCpu = stoi(cpus);
                Command = Command.substr(Command.find(" ") + 1, Command.size() - 1);

                if (Command.find(" ") != -1)
                {
                    *SchedulingType = Command.substr(0, Command.find(" ")); // Reading Scheduling type
                    Command = Command.substr(Command.find(" ") + 1, Command.size() - 1);

                    if (*SchedulingType == "r")
                    {
                        string timslc = Command.substr(0, Command.find(" ")); // Reading Time Slice
                        Command = Command.substr(Command.find(" ") + 1, Command.size() - 1);
                        *timeslice = stoi(timslc);

                        *outputFile = Command;
                    }
                    else
                    {
                        *timeslice = 0;
                        *outputFile = Command;
                    }
                }
                else
                    cout << "Invalid Command\n";
            }
            else
                cout << "Invalid Command\n";
        }
        else
            cout << "Invalid Command\n";
    }
    else
        cout << "Invalid Command\n";
}
vector<Process> read_process_from_file(string filename)
{
    vector<Process> processes_list;
    int Process_ID = 1;
    ifstream in(filename, ios::in);
    string line;
    int line_no = 0;
    while (getline(in, line))
    {
        if (line_no != 0)
        {
            string word = "";
            Process temp;
            int line_e = 0; // line element
            for (int i = 0; i < line.size(); i++)
            {

                if ((line[i] == '\t' || line[i] == ' ' || i == line.size() - 1) && (word != "" || i == line.size() - 1))
                {

                    if (line_e == 0)
                    {
                        temp.process_name = word;

                        line_e++;
                    }
                    else if (line_e == 1)
                    {
                        temp.priority = stoi(word);
                        line_e++;
                    }
                    else if (line_e == 2)
                    {
                        temp.arrival_time = stod(word);
                        line_e++;
                    }
                    else if (line_e == 3)
                    {
                        temp.process_type = word[0];
                        line_e++;
                    }
                    else if (line_e == 4)
                    {
                        temp.cpu_time = stod(word);
                        line_e++;
                    }
                    else if (line_e == 5)
                    {
                        word += line[i];
                        temp.io_time = stoi(word);
                        line_e++;
                    }

                    word = "";
                }
                else
                {
                    if (line[i] != '\t' || line[i] != ' ')
                        word += line[i];
                }
            }
            temp.cpu_time*=10;
            temp.Remaining_time = temp.cpu_time;
            temp.Process_ID = Process_ID++;
            processes_list.push_back(temp);
        }

        line_no++;
    }
    
    if (inputFile == "Processes2.txt")
    {
        for (int i = 0; i < processes_list.size(); i++)
        {
            if (processes_list[i].process_type == 'C')
            {
                processes_list[i].io_time = -1;
            }
            else
            {
                processes_list[i].io_time = rand() % 3 + 1;
            }
            processes_list[i].cpu_time = (rand() % 15 + 1)*10;

            processes_list[i].Remaining_time = processes_list[i].cpu_time;

        }
    }
    
    return processes_list;
}
void Round_Robin(os_kernel *kernel)
{
    cout << "\nRound Robin\n";
    while (kernel->Total_Processes)
    {
        kernel->wakeupRR();

        cout << "Time Elapsed : " << static_cast<int>(_time)/10<<"."<< static_cast<int>(_time)%10<< endl;
        for (int i = 0; i < nCpus; i++)
        {
            kernel->Print(i);
        }

        sleep(1, kernel);

        for (int i = 0; i < nCpus; i++)
        {
            kernel->Round_Robin(i);
        }
        cout << "ready queue size:" << kernel->scheduler.rrsize() << endl;
        cout << "running queue size:" << kernel->scheduler.running_size() << endl;
        cout << "waiting queue size:" << kernel->scheduler.waiting_size() << endl;

        cout << endl;
    }
}
void FCFS(os_kernel *kernel)
{
    cout << "\nPriority Scheduling\n";
    while (kernel->Total_Processes)
    {
        kernel->wakeup();

        cout << "Time Elapsed : " << static_cast<int>(_time)/10<<"."<< static_cast<int>(_time)%10<< endl;

        for (int i = 0; i < nCpus; i++)
        {
            kernel->Print(i);
        }
        sleep(1, kernel);

        cout << "ready queue size:" << kernel->scheduler.readysize() << endl;
        cout << "running queue size:" << kernel->scheduler.running_size() << endl;
        cout << "waiting queue size:" << kernel->scheduler.waiting_size() << endl;

        cout << endl;
    }
}
void Priority(os_kernel *kernel)
{
    cout << "\nPriority Scheduling\n";

    while (kernel->Total_Processes)
    {
        kernel->wakeup();

        cout << "Time Elapsed : " << static_cast<int>(_time)/10<<"."<< static_cast<int>(_time)%10<< endl;
        for (int i = 0; i < nCpus; i++)
        {
            kernel->Print(i);
        }

        sleep(1, kernel);

        for (int i = 0; i < nCpus; i++)
        {
            kernel->Priority_Preempt(i);
        }

        cout << "ready queue size:" << kernel->scheduler.readysize() << endl;
        cout << "running queue size:" << kernel->scheduler.running_size() << endl;
        cout << "waiting queue size:" << kernel->scheduler.waiting_size() << endl;
        cout << endl;
    }
}
void sleep(int sleeping_time, os_kernel *kernel)
{
    kernel->UpdateTime();
    for (int i = 0; i < nCpus; i++)
    {
        TT[i] += 1.0;
    }
    _time += 1.0;

    std::this_thread::sleep_for(100 * 1ms);
}
