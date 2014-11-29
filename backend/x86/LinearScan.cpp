#include "LinearScan.h"

#include "common/debug.h"
#include <assert.h>

#define NUM_X86_REGISTERS 4

char LinearScanAllocator::ID = 0;

 //register the printCode class: 
 // - give it a command-line argument
 // - a name
 // - a flag saying that we don't modify the CFG
 // - a flag saying this is not an analysis pass
llvm::RegisterPass<LinearScanAllocator> T("LinearScanAllocator", "Linear Scan Register Allocator", false, true);

LinearScanAllocator *createLinearScanRegisterAllocationPass()
{
    LinearScanAllocator *pPass = new LinearScanAllocator();
    return pPass;
}

std::vector<LiveRangeInterval*> LinearScanAllocator::sortLiveInterval()
{
    LiveRange &LR = getAnalysis<LiveRange>();
    const IntervalMapTy interval_map = LR.getIntervalMap();

    // Sort the live range intervals first
    std::vector<LiveRangeInterval*> sorted_intervals;
    for (auto interval : interval_map)
        sorted_intervals.push_back(interval.second);

    std::sort(sorted_intervals.begin(), sorted_intervals.end(), PointerCompare());

    // Pre-color any register with the registers they need.
    for (auto interval : sorted_intervals)
    {
        g_outputStream << "Def: " << interval->def_id << " , Kill: " << interval->kill_id;
        interval->pInstr->print(g_outputStream()); g_outputStream << "\n";
        g_outputStream.flush();
    }

    return sorted_intervals;
}

void LinearScanAllocator::expireOldIntervals(LiveRangeInterval *pInterval, std::set<LiveRangeInterval*> &active, std::stack<BackendRegister*> &free_registers)
{
    std::set<LiveRangeInterval*> remove_intervals;

    for (auto active_interval : active)
    {
        if (active_interval->kill_id >= pInterval->def_id)
            break;

        remove_intervals.insert(active_interval);

        BackendRegister *pReg = m_liveRangeIntervalToBackendRegister[active_interval];
        if (pReg == nullptr)
        {
            assert(m_seenInterval.find(active_interval) != m_seenInterval.end());
        }

        assert(pReg != nullptr);
        free_registers.push(pReg);

        assert(m_liveRangeIntervalToBackendRegister.find(active_interval) != m_liveRangeIntervalToBackendRegister.end());

        // m_liveRangeIntervalToBackendRegister.erase(active_interval);
        g_outputStream << "Expiring interval (";
        pReg->print(g_outputStream()); g_outputStream << "): ";
        active_interval->pInstr->print(g_outputStream());
        g_outputStream << "\n";
    }

    for (auto remove_interval : remove_intervals)
        active.erase(remove_interval);
}

void LinearScanAllocator::spillAtInterval(LiveRangeInterval *pInterval, std::set<LiveRangeInterval*> &active)
{
    // Find the longest interval from the end
    LiveRangeInterval *pSpill_interval = *(--active.end());

    if (pSpill_interval->kill_id > pInterval->kill_id)
    {
        // Use the same register the spill interval was using
        m_liveRangeIntervalToBackendRegister[pInterval] = m_liveRangeIntervalToBackendRegister[pSpill_interval];
        m_stackLocationMapping[pSpill_interval] = m_stackLocation;
        active.erase(pSpill_interval);
        m_liveRangeIntervalToBackendRegister.erase(pSpill_interval);
        active.insert(pInterval);
    }
    else
    {
        m_stackLocationMapping[pInterval] = m_stackLocation;
    }

    m_stackLocation += 4;
}

void LinearScanAllocator::performLinearScan()
{
    std::vector<LiveRangeInterval*> sorted_intervals = sortLiveInterval();
    std::set<LiveRangeInterval*> active;
    std::stack<BackendRegister*> free_registers;

    free_registers.push(&EAX);
    free_registers.push(&EBX);
    free_registers.push(&ECX);
    free_registers.push(&EDX);

    for (auto interval : sorted_intervals)
    {
        m_seenInterval.insert(interval);

        // Expire old intervals
        expireOldIntervals(interval, active, free_registers);

        if (active.size() == NUM_X86_REGISTERS)
            spillAtInterval(interval, active);
        else
        {
            BackendRegister *pReg = free_registers.top();
            free_registers.pop();
            m_liveRangeIntervalToBackendRegister[interval] = pReg;
            active.insert(interval);
        }
    }
}

bool LinearScanAllocator::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("SSA Deconstruction Pass");

    performLinearScan();
    
    g_outputStream << "\nRegisters allocated\n";
    g_outputStream << "-----------------------------------\n";

    for (auto reg_mapping : m_liveRangeIntervalToBackendRegister)
    {
        reg_mapping.first->pInstr->print(g_outputStream());
        g_outputStream << " --> ";
        reg_mapping.second->print(g_outputStream());
        g_outputStream << " { " << reg_mapping.first->def_id << " , " << reg_mapping.first->kill_id << " }";
        g_outputStream << "\n";
    }

    g_outputStream << "\nRegisters spilled\n";
    g_outputStream << "-----------------------------------\n";

    for (auto spill_mapping : m_stackLocationMapping)
    {
        spill_mapping.first->pInstr->print(g_outputStream());
        g_outputStream << " --> ";
        g_outputStream << spill_mapping.second;
        g_outputStream << " { " << spill_mapping.first->def_id << " , " << spill_mapping.first->kill_id << " }";
        g_outputStream << "\n";
    }

    return false;
}