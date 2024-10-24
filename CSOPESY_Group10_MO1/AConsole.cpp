#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <random>
#include <thread>
#include "AConsole.h"

static int processCounter = 0;

/*
* This constructor instantiates a new console given its name, instruction line, and instruction total
* 
* @param name - the name of the console
* @param instructionTotal - the total number of instructions
*/
AConsole::AConsole(const std::string& name, int instructionTotal)
	: name(name), processID(++processCounter), instructionLine(0), instructionTotal(instructionTotal), isActive(true), status(WAITING), coreID(-1), timestamp(getCurrentTime()) {}

void AConsole::runProcess(int coreID) {
    this->coreID = coreID;
    getCurrentTime();
    status = RUNNING;
    
    string fileName = name + "_log.txt";
    ofstream outFile(fileName, ios::out | ios::trunc);  

    if (outFile.is_open()) {
        outFile << "Process name: " + name + "\n";
        outFile << "Logs:\n\n";
    }
    else {
        cerr << "Error: unable to open file for writing details";
    }
    random_device rd;
    knuth_b knuth_gen(rd());
    uniform_int_distribution<> dist(20, 200);

    while (isActive && instructionLine < instructionTotal) {
        //cout << "enter while loop\n";
		int randomDelay = dist(knuth_gen);

        this_thread::sleep_for(chrono::milliseconds(randomDelay));
        instructionLine++;

        string generatedTimestamp = getCurrentTime();
		timestamp = generatedTimestamp;

        if (outFile.is_open()) {
            outFile << generatedTimestamp << " Core:" << coreID << " \"Hello world from " << name << "!\"" << "\n";
        }
    }
    if (outFile.is_open()) {
        outFile.close();
    }

    status = TERMINATED;

}

/*
* This function returns the name of the console
* 
* @return name - the name of the console
*/
std::string AConsole::getName() const { 
    return name; 
}

/*
* This function returns the timestamp when the console was created
* 
* @return timestamp - the timestamp when the console was created
*/
std::string AConsole::getTimestamp() const { 
    return timestamp; 
}

/*
* This function returns the current instruction line number
* 
* @return instructionLine - the current instruction line number

*/
int AConsole::getInstructionLine() const {
	return instructionLine;
}

/*
* This function sets the current instruction line number
* 
* @param instructionLine - the new instruction line number
*/
void AConsole::setInstructionLine(int instructionLine) {
	this->instructionLine = instructionLine;
}

/*
* This function returns the total number of instructions
* 
* @return instructionTotal - the total number of instructions
*/
int AConsole::getInstructionTotal() const { 
    return instructionTotal; 
}
/*
* This function sets the total number of instructions
* 
* @param instructionTotal - the new total number of instructions
*/
void AConsole::setInstructionTotal(int instructionTotal) {
	this->instructionTotal = instructionTotal;
}

AConsole::Status AConsole::getStatus() {
	return status;
}

int AConsole::getCoreID() {
    return coreID;
}

int AConsole::getProcessID() const {
    return processID;
}

void AConsole::setProcessID(int id) {
    processID = id;
}

/*
* This function returns the current time in the format (MM/DD/YYYY HH:MM:SS AM/PM)
* 
* @return buffer - the current time in the format (MM/DD/YYYY HH:MM:SS AM/PM)
*/
std::string AConsole::getCurrentTime() {
    std::time_t now = std::time(0);
    std::tm localTime;
    localtime_s(&localTime, &now);
    char buffer[50];
    std::strftime(buffer, sizeof(buffer), "(%m/%d/%Y %H:%M:%S%p)", &localTime);
    return buffer;
}