/*
 * Calculates the dominance frontiers for each basic block in a given function
 * The algorithm is based on the "The Simple, Fast Dominance Algorithm - Keith D. Cooper, Timothy J. Harvey and Ken Kennedy
 * http://www.hipersoft.rice.edu/grads/publications/dom14.pdf
 */

#include "midend/DominanceFrontier.h"
#include "common/debug.h"
#include "midend/DominanceTreeConstructor.h"

#include <common/llvm_warnings_push.h>
#include <llvm/ADT/DepthFirstIterator.h> //For iterating through the predecessors of the basic block
#include <llvm/IR/CFG.h> //We need a specialization of the GraphTrait class 
#include <llvm/PassSupport.h>
#include <common/llvm_warnings_pop.h>

#include <map>
#include <set>

using namespace std;
using namespace llvm;
char DominanceFrontier::ID = 0;

bool DominanceFrontier::runOnFunction(Function &F)
{
    ADD_HEADER("Dominance Frontier");
    g_outputStream << " Calculating Dominator Frontier for " << F.getName() << "\n";

    DominanceTreeConstructor& DT = getAnalysis<DominanceTreeConstructor>(); 
    std::map<BasicBlock*, BasicBlock*>& doms = DT.getDoms();
    
    Function::BasicBlockListType& bbList = F.getBasicBlockList();
    for(Function::BasicBlockListType::iterator iter = bbList.begin(); iter != bbList.end() ;++iter)
    {
        BasicBlock* bb = dynamic_cast<BasicBlock*>(&*iter);
        if(bb->getSinglePredecessor() == NULL){ //there are multiple predecessors to this block
            for(pred_iterator predIter = pred_begin(bb); //for each of its predecessor
                                              predIter != pred_end(bb); 
                                              ++predIter)
            {
                BasicBlock* pred = *predIter;
                BasicBlock* runner = pred;
                while(runner != doms[bb])
                {
                    //add b to runner's dominance frontier set
                    m_frontiers[runner].insert(bb);
                    runner = doms[runner];
                }
            }
        }
    }
    //if debug
    print();
    return false;
}

void DominanceFrontier::print()
{
    g_outputStream <<"Printing Dominance Frontier Information\n";
    std::map<BasicBlock*, std::set<BasicBlock*> >::iterator frontierIter = m_frontiers.begin();
    for(; frontierIter != m_frontiers.end(); ++frontierIter)
    {
        g_outputStream << frontierIter->first->getName() << " --> ";
        std::set<BasicBlock*>& fSet = frontierIter->second;
        std::set<BasicBlock*>::iterator setIter = fSet.begin();
        for(; setIter != fSet.end(); ++setIter)
            g_outputStream << (*setIter)->getName() << ", ";
        g_outputStream << "\n";
    }
}
