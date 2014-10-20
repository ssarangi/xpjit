#include "tempstacksize.h"

#include <common/llvm_warnings_push.h>
#include <llvm/IR/Constants.h>
#include <common/llvm_warnings_pop.h>

static llvm::RegisterPass<TemporaryStackSize> X("TemporaryStackSize", "Calculate the temporaries used in function", false, false);

char TemporaryStackSize::ID = 0;

bool TemporaryStackSize::runOnFunction(llvm::Function &F)
{
    unsigned int numTemporaries = 0;

    llvm::Instruction* pII = nullptr;
    // Loop through all the functions and figure out how many temporaries we would need
    for (llvm::Function::iterator bb = F.begin(), bbend = F.end(); bb != bbend; ++bb)
    {
        // Look at every instruction and see if it has a return value
        for (llvm::BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i)
        {
            if (i->getNumUses() > 0)
                ++numTemporaries;
            pII = i;
        }
    }

    m_numTemporaries[&F] = numTemporaries;

    /* Experiment with Metadata 
    // Add a few metadata
    llvm::Module *pM = F.getParent();
    llvm::NamedMDNode* pNamedNode = pM->getOrInsertNamedMetadata("buffer_flags");
    llvm::MDString* pS = llvm::MDString::get(pM->getContext(), "my_named_string_node");
    std::vector<llvm::Value*> nodes;
    nodes.push_back(pS);
    llvm::ConstantInt* pCI= llvm::ConstantInt::get(llvm::Type::getInt32Ty(pM->getContext()), 2);
    nodes.push_back(pCI);
    llvm::MDNode* pMDNode = llvm::MDNode::get(pM->getContext(), nodes);
    pNamedNode->addOperand(pMDNode);

    pII->setMetadata("buffer_type", pMDNode);

    pM->dump();
    */
    return false;
}

int TemporaryStackSize::getNumTemporaries(llvm::Function *pFunc)
{
    assert(m_numTemporaries.find(pFunc) != m_numTemporaries.end() && "No function entry for temporaries");
    return m_numTemporaries[pFunc];
}
