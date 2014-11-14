#include <midend/LoopAnalysis.h>
#include <common/debug.h>

#include <stack>
#include <vector>

char LoopAnalysis::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<LoopAnalysis> T("LoopAnalysisCustomAnalysis", "Loop Analysis Custom analysis", false, true);

LoopAnalysis *createNewLoopAnalysisPass()
{
    LoopAnalysis *pL = new LoopAnalysis();
    return pL;
}

void LoopAnalysis::findBasicLoopInductionVar(NaturalLoopTy &natural_loop)
{
    g_outputStream() << "Basic Induction Variables: \n";

    // Figure out the basic loop induction variable
    for (llvm::BasicBlock *pB : natural_loop.blocks)
    {
        for (llvm::BasicBlock::iterator i = pB->begin(), e = pB->end();
            i != e;
            ++i)
        {
            // Match the pattern v = v op c where c is a constant and op could be a binary op
            if (llvm::isa<llvm::BinaryOperator>(i))
            {
                llvm::Value *pOp1 = i->getOperand(0);
                llvm::Value *pOp2 = i->getOperand(1);

                if (llvm::isa<llvm::ConstantInt>(pOp1) || llvm::isa<llvm::ConstantInt>(pOp2))
                {
                    LoopInductionVarTriple triple;
                    triple.pInst = &*i;
                    triple.pV = llvm::isa<llvm::ConstantInt>(pOp1) ? pOp2 : pOp1;
                    triple.a = 0;
                    triple.b = 1;
                    natural_loop.basic_induction_var.push_back(triple);

                    i->print(g_outputStream());
                    g_outputStream() << "\n";
                    g_outputStream.flush();
                }
            }
        }
    }
}

void LoopAnalysis::deriveInductionVar(NaturalLoopTy &natural_loop)
{
    g_outputStream() << "Derived Induction Variables: \n";

    // Figure out the basic loop induction variable
    for (llvm::BasicBlock *pB : natural_loop.blocks)
    {
        for (llvm::BasicBlock::iterator i = pB->begin(), e = pB->end();
            i != e;
            ++i)
        {
            // Match the pattern v = v op c where c is a constant and op could be a binary op
            if (llvm::isa<llvm::BinaryOperator>(i))
            {
                llvm::Value *pOp1 = i->getOperand(0);
                llvm::Value *pOp2 = i->getOperand(1);

                bool findOp1 = 
                    std::find(natural_loop.basic_induction_var.begin(), natural_loop.basic_induction_var.end(), pOp1) != natural_loop.basic_induction_var.end();

                bool findOp2 =
                    std::find(natural_loop.basic_induction_var.begin(), natural_loop.basic_induction_var.end(), pOp2) != natural_loop.basic_induction_var.end();

                if (findOp1 || findOp2)
                {
                    LoopInductionVarTriple triple;
                    triple.pInst = &*i;
                    triple.pV = llvm::isa<llvm::ConstantInt>(pOp1) ? pOp2 : pOp1;
                    triple.a = 0;
                    triple.b = 1;
                    natural_loop.derived_induction_var.push_back(triple);

                    i->print(g_outputStream());
                    g_outputStream() << "\n";
                    g_outputStream.flush();
                }
            }
        }
    }
}

bool LoopAnalysis::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("Loop Analysis");
    F.getParent()->print(g_outputStream(), nullptr);
    g_outputStream.flush();

    m_pDT = &getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();

    std::vector<std::pair<llvm::BasicBlock*, llvm::BasicBlock*>> backedges;

    // Now iterate through the basic blocks from the function
    for (llvm::Function::iterator bb = F.begin(), be = F.end();
        bb != be;
        ++bb)
    {
        // Identify a back-edge
        for (llvm::succ_iterator succBB = llvm::succ_begin(bb), succBBE = llvm::succ_end(bb);
            succBB != succBBE;
            ++succBB)
        {
            // A node can dominate itself too
            if (m_pDT->dominates(*succBB, bb))
            {
                g_outputStream() << "ControlFlow: " << bb->getName().str() << " --> " << (*succBB)->getName().str() << "\n";
                backedges.emplace_back(bb, *succBB);
            }
        }
    }

    g_outputStream.flush();

    // Now that we have the back edges figure out all the BB's involved in the loop i.e. figure out the natural loop
    unsigned int loopID = 0;
    for (std::pair<llvm::BasicBlock*, llvm::BasicBlock*> srctarget_pair : backedges)
    {
        llvm::BasicBlock *pSrc = srctarget_pair.first;
        llvm::BasicBlock *pTarget = srctarget_pair.second;

        NaturalLoopTy natural_loop;

        natural_loop.ID = loopID++;
        natural_loop.blocks.insert(pSrc);
        natural_loop.blocks.insert(pTarget);

        // If the Basic Block loops on itself
        if (pSrc != pTarget)
        {
            std::stack<llvm::BasicBlock*> bb_visited;
            bb_visited.push(pSrc);

            while (!bb_visited.empty())
            {
                llvm::BasicBlock *pBB = bb_visited.top();
                bb_visited.pop();

                for (llvm::pred_iterator pi = llvm::pred_begin(pBB), pe = llvm::pred_end(pBB);
                    pi != pe;
                    ++pi)
                {
                    natural_loop.blocks.insert(*pi);

                    if (*pi != pBB && pTarget != pBB && !natural_loop.blocks.count(*pi))
                        bb_visited.push(*pi);
                }
            }
        }

        findBasicLoopInductionVar(natural_loop);
        deriveInductionVar(natural_loop);
        m_naturalLoops.push_back(natural_loop);
    }

    F.getParent()->print(g_outputStream(), nullptr);

    return false;
}