#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <format>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include "ConsoleManager.h"
#include "AConsole.h"
#include "MemoryManager.h"

using namespace std;

int num_cpu;
string scheduler;
int quantum_cycles;
int batch_process_freq;
int min_ins;
int max_ins;
int delays_per_exec;
const uint64_t MAX_VALUE = 4294967296;

bool scheduler_test_run = false;

MemoryManager* memoryManager = nullptr;
int max_overall_mem;
int mem_per_frame;
int min_mem_per_proc;
int max_mem_per_proc;

void ConsoleManager::initialize() {

	readConfig("config.txt");

    memoryManager = new MemoryManager(max_overall_mem, mem_per_frame, min_mem_per_proc, max_mem_per_proc);

    coreCount = num_cpu;
    availableCores = num_cpu;

    cpuCores = vector<bool>(num_cpu, false);
}

bool ConsoleManager::isPowerOfTwo(uint64_t n) {
    return (n >= 2) && ((n & (n - 1)) == 0);
}

/*
* This function reads the config.txt file and initializes parameters
*/
void ConsoleManager::readConfig(const string& filename) {
    ifstream configFile(filename);
    if (!configFile.is_open()) {
        cerr << "Error: Could not open config file.\n";
        return;
    }

    string line;
    while (getline(configFile, line)) {
        istringstream iss(line);
        string key;
        if (!(iss >> key)) continue;

        if (key == "num-cpu") {
            iss >> num_cpu;
            if (num_cpu < 1 || num_cpu > 128) {
                cerr << "Error: Invalid num-cpu value: " << num_cpu << ". Must be in range [1, 128].\n";
                return;
            }
        }
        else if (key == "scheduler") {
            string value;
            iss >> quoted(value);  // Use std::quoted to handle quotes
            scheduler = value;  // Assign the stripped value
            if (scheduler != "fcfs" && scheduler != "rr") {
                cerr << "Error: Invalid scheduler value: '" << scheduler << "'. Must be 'fcfs' or 'rr'.\n";
                return;
            }
        }
        else if (key == "quantum-cycles") {
            iss >> quantum_cycles;
            if (scheduler == "rr" && (quantum_cycles < 1 || quantum_cycles > MAX_VALUE)) {
                cerr << "Error: Invalid quantum-cycles value: " << quantum_cycles << ". Must be in range [1, " << MAX_VALUE << "].\n";
                return;
            }
        }
        else if (key == "batch-process-freq") {
            iss >> batch_process_freq;
            if (batch_process_freq < 1 || batch_process_freq > MAX_VALUE) {
                cerr << "Error: Invalid batch-process-freq value: " << batch_process_freq << ". Must be in range [1, " << MAX_VALUE << "].\n";
                return;
            }
        }
        else if (key == "min-ins") {
            iss >> min_ins;
            if (min_ins < 1 || min_ins > MAX_VALUE) {
                cerr << "Error: Invalid min-ins value: " << min_ins << ". Must be in range [1, " << MAX_VALUE << "].\n";
                return;
            }
        }
        else if (key == "max-ins") {
            iss >> max_ins;
            if (max_ins < 1 || max_ins > MAX_VALUE) {
                cerr << "Error: Invalid max-ins value: " << max_ins << ". Must be in range [1, " << MAX_VALUE << "].\n";
                return;
            }
        }
        else if (key == "delays-per-exec") {
            iss >> delays_per_exec;
            if (delays_per_exec < 0 || delays_per_exec > MAX_VALUE) {
                cerr << "Error: Invalid delays-per-exec value: " << delays_per_exec << ". Must be in range [0, " << MAX_VALUE << "].\n";
                return;
            }
        }
        else if (key == "max-overall-mem") {
            iss >> max_overall_mem;
            if (max_overall_mem < 2 || max_overall_mem > MAX_VALUE || !isPowerOfTwo(max_overall_mem)) {
                cerr << "Error: Invalid max-overall-mem value: " << max_overall_mem << ". Must be a power of 2 in range [2, " << MAX_VALUE << "].\n";
                return;
            }
        }
        else if (key == "mem-per-frame") {
            iss >> mem_per_frame;
            if (mem_per_frame < 2 || mem_per_frame > MAX_VALUE || !isPowerOfTwo(mem_per_frame)) {
                cerr << "Error: Invalid mem-per-frame value: " << mem_per_frame << ". Must be a power of 2 in range [2, " << MAX_VALUE << "].\n";
                return;
            }
        }
        else if (key == "min-mem-per-proc") {
            iss >> min_mem_per_proc;
            if (min_mem_per_proc < 2 || min_mem_per_proc > MAX_VALUE || !isPowerOfTwo(min_mem_per_proc)) {
                cerr << "Error: Invalid mem-per-proc value: " << min_mem_per_proc << ". Must be a power of 2 in range [2, " << MAX_VALUE << "].\n";
                return;
            }
        }

        else if (key == "max-mem-per-proc") {
            iss >> max_mem_per_proc;
            if (max_mem_per_proc < 2 || max_mem_per_proc > MAX_VALUE || !isPowerOfTwo(max_mem_per_proc)) {
                cerr << "Error: Invalid mem-per-proc value: " << max_mem_per_proc << ". Must be a power of 2 in range [2, " << MAX_VALUE << "].\n";
                return;
            }
        }
        else {
            cerr << "Error: Unknown parameter in config file: " << key << endl;
            return;
        }
    }
    //cout << "Configuration successfully loaded.\n";
    configFile.close();
    return;
}

