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

/*
 * This function simulates the execution of a process on a specified CPU core.
 * It handles both First-Come, First-Served (FCFS) and Round Robin (RR) scheduling.
 *
 * In the context of FCFS, the process runs until it completes all instructions.
 * For Round Robin, the process will yield control back to the scheduler after
 * executing a specified number of instructions (time quantum).
 *
 * Additionally, the function incorporates a busy-waiting delay mechanism, where
 * the process may wait for a specified number of CPU cycles (delaysPerExec)
 * before proceeding to the next instruction. This simulates processing time
 * that may vary depending on system conditions.
 *
 * @param coreID         - The ID of the CPU core on which the process is executing.
 * @param quantum_cycles  - The maximum number of instructions to execute before yielding
 *                          control to the scheduler (default is 0, indicating FCFS).
 * @param delaysPerExec   - The number of CPU cycles to busy-wait before executing the
 *                          next instruction. A value of 0 means no waiting, allowing
 *                          immediate execution of the next instruction.
 */
void AConsole::runProcess(int coreID, int quantum_cycles, int delaysPerExec) {
    this->coreID = coreID;
    getCurrentTime();
    status = RUNNING;

    random_device rd;
    knuth_b knuth_gen(rd());
    uniform_int_distribution<> dist(20, 200);

    int executedInstructions = 0;

    while (isActive && instructionLine < instructionTotal) {
        if (delaysPerExec > 0) {
            for (int delay = 0; delay < delaysPerExec; ++delay) {
                // This loop simulates the busy-waiting delay
            }
        }

        if (quantum_cycles > 0 && executedInstructions >= quantum_cycles) {
            status = WAITING;
            break;
        }

        // Introduce a random delay for realism, so everything won't be instant
        this_thread::sleep_for(chrono::milliseconds(dist(knuth_gen)));

        instructionLine++;
        executedInstructions++;
    }

    if (instructionLine == instructionTotal) {
        status = TERMINATED;
    }
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
* This function returns the active status of the console
*
* @return isActive - the active status of the console
*/
bool AConsole::getIsActive() const {
    return isActive;
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
* This function returns the status of the console
*
* @return status - the current status of the console
*/
AConsole::Status AConsole::getStatus() const {
    return status;
}

/*
* This function returns the core ID of the console
*
* @return coreID - the core ID of the console
*/
int AConsole::getCoreID() const {
    return coreID;
}

/*
* This function returns the process ID of the console
*
* @return processID - the process ID of the console
*/
int AConsole::getProcessID() const {
    return processID;
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
* This function sets the total number of instructions
*
* @param instructionTotal - the new total number of instructions
*/
void AConsole::setInstructionTotal(int instructionTotal) {
    this->instructionTotal = instructionTotal;
}

/*
* This function sets the process ID of the console
*
* @param id - the new process ID of the console
*/
void AConsole::setProcessID(int id) {
    processID = id;
}

/*
* This function sets the active status of the console
*
* @param active - the new active status of the console
*/
void AConsole::setIsActive(bool active) {
    isActive = active;
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