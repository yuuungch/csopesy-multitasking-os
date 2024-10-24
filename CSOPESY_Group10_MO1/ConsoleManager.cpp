#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <format>
#include <iomanip>
#include "ConsoleManager.h"
#include "AConsole.h"

using namespace std;

ConsoleManager::ConsoleManager(int cores) : coreCount(cores), availableCores(cores) {
    cpuCores = vector<bool>(cores, false);
    startScheduler();
}

/*
* This function adds a new console to the list of consoles
* 
* @param name - the name of the console
*/
void ConsoleManager::addConsole(const string& name, const int maxInstructions, bool fromScreenCommand = false) {
    lock_guard<mutex> lock(processMutex);

    // Check if the console name already exists in the map
    if (consoles.find(name) != consoles.end()) {
        cout << "Console \"" << name << "\" already exists." << endl;
        return;
    }

    // Create a unique process ID for the new console
    static int nextId = 1;
    int processId = nextId++;  // Generate the next process ID

    // Create a new console with the provided name and max instructions
    AConsole* newConsole = new AConsole(name, maxInstructions);

    // Set the process ID using the setProcessID function
    newConsole->setProcessID(processId);  // Ensure the process ID is properly set

    // Initialize additional details such as starting at instruction line 0
    newConsole->setInstructionLine(0);  // Start at instruction line 0

    // Add the new console to the map and the waiting queue
    waitingQueue.push(newConsole);
    consoles[name] = newConsole;

    // Check if the console was created using the screen -s command
    if (fromScreenCommand) {
        // Call displayConsole to show the relevant details
        displayConsole(name);
    }

    else
    {
        cout << "Console \"" << name << "\" created." << endl;
    }
}


/*
* This function displays the information of the specified console
* 
* @param name - the name of the console
*/
void ConsoleManager::displayConsole(const string& name) const {
    // Check if the console name exists in the map
    auto it = consoles.find(name);
    if (it != consoles.end()) {
        AConsole* console = it->second;  // Get the pointer to the AConsole object
        system("cls");  

        // Display console information
        cout << "Process: \"" << console->getName() << "\"" << endl;
        cout << "ID: " << console->getProcessID() << endl;  // Assuming you have a getID() function in AConsole
        // cout << "Created At: " << console->getTimestamp() << endl;
        cout << "Current Line of Instruction: " << console->getInstructionLine() << endl;
        cout << "Lines of Code: " << console->getInstructionTotal() << endl;
    }
    else {
        // If console does not exist, display a message
        cout << "Console \"" << name << "\" does not exist." << endl;
    }
}

void ConsoleManager::listConsoles() {
    lock_guard<mutex> lock(processMutex);

    if (!hasConsoles()) {
        cout << "No consoles to list.\n";
        return;
    }

    bool hasQueued = false;
    bool hasRunning = false;
    bool hasFinished = false;

    cout << "-----------------------------------------\n";

    cout << "Queued Processes:\n";
    for (const auto& consolePair : consoles) {
        AConsole* console = consolePair.second;  
        // get all queued consoles
        if (console->getStatus() == AConsole::WAITING) {
            hasQueued = true;
            cout << console->getName() + "\t" + console->getTimestamp() + "\tCore: " + to_string(console->getCoreID()) + "\t" + to_string(console->getInstructionLine()) + "/" + to_string(console->getInstructionTotal()) + "\n";
        }
    }
    cout << "\n";
    if (!hasQueued) cout << "No queued consoles.\n";

    cout << "Running Processes:\n";
    for (const auto& consolePair : consoles) {
        AConsole* console = consolePair.second;  
        // get all running consoles
        if (console->getStatus() == AConsole::RUNNING) {
            hasRunning = true;
            cout << console->getName() + "\t" + console->getTimestamp() + "\tCore: " + to_string(console->getCoreID()) + "\t" + to_string(console->getInstructionLine()) + "/" + to_string(console->getInstructionTotal()) + "\n";
        }
    }
    cout << "\n";
    if (!hasRunning) cout << "No running consoles.\n";

    cout << "Finished Processes:\n";
    for (const auto& consolePair : consoles) {
        AConsole* console = consolePair.second;  
        // get all finished consoles
        if (console->getStatus() == AConsole::TERMINATED) {
            hasFinished = true;
            cout << console->getName() + "\t" + console->getTimestamp() + "\tFinished\t" + to_string(console->getInstructionLine()) + "/" + to_string(console->getInstructionTotal()) + "\n";
        }
    }
    cout << "\n";
    if (!hasFinished) cout << "No terminated consoles.\n";
}


void ConsoleManager::startScheduler() {
	thread schedulerThread(&ConsoleManager::schedulerFCFS, this);
    schedulerThread.detach();
}

