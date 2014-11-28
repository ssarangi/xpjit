#include "LinearScan.h"

#include "common/debug.h"

char LinearScanAllocator::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<LinearScanAllocator> T("LinearScanAllocator", "Linear Scan Register Allocator", false, true);

LinearScanAllocator *createLinearScanRegisterAllocationPass()
{
    LinearScanAllocator *pPass = new LinearScanAllocator();
    return pPass;
}

void LinearScanAllocator::sortLiveInterval()
{
    LiveRange &LR = getAnalysis<LiveRange>();
    const IntervalMapTy interval_map = LR.getIntervalMap();

    // Sort the live range intervals first
    std::vector<LiveRangeInterval> sorted_intervals;
    for (auto interval : interval_map)
        sorted_intervals.push_back(*interval.second);

    std::sort(sorted_intervals.begin(), sorted_intervals.end());

    // Pre-color any register with the registers they need.
    for (auto interval : sorted_intervals)
    {
        g_outputStream << "Def: " << interval.def_id << " , Kill: " << interval.kill_id;
        interval.pInstr->print(g_outputStream()); g_outputStream << "\n";
        g_outputStream.flush();
    }
}

void LinearScanAllocator::performLinearScan()
{
    sortLiveInterval();
}

bool LinearScanAllocator::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("SSA Deconstruction Pass");

    performLinearScan();

    return false;
}
