#include "MemoryManager.h"
#include <fstream>
#include <iostream>
#include <ctime>

// Constructor: initializes memory frames based on total memory size and frame size.
MemoryManager::MemoryManager(size_t max_overall_mem, size_t mem_per_frame, size_t min_mem_per_proc, size_t max_mem_per_proc)
    : max_overall_mem(max_overall_mem),
    mem_per_frame(mem_per_frame),
    min_mem_per_proc(min_mem_per_proc),
    max_mem_per_proc(max_mem_per_proc)
{
    size_t numFrames = max_overall_mem / mem_per_frame;
    memoryFrames.resize(numFrames, { -1, false });  // Initialize frames as unoccupied

    if (max_overall_mem == mem_per_frame) {
        allocationType = "paging";
    }
    else {
        allocationType = "flat";
    }
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
    size_t requiredFrames = min_mem_per_proc / mem_per_frame;

    // Ensure process does not exceed available memory
    if (requiredFrames * mem_per_frame > max_overall_mem) {
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

    // Write the summary information
    file << "Timestamp: (" << timestamp << ")\n";
    file << "Number of processes in memory: " << calculateNumberofProcesses() << "\n";
    file << "Total external fragmentation in KB: " << calculateExternalFragmentation() << "\n";
    file << "\n----end---- = " << max_overall_mem << "\n\n";

    /*size_t occupiedMemory = 0;
    for (const auto& frame : memoryFrames) {
        if (frame.isOccupied) {
            occupiedMemory += mem_per_frame;
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
    size_t maxProcessesInMemory = max_overall_mem / min_mem_per_proc;

    // Memory Layout Representation
    size_t processesDisplayed = 0;

    // Iterate over memoryFrames and show allocated memory addresses
    for (int i = 0; i < memoryFrames.size(); ++i) {
        if (memoryFrames[i].isOccupied && processesDisplayed < maxProcessesInMemory) {
            int processID = memoryFrames[i].processID;

            // Print the upper boundary (current address)
            size_t processStartAddress = max_overall_mem - (i * mem_per_frame); // Memory address where the process starts
            size_t processEndAddress = processStartAddress - min_mem_per_proc;  // End address of the process in memory

            // Write the boundaries to the file
            file << processStartAddress << "\n"; // Upper boundary
            file << "P" << processID << "\n";  // Process ID
            file << processEndAddress << "\n\n"; // Lower boundary

            // Skip the frames occupied by this process (i.e., skip min_mem_per_proc / mem_per_frame frames)
            size_t framesToSkip = min_mem_per_proc / mem_per_frame;
            i += framesToSkip - 1;

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
    int freeSlots = 0;
    int largestContiguousBlock = 0;
    int currentBlockSize = 0;

    for (Frame frame : memoryFrames) {
        if (!frame.isOccupied) { // If the slot is free
            freeSlots++;
            currentBlockSize++;
            if (currentBlockSize > largestContiguousBlock) {
                largestContiguousBlock = currentBlockSize;
            }
        }
        else {
            currentBlockSize = 0; // Reset block size when encountering an occupied slot
        }
    }

    // External fragmentation: total free memory not in the largest contiguous block
    int fragmentedMemory = (freeSlots - largestContiguousBlock) * mem_per_frame;
    return fragmentedMemory > 0 ? fragmentedMemory : 0;
}

int MemoryManager::calculateNumberofProcesses() const {
    std::unordered_set<int> uniqueProcesses;

    // Count unique process IDs in memory
    for (const auto& frame : memoryFrames) {
        if (frame.isOccupied) {
            uniqueProcesses.insert(frame.processID);  // Insert unique process IDs
        }
    }

    // Return the count of unique processes
    return uniqueProcesses.size();
}

size_t MemoryManager::getMaxOverallMem() const {
    return max_overall_mem;
}

void MemoryManager::setMaxOverallMem(size_t max_overall_mem) {
    max_overall_mem = max_overall_mem;
}

size_t MemoryManager::getMemPerFrame() const {               
    return mem_per_frame;
}

void MemoryManager::setMemPerFrame(size_t mem_per_frame) {
    mem_per_frame = mem_per_frame;
}

size_t MemoryManager::getMinMemPerProc() const {
    return min_mem_per_proc;
}

void MemoryManager::setMinMemPerProc(size_t minMemPerProc) {
    min_mem_per_proc = minMemPerProc;
}

size_t MemoryManager::getMaxMemPerProc() const {
    return max_mem_per_proc;
}

const std::vector<MemoryManager::Frame>& MemoryManager::getMemoryFrames() const {
    return memoryFrames;
}

void MemoryManager::setMemoryFrames(const std::vector<MemoryManager::Frame>& frames) {
    memoryFrames = frames;
}

const std::string MemoryManager::getAllocationType() const {
    return allocationType;
}

void MemoryManager::setAllocationType(const std::string& type) {
    if (type == "flat" || type == "paging") {
        allocationType = type;
    }
}