/*
* This function checks if the specified console exists
* 
* @param name - the name of the console
* @return true if the console exists, false otherwise
*/
bool ConsoleManager::consoleExists(const string& name) const {
	// Check if the console name exists in the list of consoles
    for (const auto& console : consoles) {
        // If console exists
        if (console.second->getName() == name) {
            return true;
        }
    }
    // If console does not exist
    return false;
}

/*
* This function checks if the list of consoles is empty or not
* 
* @return true if the list of consoles is not empty, false otherwise
*/
bool ConsoleManager::hasConsoles() const {
    return !consoles.empty();
}

/*
* This function initializes and runs the console program inside the specified console
* 
* @param name - the name of the console
*/
void ConsoleManager::loopConsole(const string& name) {
    // Find specified console in the list of consoles
    for (auto& console : consoles) {
        // If console exists
        if (console.second->getName() == name) {
            vector<string> buffer;
            string input;
            currentConsole = true;

            // Start console program
            do {
                buffer.clear();
                cout << "Console [" << console.second->getName() << "] Enter a command: ";

                // Read user input
                while (cin >> input) {
                    buffer.push_back(input);
                    if (cin.peek() == '\n') break;
                }

                if (buffer.empty()) continue;

                const string& command = buffer[0];

                if (command == "exit") {
                    return;  // Exit command
                }
                else if (command == "process-smi") {
                    // Check if the process has finished
                    if (console.second->getStatus() == AConsole::TERMINATED) {
                        cout << "Finished!" << endl;
                    }

                    else {
                        // Display current process information
                        cout << "Process: \"" << console.second->getName() << "\"" << endl;
                        cout << "ID: " << console.second->getProcessID() << endl;  // Assuming you have a getID() function in AConsole
                        // cout << "Created At: " << console.second->getTimestamp() << endl;
                        cout << "Current Line of Instruction: " << console.second->getInstructionLine() << endl;
                        cout << "Lines of Code: " << console.second->getInstructionTotal() << endl;
                    }
                }
                else if (command == "running") {
                    // Display running processes
                    cout << "Running Processes:\n";
                    bool hasRunning = false;
                    for (const auto& consolePair : consoles) {
                        AConsole* proc = consolePair.second;
                        if (proc->getStatus() == AConsole::RUNNING) {
                            hasRunning = true;
                            cout << proc->getName() + "\t" +
                                proc->getTimestamp() + "\t" +
                                "Core: " + to_string(proc->getCoreID()) + "\t" +
                                to_string(proc->getInstructionLine()) + "/" +
                                to_string(proc->getInstructionTotal()) + "\n";
                        }
                    }
                    if (!hasRunning) cout << "No running consoles.\n";
                    cout << "\n";
                }
                else if (command == "finished") {
                    // Display finished processes
                    cout << "Finished Processes:\n";
                    bool hasFinished = false;
                    for (const auto& consolePair : consoles) {
                        AConsole* proc = consolePair.second;
                        if (proc->getStatus() == AConsole::TERMINATED) {
                            hasFinished = true;
                            cout << proc->getName() + "\t" +
                                proc->getTimestamp() + "\t" +
                                "Finished\t" +
                                to_string(proc->getInstructionLine()) + "/" +
                                to_string(proc->getInstructionTotal()) + "\n";
                        }
                    }
                    if (!hasFinished) cout << "No finished consoles.\n";
                }
                else {
                    cout << "Command [" << command << "] not recognized. Try again." << "\n";
                }

            } while (currentConsole);
            return; // Exit console if not found
        }
    }
}

AConsole::Status ConsoleManager::getConsoleStatus(const string& name) const {
    // Check if the console name exists in the map
    auto it = consoles.find(name);
    if (it != consoles.end()) {
        return it->second->getStatus(); // Return the status of the console
    }
    // If console does not exist, return a default status (or handle it as you prefer)
    return AConsole::TERMINATED; // Assuming terminated is a safe fallback; you can change this
}



void ConsoleManager::schedulerFCFS() {
    while (true) {
        this_thread::sleep_for(chrono::seconds(1));

        lock_guard<mutex> lock(processMutex);

        for (int i = 0; i < cpuCores.size(); ++i) {
            if (!cpuCores[i] && !waitingQueue.empty()) {
                AConsole* nextProcess = waitingQueue.front();
                waitingQueue.pop();

                cpuCores[i] = true;

                runningProcesses[nextProcess->getName()] = thread([this, nextProcess, i]() {
                    nextProcess->runProcess(i);
                    lock_guard<mutex> lock(processMutex);
                    cpuCores[i] = false;
                   
                   
                    });

                runningProcesses[nextProcess->getName()].detach();
            }
        }

        
    }
}