void ConsoleManager::testConfig() {
    // test if the config file was read successfully, print all values
    cout << "num-cpu: " << num_cpu << endl;
    cout << "scheduler: " << scheduler << endl;
    cout << "quantum-cycles: " << quantum_cycles << endl;
    cout << "batch-process-freq: " << batch_process_freq << endl;
    cout << "min-ins: " << min_ins << endl;
    cout << "max-ins: " << max_ins << endl;
    cout << "delays-per-exec: " << delays_per_exec << endl;
    cout << "max-overall-mem: " << max_overall_mem << endl;
    cout << "mem-per-frame: " << mem_per_frame << endl;
    cout << "min-mem-per-proc: " << min_mem_per_proc << endl;
    cout << "max-mem-per-proc: " << max_mem_per_proc << endl;
}

/*
* This function adds a new console to the list of consoles
* 
* @param name - the name of the console
*/

void ConsoleManager::addConsole(const string& name, bool fromScreenCommand = false) {
    lock_guard<mutex> lock(processMutex);

    // Check if the console name already exists in the map
    if (consoles.find(name) != consoles.end()) {
        cout << "Console \"" << name << "\" already exists." << endl;
        return;
    }

    // Create a unique process ID for the new console
    static int nextId = 1;
    int processId = nextId++;  // Generate the next process ID

    // Generate a random number of instructions between min_ins and max_ins
    random_device rd;
    knuth_b knuth_gen(rd());
    uniform_int_distribution<> dist(min_ins, max_ins);
    int maxInstructions = dist(knuth_gen);

    // Create a new console with the provided name and max instructions
    AConsole* newConsole = new AConsole(name, maxInstructions, memoryManager);

    // Set the process ID and other properties
    newConsole->setProcessID(processId);  // Ensure the process ID is properly set
    newConsole->setInstructionLine(0);  // Start at instruction line 0

    size_t processesInMemory = memoryManager->calculateNumberofProcesses();
    size_t maxProcessesInMemory = memoryManager->getMaxOverallMem() / memoryManager->getMinMemPerProc();

    // Check the allocation type
    if (memoryManager->getAllocationType() == "flat") {
        // Flat memory allocation logic
        if (waitingQueue.empty() && processesInMemory < maxProcessesInMemory) {
            if (memoryManager->allocateMemory(newConsole->getProcessID())) {
                waitingQueue.push(newConsole);
                memoryQueue.push(newConsole);
                // cout << "Console " << newConsole->getName() << " allocated memory and added to memoryQueue." << endl;
            }
        }
        else {
            waitingQueue.push(newConsole);
            // cout << "Added to waitingQueue: " << newConsole->getName() << endl;
        }
    }
    else if (memoryManager->getAllocationType() == "paging") {
        // Paging memory allocation logic
        size_t requiredMemory = memoryManager->getMinMemPerProc() + (rand() % (memoryManager->getMaxMemPerProc() - memoryManager->getMinMemPerProc() + 1));
        size_t requiredFrames = (requiredMemory + memoryManager->getMemPerFrame() - 1) / memoryManager->getMemPerFrame();

        if (waitingQueue.empty() && memoryManager->findFreeFrames(requiredFrames) != -1) {
            if (memoryManager->allocateMemory(newConsole->getProcessID())) {
                waitingQueue.push(newConsole);
                memoryQueue.push(newConsole);
                // cout << "Console " << newConsole->getName() << " allocated memory and added to memoryQueue." << endl;
            }
        }
        else {
            waitingQueue.push(newConsole);
            // cout << "Added to waitingQueue: " << newConsole->getName() << endl;
        }
    }

    // Add the new console to the consoles map
    consoles[name] = newConsole;

    // Check if the console was created using the screen -s command
    if (fromScreenCommand) {
        // Call displayConsole to show the relevant details
        displayConsole(name);
    }

    startScheduler();
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
        cout << "ID: " << console->getProcessID() << endl; 
        // cout << "Created At: " << console->getTimestamp() << endl;
        cout << "Current Line of Instruction: " << console->getInstructionLine() << endl;
        cout << "Lines of Code: " << console->getInstructionTotal() << endl;
    }
    else {
        // If console does not exist, display a message
        cout << "Console \"" << name << "\" does not exist." << endl;
    }
}

