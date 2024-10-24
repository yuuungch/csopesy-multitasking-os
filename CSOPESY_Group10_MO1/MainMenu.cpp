#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "ConsoleManager.h"
#include "AConsole.h"

using namespace std;

ConsoleManager consoles;
bool isInitialized = false;
int num_cpu;
string scheduler;
int quantum_cycles;
int batch_process_freq;
int min_ins;
int max_ins;
int delays_per_exec;

const uint64_t MAX_VALUE = 4294967296;

/*
* This function prints the ASCII text header
*/
void displayHeader() {
    cout << R"(
 ________  ________  ________  ________  _______   ________       ___    ___ 
|\   ____\|\   ____\|\   __  \|\   __  \|\  ___ \ |\   ____\     |\  \  /  /|
\ \  \___|\ \  \___|\ \  \|\  \ \  \|\  \ \   __/|\ \  \___|_    \ \  \/  / /
 \ \  \    \ \_____  \ \  \\\  \ \   ____\ \  \_|/_\ \_____  \    \ \    / / 
  \ \  \____\|____|\  \ \  \\\  \ \  \___|\ \  \_|\ \|____|\  \    \/  /  /  
   \ \_______\____\_\  \ \_______\ \__\    \ \_______\____\_\  \ __/  / /    
    \|_______|\_________\|_______|\|__|     \|_______|\_________\\___/ /     
             \|_________|                            \|_________\|___|/              
    )" << endl;

    cout << "\033[32m" << "Hello, Welcome to CSOPESY command-line interface." << "\033[0m" << endl;
    cout << "\033[33m" << "Type 'exit' to quit, 'clear' to clear the screen.\n" << "\033[0m" << endl;
}

/*
* This function clears the console screen
*/
void clearCommand() {
    system("cls");
}

/*
* This function reads the config.txt file and initializes parameters
*/
bool readConfig(const string& filename) {
    ifstream configFile(filename);
    if (!configFile.is_open()) {
        cerr << "Error: Could not open config file.\n";
        return false;
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
                return false;
            }
        }
        else if (key == "scheduler") {
            string value;
            iss >> quoted(value);  // Use std::quoted to handle quotes
            scheduler = value;  // Assign the stripped value
            if (scheduler != "fcfs" && scheduler != "rr") {
                cerr << "Error: Invalid scheduler value: '" << scheduler << "'. Must be 'fcfs' or 'rr'.\n";
                return false;
            }
        }
        else if (key == "quantum-cycles") {
            iss >> quantum_cycles;
            if (quantum_cycles < 1 || quantum_cycles > MAX_VALUE) {
                cerr << "Error: Invalid quantum-cycles value: " << quantum_cycles << ". Must be in range [1, " << MAX_VALUE << "].\n";
                return false;
            }
        }
        else if (key == "batch-process-freq") {
            iss >> batch_process_freq;
            if (batch_process_freq < 1 || batch_process_freq > MAX_VALUE) {
                cerr << "Error: Invalid batch-process-freq value: " << batch_process_freq << ". Must be in range [1, " << MAX_VALUE << "].\n";
                return false;
            }
        }
        else if (key == "min-ins") {
            iss >> min_ins;
            if (min_ins < 1 || min_ins > MAX_VALUE) {
                cerr << "Error: Invalid min-ins value: " << min_ins << ". Must be in range [1, " << MAX_VALUE << "].\n";
                return false;
            }
        }
        else if (key == "max-ins") {
            iss >> max_ins;
            if (max_ins < 1 || max_ins > MAX_VALUE) {
                cerr << "Error: Invalid max-ins value: " << max_ins << ". Must be in range [1, " << MAX_VALUE << "].\n";
                return false;
            }
        }
        else if (key == "delays-per-exec") {
            iss >> delays_per_exec;
            if (delays_per_exec < 0 || delays_per_exec > MAX_VALUE) {
                cerr << "Error: Invalid delays-per-exec value: " << delays_per_exec << ". Must be in range [0, " << MAX_VALUE << "].\n";
                return false;
            }
        }
        else {
            cerr << "Error: Unknown parameter in config file: " << key << endl;
            return false;
        }
    }

    cout << "Configuration successfully loaded.\n";
    configFile.close();
    return true;
}

