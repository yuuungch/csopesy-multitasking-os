#pragma once
#include <string>
#include <ctime>
#include <vector>
#include <iostream>
using namespace std;

class AConsole {
    private:
        int processID;
        string name;
        string timestamp;
        int instructionLine;
        int instructionTotal;
        int coreID;
        bool isActive;
        

    public:
        enum Status { RUNNING, WAITING, TERMINATED };
        Status status;

        AConsole(const string& name, int instructionTotal);



        void runProcess(int coreID);

        string getName() const;
        string getTimestamp() const;
		int getInstructionLine() const;
		void setInstructionLine(int instructionLine);
        int getInstructionTotal() const;
        void setInstructionTotal(int instructionTotal);
        Status getStatus();
        int getCoreID();
        int getProcessID() const;      
        void setProcessID(int id);     

    private:
        static string getCurrentTime();
};