/*
* This function displays the current general CPU info
*/
void ConsoleManager::displayCPUInfo() {
    int usedCores = coreCount - availableCores;

	float cpuUsage = 0.0;
	if (usedCores > 0) {
		cpuUsage = (usedCores / (float) coreCount) * 100;
	}

	cout << "CPU Cores: " << coreCount << endl;
	cout << "CPU Utilization: " << fixed << setprecision(2) << cpuUsage << "%" << endl;
    cout << "Cores used: " << usedCores << endl;
    cout << "Cores available: " << availableCores << endl;
}

/*
* This function lists the status of all the consoles in the console screen
*/
void ConsoleManager::listConsoles() {
    lock_guard<mutex> lock(processMutex);

    displayCPUInfo();

    cout << "\n-----------------------------------------\n";

    if (!hasConsoles()) {
        cout << "No consoles to list.\n";
        return;
    }

    bool hasQueued = false;
    bool hasRunning = false;
    bool hasFinished = false;

  /*cout << "Queued Processes:\n";
    for (const auto& consolePair : consoles) {
        AConsole* console = consolePair.second;  
        // get all queued consoles
        if (console->getStatus() == AConsole::WAITING) {
            hasQueued = true;
            cout << console->getName() + "\t" + console->getTimestamp() + "\tCore: " + to_string(console->getCoreID()) + "\t" + to_string(console->getInstructionLine()) + "/" + to_string(console->getInstructionTotal()) + "\n";
        }
    }
    cout << "\n";
    if (!hasQueued) cout << "No queued consoles.\n"; */

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

/*
* This function prints the status of all the consoles as a .txt file
*/
void ConsoleManager::reportUtil() {
    lock_guard<mutex> lock(processMutex);

    string fileName = "console_report.txt";
    ofstream outFile(fileName, ios::out | ios::trunc);

    if (!outFile.is_open()) {
        cerr << "Error: unable to open file for writing report details";
        return;
    }

    // Start writing to the file
    outFile << "Console Report\n";
    outFile << "-----------------------------------------\n";

    // Display CPU Info
    int usedCores = coreCount - availableCores;

    float cpuUsage = 0.0;
    if (usedCores > 0) {
        cpuUsage = (usedCores / (float)coreCount) * 100;
    }

    outFile << "CPU Cores: " << coreCount << endl;
    outFile << "CPU Utilization: " << fixed << setprecision(2) << cpuUsage << "%" << endl;
    outFile << "Cores used: " << usedCores << endl;
    outFile << "Cores available: " << availableCores << endl;

    if (!hasConsoles()) {
        outFile << "No consoles to list.\n";
        outFile.close();
        return;
    }

    bool hasQueued = false;
    bool hasRunning = false;
    bool hasFinished = false;

 /* outFile << "Queued Processes:\n";
    for (const auto& consolePair : consoles) {
        AConsole* console = consolePair.second;
        if (console->getStatus() == AConsole::WAITING) {
            hasQueued = true;
            outFile << console->getName() + "\t" + console->getTimestamp() + "\tCore: " + to_string(console->getCoreID()) + "\t" + to_string(console->getInstructionLine()) + "/" + to_string(console->getInstructionTotal()) + "\n";
        }
    }
    outFile << "\n";
    if (!hasQueued) outFile << "No queued consoles.\n"; */

    outFile << "Running Processes:\n";
    for (const auto& consolePair : consoles) {
        AConsole* console = consolePair.second;
        if (console->getStatus() == AConsole::RUNNING) {
            hasRunning = true;
            outFile << console->getName() + "\t" + console->getTimestamp() + "\tCore: " + to_string(console->getCoreID()) + "\t" + to_string(console->getInstructionLine()) + "/" + to_string(console->getInstructionTotal()) + "\n";
        }
    }
    outFile << "\n";
    if (!hasRunning) outFile << "No running consoles.\n";

    outFile << "Finished Processes:\n";
    for (const auto& consolePair : consoles) {
        AConsole* console = consolePair.second;
        if (console->getStatus() == AConsole::TERMINATED) {
            hasFinished = true;
            outFile << console->getName() + "\t" + console->getTimestamp() + "\tFinished\t" + to_string(console->getInstructionLine()) + "/" + to_string(console->getInstructionTotal()) + "\n";
        }
    }
    outFile << "\n";
    if (!hasFinished) outFile << "No terminated consoles.\n";

    outFile.close();
    cout << "Report generated: " << fileName << "\n";
}


void ConsoleManager::startScheduler() {
    if (scheduler == "fcfs") {
        thread schedulerThread(&ConsoleManager::schedulerFCFS, this);
		schedulerThread.detach();
	}
    else if (scheduler == "rr") {
        thread schedulerThread(&ConsoleManager::schedulerRR, this);
        schedulerThread.detach();
    }
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
                        cout << "ID: " << console.second->getProcessID() << endl;
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
    return AConsole::TERMINATED;
}

void ConsoleManager::schedulerTest(bool set_scheduler) {
    scheduler_test_run = set_scheduler;

    startScheduler();

    thread([this] {
        int cycles = 1;
        int i = 1;

        while (scheduler_test_run) {
            if (cycles % batch_process_freq == 0) {
                if (i < 10)
                    addConsole("process00" + to_string(i));
                else if (i < 100)
                    addConsole("process0" + to_string(i));
                else
                    addConsole("process" + to_string(i));

                i++;
            }
            this_thread::sleep_for(chrono::milliseconds(100)); 
            cycles++;
        }
    }).detach();
}
/*
void ConsoleManager::processSMI() {
    int cpuUtilization = 0;
    int usedMemory = 0;
    int maxOverallMemory = memoryManager->getMaxOverallMem();
    int memoryUtilization = 0;

    std::cout << "--------------------------------------------\n";
    std::cout << "| PROCESS-SMI V01.00 Driver Version 01.00 | \n";
    std::cout << "--------------------------------------------\n";
    std::cout << "CPU-Util: " << cpuUtilization << "%\n";
    std::cout << "Memory Usage: " << usedMemory << "/" << maxOverallMemory << "\n";
    std::cout << "Memory-Util: " << memoryUtilization << "%" << "%\n";
    std::cout << "============================================    \n";
    std::cout << "Running processes and memory usage: \n";
    std::cout << "--------------------------------------------\n";

    // TODO: Display running processes and memory usage

    // This does not yet have memory usage displayed
    for (const auto& consolePair : consoles) {
        AConsole* console = consolePair.second;
        // get all running consoles
        if (console->getStatus() == AConsole::RUNNING) {
            cout << console->getName() + "\n";
        }
    }
}*/

void ConsoleManager::processSMI() {
	int usedMemory = memoryManager->calculateNumberofProcesses() * memoryManager->getMinMemPerProc();
    int freeMemory = memoryManager->getMaxOverallMem() - usedMemory;

    cout << "--------------------------------------------\n";
    cout << "| PROCESS-SMI V01.00 Driver Version 01.00 |\n";
    cout << "--------------------------------------------\n";
    cout << "Total Memory: " << memoryManager->getMaxOverallMem() << " KB\n";
    cout << "Used Memory: " << usedMemory << " KB\n";
    cout << "Free Memory: " << freeMemory << " KB\n";
    cout << "--------------------------------------------\n";
    cout << "Running Processes:\n";

    for (const auto& consolePair : consoles) {
        AConsole* console = consolePair.second;
        if (console->getStatus() == AConsole::RUNNING) {
			if (memoryManager->getProcessMemory(console->getProcessID()) > 0) {
				cout << console->getName() << " - Memory: "
					<< memoryManager->getProcessMemory(console->getProcessID()) << " KB\n";
			}
        }
    }

    /*queue<AConsole*> memoryQueueCopy = memoryQueue;

    if (!memoryQueueCopy.empty()) {
        for (AConsole* console = memoryQueueCopy.front(); !memoryQueueCopy.empty(); memoryQueueCopy.pop()) {
            if (memoryManager->getProcessMemory(console->getProcessID()) > 0) {
                cout << console->getName() << " - Memory: "
                    << memoryManager->getProcessMemory(console->getProcessID()) << " KB\n";
            }
        }
    }*/
}


void ConsoleManager::schedulerFCFS() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(10));

        lock_guard<mutex> lock(processMutex);

        size_t processesInMemory = memoryManager->calculateNumberofProcesses();
        size_t maxProcessesInMemory = memoryManager->getMaxOverallMem() / memoryManager->getMinMemPerProc();

        if (processesInMemory < maxProcessesInMemory && !waitingQueue.empty()) {
            AConsole* nextWaitingProcess = waitingQueue.front();
            waitingQueue.pop();

            // Allocate memory for the next process
            if (memoryManager->allocateMemory(nextWaitingProcess->getProcessID())) {
                // If memory allocation is successful, add process to memoryQueue
                memoryQueue.push(nextWaitingProcess);
            }
        }

        for (int i = 0; i < cpuCores.size(); ++i) {
            if (!cpuCores[i] && !memoryQueue.empty()) {
                AConsole* nextProcess = memoryQueue.front();
                memoryQueue.pop();

                cpuCores[i] = true;
                availableCores--;

                runningProcesses[nextProcess->getName()] = thread([this, nextProcess, i]() {
                    nextProcess->runProcess(i, 0, delays_per_exec);
                    lock_guard<mutex> lock(processMutex);

                    if (nextProcess->getIsActive() && nextProcess->getInstructionLine() < nextProcess->getInstructionTotal()) {
                        memoryQueue.push(nextProcess); 
                    }

                    cpuCores[i] = false;
                    availableCores++;
                    });

                runningProcesses[nextProcess->getName()].detach();
            }
        }
    }
}

