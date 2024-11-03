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

ConsoleManager::ConsoleManager() {

	readConfig("config.txt");

    coreCount = num_cpu;
    availableCores = num_cpu;

    cpuCores = vector<bool>(num_cpu, false);
    startScheduler();
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
            if (quantum_cycles < 1 || quantum_cycles > MAX_VALUE) {
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

/*  cout << "Queued Processes:\n";
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

void ConsoleManager::schedulerTest(bool set_scheduler) {
    scheduler_test_run = set_scheduler;

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

void ConsoleManager::schedulerFCFS() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(100));

        lock_guard<mutex> lock(processMutex);

        for (int i = 0; i < cpuCores.size(); ++i) {
            if (!cpuCores[i] && !waitingQueue.empty()) {
                AConsole* nextProcess = waitingQueue.front();
                waitingQueue.pop();

                cpuCores[i] = true;
                availableCores--;

                runningProcesses[nextProcess->getName()] = thread([this, nextProcess, i]() {
                    nextProcess->runProcess(i, 0, delays_per_exec);
                    lock_guard<mutex> lock(processMutex);
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
        this_thread::sleep_for(chrono::milliseconds(100));

        lock_guard<mutex> lock(processMutex);

        for (int i = 0; i < cpuCores.size(); ++i) {
            if (!cpuCores[i] && !waitingQueue.empty()) {
                AConsole* nextProcess = waitingQueue.front();
                waitingQueue.pop();

                cpuCores[i] = true;
                availableCores--;

                runningProcesses[nextProcess->getName()] = thread([this, nextProcess, i]() {
                    nextProcess->runProcess(i, quantum_cycles, delays_per_exec);
                    lock_guard<mutex> lock(processMutex);

                    // If the process has not completed, requeue it
                    if (nextProcess->getIsActive() && nextProcess->getInstructionLine() < nextProcess->getInstructionTotal()) {
                        waitingQueue.push(nextProcess);
                    }
                    cpuCores[i] = false;
                    availableCores++;
                    });

                runningProcesses[nextProcess->getName()].detach();
            }
        }
    }
}