/*
* This function processes the screen command
* User may opt to start a new console or reopen an existing console
*
* @param commandBuffer - a vector of strings containing the command and its arguments
*/
void screenCommand(const vector<string>& commandBuffer) {
    // screen command error validation
    if (commandBuffer.size() == 2) {
        if (commandBuffer[1] == "-s" || commandBuffer[1] == "-r")
            cout << "Usage: screen [-r | -s] [name]\n";

        else if (commandBuffer[1] == "-ls") {
            cout << "Listing all running consoles\n";
            consoles.listConsoles();
        }
        else {
            cout << "Screen command \"" << commandBuffer[1] << "\" not recognized. Try again.\n";
        }
    }
    // if screen command is valid
    else if (commandBuffer.size() == 3) {
        // if screen command is "start console"
        if (commandBuffer[1] == "-s") {
            // check if console already exists
            if (consoles.consoleExists(commandBuffer[2])) {
                cout << "Console \"" << commandBuffer[2] << "\" already exists.\n";
            }
            // if console does not exist, create new console
            else {
                clearCommand();
                consoles.addConsole(commandBuffer[2], 100, true); // Add new console to console list
                consoles.loopConsole(commandBuffer[2]); // initialize console program
                clearCommand();
                displayHeader();
            }
        }
        // if screen command is "reopen console"
        else if (commandBuffer[1] == "-r") {
            // Check if console exists
            if (!consoles.consoleExists(commandBuffer[2])) {
                cout << "Process \"" << commandBuffer[2] << "\" not found." << endl;
            }
            // Check if the console has finished execution
            else if (consoles.getConsoleStatus(commandBuffer[2]) == AConsole::TERMINATED) {
                cout << "Process \"" << commandBuffer[2] << "\" not found." << endl;
            }
            // If console exists and is still running, reopen console
            else {
                cout << "Reopening console \"" << commandBuffer[2] << "\"\n";
                consoles.displayConsole(commandBuffer[2]); // Display reopened console
                consoles.loopConsole(commandBuffer[2]); // Reinitialize console program
                clearCommand();
                displayHeader();
            }
        }
        // if screen command syntax is invalid
        else {
            cout << "Screen command \"" << commandBuffer[1] << "\" not recognized. Try again.\n";
        }
    }
    // if command syntax is invalid
    else {
        cout << "Usage: screen [-r | -s] [name]\n";
    }
}

/*
* This function checks if the input command is valid (accepted) or not
* including specific actions for clear, exit, and screen commands
*
* @param commandBuffer - a vector of strings containing the command and its arguments
*/
void checkCommand(const vector<string>& commandBuffer) {

    string command = commandBuffer[0];

    if (!isInitialized && command != "initialize") {
        cout << "Please run the 'initialize' command first.\n";
        return;
    }

    if (command == "initialize") {
        if (isInitialized) {
            cout << "Already initialized.\n";
            return;
        }
        if (readConfig("config.txt")) {
            isInitialized = true;
        }
    }
    else if (command == "screen") {
        screenCommand(commandBuffer);
    }
    else if (command == "clear") {
        clearCommand();
        displayHeader();
    }
    else if (command == "exit") {
        cout << command << " command recognized. Thank you! Exiting program.\n";
        exit(0);
    }
    else {
        cout << "Command " << command << " not recognized. Please try again.\n";
    }
}

int main() {
    vector<string> commandBuffer;
    string command;

    displayHeader();

    while (true) {
        commandBuffer.clear();

        cout << "Enter command: ";

        // This loop reads the command and iteratively pushes each string read 
        // into the commandBuffer vector until it detects a newline
        while (cin >> command) {
            commandBuffer.push_back(command);
            if (cin.peek() == '\n')
                break;
        }

        // if commandBuffer is not empty, check command entered
        if (!commandBuffer.empty()) {
            checkCommand(commandBuffer);
        }
    }

    return 0;
}
