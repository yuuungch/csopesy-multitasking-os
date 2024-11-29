#pragma once
#include <vector>
#include <string>
#include <unordered_set>
#include <iostream>

class MemoryManager {
private:
    struct Frame {
        int processID;
        bool isOccupied;
    };

    std::vector<Frame> memoryFrames;
    size_t max_overall_mem;  // Total memory available for all processes
    size_t mem_per_frame;    // Size of each memory frame
    size_t min_mem_per_proc; // Minimum memory required for each process
    size_t max_mem_per_proc; // Maximum memory allowed for each process

    int findFreeFrames(size_t requiredFrames) const;
    std::string getCurrentTime() const;

public:
    // Constructor to initialize memory parameters
    MemoryManager(size_t max_overall_mem, size_t mem_per_frame, size_t min_mem_per_proc, size_t max_mem_per_proc);

    // memory allocation
    bool allocateMemory(int processID);

    // free memory
    void freeMemory(int processID);

    // for memory status
    void generateSnapshot(int quantumCycle) const;

    int calculateExternalFragmentation() const;

    int calculateNumberofProcesses() const;

    // Updated function names to reflect the new variable names
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
};
