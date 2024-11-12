#include "MemoryManager.h"
#include <fstream>
#include <iostream>
#include <ctime>

// Constructor: initializes memory frames based on total memory size and frame size.
MemoryManager::MemoryManager(size_t memorySize, size_t frameSize, size_t memPerProc)
    : totalMemory(memorySize), frameSize(frameSize), memPerProc(memPerProc) {
    size_t numFrames = memorySize / frameSize;
    memoryFrames.resize(numFrames, { -1, false });  // Initialize frames as unoccupied
}

// Allocate memory for a process using the first-fit method.
bool MemoryManager::allocateMemory(int processID) {

    // Check if the process is already in memory
    bool isAlreadyAllocated = false;
    for (const auto& frame : memoryFrames) {
        if (frame.isOccupied && frame.processID == processID) {
            isAlreadyAllocated = true;
            break;
        }
    }

    // If the process is already in memory, return true to avoid duplicate allocation
    if (isAlreadyAllocated) {
        return false;
    }

    // Calculate required number of frames for the process
    size_t requiredFrames = memPerProc / frameSize;

    // Ensure process does not exceed available memory
    if (requiredFrames * frameSize > totalMemory) {
        return false; // Not enough memory to allocate the process
    }

    // Find free frames for the process
    int startFrame = findFreeFrames(requiredFrames);

    if (startFrame == -1) {
        return false; // No sufficient free space found
    }

    // Allocate memory for the process: mark the frames as occupied
    for (size_t i = startFrame; i < startFrame + requiredFrames; ++i) {
        memoryFrames[i] = { processID, true };  // Mark each frame as occupied by the process
    }


    /*std::cout << "Memory Frames: [";
    for (size_t i = 0; i < memoryFrames.size(); ++i) {
        if (memoryFrames[i].isOccupied) {
            std::cout << "P" << memoryFrames[i].processID;
        }
        else {
            std::cout << "*";
        }
        if (i != memoryFrames.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]\n"; */

    return true;
}


// Free memory for a process
void MemoryManager::freeMemory(int processID) {
    for (auto& frame : memoryFrames) {
        if (frame.processID == processID) {
            frame.isOccupied = false;  
            frame.processID = -1;
        }
    }
}


// Generate a snapshot of the current memory status, saving it to a file.
void MemoryManager::generateSnapshot(int quantumCycle) const {
    std::ofstream file("memory_stamp_" + std::to_string(quantumCycle) + ".txt");

    std::string timestamp = getCurrentTime();
    int externalFragmentation = calculateExternalFragmentation();

    // Write the summary information
    file << "Timestamp: (" << timestamp << ")\n";
    file << "Number of processes in memory: " << (totalMemory - externalFragmentation) / memPerProc << "\n";
    file << "Total external fragmentation in KB: " << externalFragmentation << "\n";
    file << "\n----end---- = " << totalMemory << "\n\n";

    /*size_t occupiedMemory = 0;
    for (const auto& frame : memoryFrames) {
        if (frame.isOccupied) {
            occupiedMemory += frameSize;
        }
    }

    size_t totalMemoryInFrames = memoryFrames.size();

    // Display occupied and maximum memory per process
    file << "Current Occupied Memory: " << occupiedMemory << " bytes\n";
    file << "Total Memory Size of Memory Frames: " << totalMemoryInFrames << " bytes\n\n";


    // Display the memory frames contents (simple view of memory frames)
    file << "Memory Frames: [";
    for (size_t i = 0; i < memoryFrames.size(); ++i) {
        if (memoryFrames[i].isOccupied) {
            file << "P" << memoryFrames[i].processID;
        }
        else {
            file << "*";
        }
        if (i != memoryFrames.size() - 1) {
            file << ", ";
        }
    }
    file << "]\n\n"; */


    // Calculate the maximum number of processes that can fit in memory
    size_t maxProcessesInMemory = totalMemory / memPerProc;

    // Memory Layout Representation
    size_t currentAddress = totalMemory;  
    size_t processesDisplayed = 0;  

    // Iterate over memoryFrames and show allocated memory addresses
    for (int i = 0; i < memoryFrames.size(); ++i) {
        if (memoryFrames[i].isOccupied && processesDisplayed < maxProcessesInMemory) {
            int processID = memoryFrames[i].processID;

            // Print the upper boundary
            file << currentAddress << "\n";

            // Display the process ID
            file << "P" << processID << "\n";

            // Print the lower boundary
            file << currentAddress - memPerProc << "\n\n";

            // Skip the frames occupied by this process (i.e., skip memPerProc / frameSize frames)
            size_t framesToSkip = memPerProc / frameSize;
            i += framesToSkip - 1; 

            // Move current address down by the size of a process
            currentAddress -= memPerProc; 
            processesDisplayed++;

            if (processesDisplayed >= maxProcessesInMemory) break;
        }
    }

    file << "----start---- = 0\n";
    file.close();
}

// Find the starting frame index for the first contiguous block of free frames.
int MemoryManager::findFreeFrames(size_t requiredFrames) const {
    size_t consecutiveFreeFrames = 0;
    int startFrame = -1;

    for (size_t i = 0; i < memoryFrames.size(); ++i) {
        if (!memoryFrames[i].isOccupied) {
            if (startFrame == -1) startFrame = i;
            consecutiveFreeFrames++;
            if (consecutiveFreeFrames == requiredFrames) {
                return startFrame;
            }
        }
        else {
            consecutiveFreeFrames = 0;
            startFrame = -1;
        }
    }
    return -1;
}

// Get the current system time for snapshot
std::string MemoryManager::getCurrentTime() const {
    std::time_t now = std::time(0);
    std::tm localTime;
    localtime_s(&localTime, &now);
    char buffer[50];
    std::strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S%p", &localTime);
    return buffer;
}

// Calculate external fragmentation in the system
int MemoryManager::calculateExternalFragmentation() const {
    std::unordered_set<int> uniqueProcesses;
    int totalMemoryUsed = 0;

    // Count unique process IDs in memory and calculate memory they occupy
    for (const auto& frame : memoryFrames) {
        if (frame.isOccupied) {
            uniqueProcesses.insert(frame.processID);
        }
    }

    totalMemoryUsed = uniqueProcesses.size() * memPerProc;

    int fragmentationKB = totalMemory - totalMemoryUsed;

    // Ensure fragmentation is not negative
    if (fragmentationKB < 0) fragmentationKB = 0;

    return fragmentationKB; 
}

size_t MemoryManager::getFrameSize() const {
    return frameSize;
}

void MemoryManager::setFrameSize(size_t frameSize) {
    frameSize = frameSize;
}

size_t MemoryManager::getTotalMemory() const {
    return totalMemory;
}

void MemoryManager::setTotalMemory(size_t totalMemory) {
    totalMemory = totalMemory;
}

size_t MemoryManager::getMemPerProc() const {
    return memPerProc;
}

void MemoryManager::setMemPerProc(size_t memPerProc) {
    memPerProc = memPerProc;
}

const std::vector<MemoryManager::Frame>& MemoryManager::getMemoryFrames() const {
    return memoryFrames;
}

void MemoryManager::setMemoryFrames(const std::vector<MemoryManager::Frame>& frames) {
    memoryFrames = frames;
}