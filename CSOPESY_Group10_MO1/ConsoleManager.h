#pragma once
#include <map>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <vector>
#include <iomanip>
#include "AConsole.h"
#include "MemoryManager.h"

using namespace std;

class ConsoleManager {
private:
    map<string, AConsole*> consoles;       // Map of console name to console objects
    bool reportingMode = false;           // Whether reporting mode is enabled
    bool currentConsole = false;          // Tracks the currently active console
    int coreCount;                        // Total number of CPU cores
    int availableCores;                   // Number of available cores
    vector<bool> cpuCores;                // Tracks CPU core availability
    queue<AConsole*> waitingQueue;        // Queue for processes waiting for memory
    queue<AConsole*> memoryQueue;         // Queue for processes ready for memory allocation
    map<string, thread> runningProcesses; // Map of process names to threads
    mutex processMutex;                   // Mutex for thread safety

public:
    // Initialization and configuration
    void initialize();
    void readConfig(const string& filename);
    void testConfig();

    // Process management
    void addConsole(const string& name, bool fromScreenCommand);
    void loopConsole(const string& name);
    bool consoleExists(const string& name) const;
    bool hasConsoles() const;

    // Console display utilities
    void displayConsole(const string& name) const;
    void displayCPUInfo();
    void listConsoles();
    void reportUtil();
    void processSMI();
    void vmStat() const;  // Display detailed virtual memory statistics

    // Scheduler and process state management
    void startScheduler();
    void schedulerTest(bool set_scheduler);
    void schedulerFCFS();
    void schedulerRR();

    // Process state queries
    AConsole::Status getConsoleStatus(const string& name) const;

    // Helper functions
    bool isPowerOfTwo(const uint64_t n);
};
