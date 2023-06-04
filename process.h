#pragma once

#include <iostream>
#include <climits>
using namespace std;
#include <stack>
#include <list>


class pcb
{
    public:
    stack<int> STACK;
    // registers
    int aex;
    int bex;
    int cex;
    int dex;

    int sp;
    int bp;
    int si;
    int di;
    int pc;

    list<int> opened_files;
    int processid;
    int process_state;
    
    int memory_start_address;
    int memory_end_address;
};
class Process
{
public:
	pcb PCB;
	int Process_ID;
	string process_name;
	int priority;
	double arrival_time;
	char process_type;
	double cpu_time;
	int io_time;
	int Remaining_time;
	int Remaining_time_Slice;
	int waiting_time;
	int context_switches;
	int Remaining_IO_time;
	Process()
	{
		Process_ID = 0;
		process_name = "";
		priority = 0;
		arrival_time = 0;
		process_type = 'N';
		cpu_time = 0;
		io_time=0;
		Remaining_time = 0;
		Remaining_time_Slice = 0;
		waiting_time = 0;
		context_switches = 0;
		Remaining_IO_time = 20;
	}
	void init(string Name,int _ID)
	{
		Process_ID = _ID;
		process_name = Name;
		priority = 0;
		arrival_time = 0;
		process_type = 'N';
		cpu_time = 0;
		io_time=0;
		Remaining_time = 0;
		context_switches = 0;
		Remaining_IO_time = 20;
	}
	void Print()
	{
		cout<<process_name<<" "<<priority<<" "<<arrival_time<<" "<<process_type<<" "<<cpu_time<<" "<<io_time<<endl;
	}
};