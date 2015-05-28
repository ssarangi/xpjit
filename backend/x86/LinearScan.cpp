#include "LinearScan.h"

#include "common/debug.h"
#include <assert.h>

#include <asmjit/base/operand.h>
#include <asmjit/x86/x86operand.h>

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

bool LinearScanAllocator::assignRegistersForALU(LiveRangeInterval *pInterval)
{
    bool assigned_register = false;
    // Check if the instruction is an ALU instruction or not.
    llvm::Instruction *pI = pInterval->pInstr;
    llvm::Value *pOp1 = pI->getOperand(0);

    switch (pI->getOpcode())
    {
    case llvm::Instruction::Add:
    case llvm::Instruction::Mul:
        // Destination of the add instruction is the first operand. So we need to use the same register as the first
        // operand.
        if (!llvm::isa<llvm::Constant>(pOp1))
        {
            // Assign this register.
            BaseVariable *pReg = getSymbol(pOp1);
            if (pReg != nullptr)
            {
                pReg->print(g_outputStream());
                g_outputStream.flush();
                m_liveRangeIntervalToBackendRegister[pInterval] = pReg;
                assigned_register = true;
            }
        }
        break;
    case llvm::Instruction::Sub:
    case llvm::Instruction::FSub:
        break;
    case llvm::Instruction::FAdd:
    case llvm::Instruction::FMul:
        break;
    case llvm::Instruction::FDiv:
        break;
    }

    return assigned_register;
}

std::vector<LiveRangeInterval*> LinearScanAllocator::sortLiveInterval()
{
    const IntervalMapTy interval_map = m_pLR->getIntervalMap();

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

        BaseVariable *pVar = m_liveRangeIntervalToBackendRegister[active_interval];

        if (pVar->isInstanceOf(REGISTER))
        {
            BackendRegister *pReg = static_cast<BackendRegister*>(pVar);
            assert(pReg != nullptr);
            free_registers.push(pReg);

            assert(m_liveRangeIntervalToBackendRegister.find(active_interval) != m_liveRangeIntervalToBackendRegister.end());

            g_outputStream << "Expiring interval (";
            pReg->print(g_outputStream()); g_outputStream << "): ";
            active_interval->pInstr->print(g_outputStream());
            g_outputStream << "\n";
        }
    }

    for (auto remove_interval : remove_intervals)
        active.erase(remove_interval);
}

void LinearScanAllocator::spillAtInterval(LiveRangeInterval *pInterval, std::set<LiveRangeInterval*> &active)
{
    // Find the longest interval from the end
    LiveRangeInterval *pSpill_interval = *(--active.end());

    BackendVariable *pSpillVar = new BackendVariable(m_stackLocation);

    if (pSpill_interval->kill_id > pInterval->kill_id)
    {
        // Use the same register the spill interval was using
        m_liveRangeIntervalToBackendRegister[pInterval] = m_liveRangeIntervalToBackendRegister[pSpill_interval];
        m_liveRangeIntervalToBackendRegister[pSpill_interval] = pSpillVar;
        active.erase(pSpill_interval);
        m_liveRangeIntervalToBackendRegister.erase(pSpill_interval);
        active.insert(pInterval);
    }
    else
    {
        m_liveRangeIntervalToBackendRegister[pInterval] = pSpillVar;
    }

    m_stackLocation += 4;
}

void LinearScanAllocator::performLinearScan()
{
    std::vector<LiveRangeInterval*> sorted_intervals = sortLiveInterval();
    std::set<LiveRangeInterval*> active;
    std::stack<asmjit::Operand*> free_registers;

    // Do not add these registers. For now we will use a simplistic register allocation scheme.
    //free_registers.push(&RAX);
    //free_registers.push(&RBX);
    //free_registers.push(&RCX);
    //free_registers.push(&RDX);

    free_registers.push(&asmjit::x86::r8d);
    free_registers.push(&asmjit::x86::r9d);
    free_registers.push(&asmjit::x86::r10d);
    free_registers.push(&asmjit::x86::r11d);
    free_registers.push(&asmjit::x86::r12d);
    free_registers.push(&asmjit::x86::r13d);
    free_registers.push(&asmjit::x86::r14d);
    free_registers.push(&asmjit::x86::r15d);

    //// Precolor some of the intervals
    //for (auto interval : sorted_intervals)
    //{
    //    interval->pInstr->print(g_outputStream());
    //    g_outputStream << "\n";
    //    assignRegistersForALU(interval);
    //}

    for (auto interval : sorted_intervals)
    {
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
    ADD_HEADER("Linear Scan Register Allocator Pass: " + F.getName().str());

    m_pLR = &getAnalysis<LiveRange>();

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

    return false;
}