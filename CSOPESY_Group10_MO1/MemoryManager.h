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
    size_t frameSize;
    size_t totalMemory;
    size_t memPerProc;

    int findFreeFrames(size_t requiredFrames) const;
    std::string getCurrentTime() const;

    public:
        MemoryManager(size_t memorySize, size_t frameSize, size_t memPerProc);

        // memory allocation
        bool allocateMemory(int processID);

        // free memory
        void freeMemory(int processID);

        // for memory status
        void generateSnapshot(int quantumCycle) const;

        int calculateExternalFragmentation() const;

        int calculateNumberofProcesses() const;

        size_t getFrameSize() const;
        void setFrameSize(size_t frameSize);

        size_t getTotalMemory() const;
        void setTotalMemory(size_t totalMemory);

        size_t getMemPerProc() const;
        void setMemPerProc(size_t memPerProc);

        const std::vector<Frame>& getMemoryFrames() const;
        void setMemoryFrames(const std::vector<Frame>& frames);
   };
