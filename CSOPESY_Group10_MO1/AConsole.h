#pragma once
#include <string>
#include <ctime>
#include <vector>
#include <iostream>
#include "MemoryManager.h"

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
    MemoryManager* memoryManager;

public:
    enum Status { RUNNING, WAITING, TERMINATED };
    Status status;

    // Updated constructor to accept a MemoryManager pointer
    AConsole(const string& name, int instructionTotal, MemoryManager* memManager);

    // Function to run the process
    void runProcess(int coreID, int quantum_cycles, int delaysPerExec);

    // Getter functions
    string getName() const;
    string getTimestamp() const;
    int getInstructionLine() const;
    void setInstructionLine(int instructionLine);
    int getInstructionTotal() const;
    void setInstructionTotal(int instructionTotal);
    Status getStatus() const;
    int getCoreID() const;
    int getProcessID() const;
    void setProcessID(int id);
    bool getIsActive() const;
    void setIsActive(bool active);

private:
    static string getCurrentTime();
};