void ConsoleManager::schedulerRR() {

    while (true) {
        this_thread::sleep_for(chrono::milliseconds(10));

        lock_guard<mutex> lock(processMutex);

        // Check if there's room in the memory, and if so, move the next process from waitingQueue to memoryQueue
        size_t processesInMemory = memoryManager->calculateNumberofProcesses();
        size_t maxProcessesInMemory = memoryManager->getMaxOverallMem() / memoryManager->getMinMemPerProc();

        if (processesInMemory < maxProcessesInMemory && !waitingQueue.empty()) {
            AConsole* nextWaitingProcess = waitingQueue.front();
            waitingQueue.pop();

            if (memoryManager->allocateMemory(nextWaitingProcess->getProcessID())) {
                memoryQueue.push(nextWaitingProcess);
            }
        }

        // Run processes on available CPU cores
        for (int i = 0; i < cpuCores.size(); ++i) {
            if (!cpuCores[i] && !memoryQueue.empty()) {
                AConsole* nextProcess = memoryQueue.front();
                memoryQueue.pop();

                // Process in memory, run the process
                cpuCores[i] = true;
                availableCores--;

                runningProcesses[nextProcess->getName()] = thread([this, nextProcess, i]() {
                    nextProcess->runProcess(i, quantum_cycles, delays_per_exec);
                    lock_guard<mutex> lock(processMutex);

                    // After execution, check if the process is still active
                    if (nextProcess->getIsActive() && nextProcess->getInstructionLine() < nextProcess->getInstructionTotal()) {
                        memoryQueue.push(nextProcess); // Requeue if still active (in memory)
                    }

                    cpuCores[i] = false;
                    availableCores++;
                    });

                runningProcesses[nextProcess->getName()].detach();
            }
        }

    }
}
void ConsoleManager::vmStat() const {
    mutex processMutex;
    lock_guard<mutex> lock(processMutex);

    // Memory statistics
    size_t totalMemory = memoryManager->getMaxOverallMem();
    size_t usedMemory = memoryManager->calculateUsedMemory();
    size_t freeMemory = totalMemory - usedMemory;

    // CPU ticks
    size_t idleTicks = 0;
    size_t activeTicks = 0;
    size_t totalTicks = 0;
    for (const auto& core : cpuCores) {
        if (core) { // Core is active
            activeTicks++;
        }
        else { // Core is idle
            idleTicks++;
        }
    }
    totalTicks = idleTicks + activeTicks;

    // Paging statistics
    size_t numPagedIn = memoryManager->getPagedInCount();
    size_t numPagedOut = memoryManager->getPagedOutCount();

    // Display the statistics
    cout << "--------------------------------------------\n";
    cout << "| VIRTUAL MEMORY STATISTICS               |\n";
    cout << "--------------------------------------------\n";
    cout << "Total Memory (KB): " << totalMemory << "\n";
    cout << "Used Memory (KB): " << usedMemory << "\n";
    cout << "Free Memory (KB): " << freeMemory << "\n";
    cout << "--------------------------------------------\n";
    cout << "CPU Statistics:\n";
    cout << "Idle CPU Ticks: " << idleTicks << "\n";
    cout << "Active CPU Ticks: " << activeTicks << "\n";
    cout << "Total CPU Ticks: " << totalTicks << "\n";
    cout << "--------------------------------------------\n";
    cout << "Paging Statistics:\n";
    cout << "Number of Pages Paged In: " << numPagedIn << "\n";
    cout << "Number of Pages Paged Out: " << numPagedOut << "\n";
    cout << "--------------------------------------------\n";
}

