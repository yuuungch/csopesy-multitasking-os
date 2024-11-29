#include "MemoryManager.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdlib>

// Constructor: initializes memory frames based on total memory size and frame size.
MemoryManager::MemoryManager(size_t max_overall_mem, size_t mem_per_frame, size_t min_mem_per_proc, size_t max_mem_per_proc)
    :
    max_overall_mem(max_overall_mem),
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

    // Determine a random memory size for the process between min and max memory required
    size_t processMemory = min_mem_per_proc + rand() % (max_mem_per_proc - min_mem_per_proc + 1);
    size_t requiredPages = processMemory / mem_per_frame;

    // Ensure we have enough memory to allocate the process
    if (requiredPages * mem_per_frame > max_overall_mem) {
        return false; // Not enough memory to allocate the process
    }

    if (allocationType == "paging") {
        // Paging allocation logic
        int startFrame = findFreeFrames(requiredPages);

        if (startFrame == -1) {
            // If no free frames are available, we need to swap out an old process to the backing store
            int oldestProcessID = findOldestProcess();
            freeMemory(oldestProcessID);

            // Try to allocate memory again after swapping out
            startFrame = findFreeFrames(requiredPages);
            if (startFrame == -1) {
                return false; // Still no free frames available after swapping out
            }
        }

        // Allocate memory for the process (load the pages into memory)
        for (size_t i = startFrame; i < startFrame + requiredPages; ++i) {
            memoryFrames[i] = { processID, true, std::time(0) }; // Mark the page as occupied and set lastAccessed timestamp
        }

    }
    else { // Flat memory allocation
        // Flat allocation logic: Process memory must fit within contiguous frames
        size_t requiredFrames = processMemory / mem_per_frame;

        // Ensure process memory doesn't exceed the available space
        if (requiredFrames * mem_per_frame > max_overall_mem) {
            return false; // Not enough memory to allocate the process
        }

        // Find a contiguous block of free frames for the process
        int startFrame = findFreeFrames(requiredFrames);

        if (startFrame == -1) {
            // If no contiguous space is available, swap out the oldest process
            int oldestProcessID = findOldestProcess();
            freeMemory(oldestProcessID);

            // Try again after swapping out
            startFrame = findFreeFrames(requiredFrames);
            if (startFrame == -1) {
                return false; // Still no contiguous space available
            }
        }

        // Allocate memory for the process (mark frames as occupied)
        for (size_t i = startFrame; i < startFrame + requiredFrames; ++i) {
            memoryFrames[i] = { processID, true, std::time(0) }; // Mark the frame as occupied and set lastAccessed timestamp
        }
    }

    return true;
}

// Free memory for a process
void MemoryManager::freeMemory(int processID) {
    for (auto& frame : memoryFrames) {
        if (frame.processID == processID) {
            frame.isOccupied = false;
            frame.processID = -1;
            frame.lastAccessed = 0; // Clear lastAccessed when freeing memory
        }
    }
}

// Find the starting frame index for the first contiguous block of free frames.
int MemoryManager::findFreeFrames(size_t requiredFrames) const {
    if (allocationType == "flat") {
        // Flat memory allocation (requires contiguous blocks of memory)
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
    }
    else if (allocationType == "paging") {
        // Paging memory allocation (non-contiguous pages)
        size_t freeFramesFound = 0;
        int startFrame = -1;

        for (size_t i = 0; i < memoryFrames.size(); ++i) {
            if (!memoryFrames[i].isOccupied) {
                if (freeFramesFound == 0) {
                    startFrame = i; // Mark the start of the free frames
                }
                freeFramesFound++;

                // If we've found the required number of free frames, return the start index
                if (freeFramesFound == requiredFrames) {
                    return startFrame;
                }
            }
        }
    }

    return -1; // No sufficient free frames available
}

// Find the process that has been in memory the longest (oldest process)
int MemoryManager::findOldestProcess() const {
    int oldestProcessID = -1;
    std::time_t oldestAccessTime = std::time_t(0); // Initialize to epoch time (oldest)

    // Loop through all memory frames to find the process with the oldest access time
    for (const auto& frame : memoryFrames) {
        if (frame.isOccupied) {
            // If we haven't found a process yet or if this one was accessed earlier
            if (oldestProcessID == -1 || frame.lastAccessed < oldestAccessTime) {
                oldestProcessID = frame.processID;
                oldestAccessTime = frame.lastAccessed;
            }
        }
    }

    return oldestProcessID; // Return the process ID of the oldest process
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
