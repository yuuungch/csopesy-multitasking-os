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
	ConsoleManager(); // default constructor

    void addConsole(const string& name, bool fromScreenCommand);
	void readConfig(const string& filename);
    void testConfig();
    void displayConsole(const string& name) const;
    void listConsoles();
    void startScheduler();
    bool consoleExists(const string& name) const;
    bool hasConsoles() const;

    AConsole::Status getConsoleStatus(const string& name) const;

    void loopConsole(const string& name);

    void schedulerTest();
    void schedulerStop();

    void schedulerFCFS();
    void schedulerRR();
};
