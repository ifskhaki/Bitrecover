#ifndef STATUS_DISPLAY_H
#define STATUS_DISPLAY_H

#include "bitrecover/types.h"
#include <vector>
#include <string>

class StatusDisplay {
public:
    StatusDisplay();
    ~StatusDisplay() = default;

    void update(const std::vector<bitrecover::GPUStats>& stats);
    void showMatch(const bitrecover::MatchResult& match);
    void clear();

private:
    void printHeader();
    void printGPUStats(const bitrecover::GPUStats& stats);
    bool firstUpdate_;
};

#endif // STATUS_DISPLAY_H

