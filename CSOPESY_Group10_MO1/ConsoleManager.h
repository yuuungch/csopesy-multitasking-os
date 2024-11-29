#pragma once
#include <map>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <vector>
#include "AConsole.h"
#include "MemoryManager.h"

using namespace std;

class ConsoleManager {
private:
    map<string, AConsole*> consoles;
    bool reportingMode = false;
    bool currentConsole = false;
    int coreCount;
    int availableCores;
    vector<bool> cpuCores;
    queue<AConsole*> waitingQueue;
    queue<AConsole*> memoryQueue;
    map<string, thread> runningProcesses;
    mutex processMutex;

public:
    void initialize();
    bool isPowerOfTwo(const uint64_t n);
    void addConsole(const string& name, bool fromScreenCommand);
    void readConfig(const string& filename);
    void testConfig();
    void displayConsole(const string& name) const;
    void displayCPUInfo();
    void listConsoles();
    void reportUtil();
    void processSMI();
    void startScheduler();
    bool consoleExists(const string& name) const;
    bool hasConsoles() const;
    AConsole::Status getConsoleStatus(const string& name) const;
    void loopConsole(const string& name);
    void schedulerTest(bool set_scheduler);
    void schedulerFCFS();
    void schedulerRR();
};
