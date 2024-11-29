#pragma once
#include <vector>
#include <string>
#include <unordered_set>
#include <ctime>

class MemoryManager {
private:
    struct Frame {
        int processID = -1;          // Process ID that occupies the frame
        bool isOccupied = false;     // Is the frame occupied?
        std::time_t lastAccessed = 0; // Timestamp for the last access (for paging and memory management)
    };

    std::vector<Frame> memoryFrames;
    size_t max_overall_mem;  // Total memory available for all processes
    size_t mem_per_frame;    // Size of each memory frame
    size_t min_mem_per_proc; // Minimum memory required for each process
    size_t max_mem_per_proc; // Maximum memory allowed for each process
    std::string allocationType;  // Allocation type: "flat" or "paging"

    // Helper functions
    int findFreeFrames(size_t requiredFrames) const;
    int findOldestProcess() const;  // For swapping out the oldest process
    std::string getCurrentTime() const;  // For generating snapshots

public:
    // Constructor to initialize memory parameters
    MemoryManager(size_t max_overall_mem, size_t mem_per_frame, size_t min_mem_per_proc, size_t max_mem_per_proc);

    // Memory allocation
    bool allocateMemory(int processID);

    // Free memory
    void freeMemory(int processID);

    // Memory management utilities
    int calculateNumberofProcesses() const;         // Returns the number of unique processes in memory

    // Getters and setters
    size_t getMaxOverallMem() const;
    void setMaxOverallMem(size_t maxOverallMem);

    size_t getMemPerFrame() const;
    void setMemPerFrame(size_t memPerFrame);

    size_t getMinMemPerProc() const;
    void setMinMemPerProc(size_t minMemPerProc);

    size_t getMaxMemPerProc() const;
    void setMaxMemPerProc(size_t maxMemPerProc);

    const std::vector<Frame>& getMemoryFrames() const;
    void setMemoryFrames(const std::vector<Frame>& frames);

    const std::string getAllocationType() const;
    void setAllocationType(const std::string& type);
};
