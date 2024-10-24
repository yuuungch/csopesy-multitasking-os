#pragma once
#include <map>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <vector>
#include "AConsole.h"

using namespace std;

class ConsoleManager {
private:
    std::map<std::string, AConsole*> consoles;

    bool currentConsole = false;
    int coreCount;
    int availableCores;
    vector<bool> cpuCores;
    queue<AConsole*> waitingQueue;
    map<string, thread> runningProcesses;
    mutex processMutex;


public:
	ConsoleManager(int cores = 4); // default constructor

    void addConsole(const string& name, const int maxInstructions, bool fromScreenCommand);
    void displayConsole(const string& name) const;
    void listConsoles();
    void startScheduler();
    bool consoleExists(const string& name) const;
    bool hasConsoles() const;
    AConsole::Status getConsoleStatus(const string& name) const;

    void loopConsole(const string& name);

    void schedulerFCFS();

    // void Week6_FCFSScheduler();
};
