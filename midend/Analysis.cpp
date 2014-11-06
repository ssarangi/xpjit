#include "Analysis.h"

llvm::DenseMap<const llvm::Instruction*, int> instMap;
char Analysis::ID = 0;

llvm::FunctionPass* createAnalysisPass()
{
    return new Analysis();
}

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<Analysis> X("Analysis", "Analysis Pass",
    false, true);

void print_elem(const llvm::Instruction* i)
{
    llvm::errs() << instMap.lookup(i) << " ";
}

void Analysis::initInstrToLatticeBit(LatticeEncoding &instToLatticeBit, llvm::Function &F)
{
    unsigned instIdx = 0;

    // Arguments count as definitions as well - first F.arg_size() bits.
    for (llvm::Function::arg_iterator argIt = F.arg_begin(); argIt != F.arg_end(); argIt++)
    {
        instToLatticeBit.insert(std::make_pair(&*argIt, instIdx));
        instIdx++;
    }

    // Don't bother filtering out non-definition instructions. Their
    // corresponding use / in / out bits will be always zero and the
    // resulting in / out sets will look right.
    for (llvm::inst_iterator instIt = inst_begin(F); instIt != inst_end(F); instIt++)
    {
        instToLatticeBit.insert(std::make_pair(&*instIt, instIdx));
        instIdx++;
    }
}

void Analysis::initBlocksInfo(BlockInfoMapping &blockToInfo, unsigned defcount, llvm::Function &F)
{
    for (llvm::Function::iterator blockIt = F.begin(); blockIt != F.end(); blockIt++)
    {
        BasicBlockLivenessInfo *info = new BasicBlockLivenessInfo(&*blockIt, defcount);
        blockToInfo.insert(std::make_pair(blockIt, info));
    }
}

void Analysis::initBlockDef(BasicBlockLivenessInfo *blockInfo, LatticeEncoding &instToLatticeBit)
{
    llvm::BasicBlock *block = blockInfo->block;
    for (llvm::BasicBlock::iterator instIt = block->begin(); instIt != block->end(); instIt++)
    {
        blockInfo->def->set(instToLatticeBit[instIt]);
    }
}

void Analysis::initEntryArgDef(BasicBlockLivenessInfo *blockInfo, unsigned argCount)
{
    for (unsigned argBit = 0; argBit < argCount; argBit++)
        blockInfo->def->set(argBit);
}

void Analysis::markUses(
    llvm::Value::use_iterator use_begin,
    llvm::Value::use_iterator use_end,
    BlockInfoMapping &blockToInfo,
    llvm::Instruction *defInst,
    unsigned defBit)
{
    for (llvm::Value::use_iterator useIt = use_begin; useIt != use_end; useIt++)
    {
        if (llvm::Instruction *useInst = llvm::dyn_cast<llvm::Instruction>(*useIt))
        {
            llvm::BasicBlock *useBlock = useInst->getParent();
            // Avoid setting use bit for uses in the same block as the
            // definition. This is equivalent with the way use sets are
            // defined for the liveness analysis when working with SSA
            // form (thus having a textual-unique, dominant definition
            // for every use of a value).
            if (defInst == NULL || (defInst != NULL && defInst->getParent() != useBlock))
            {
                BasicBlockLivenessInfo *useBlockInfo = blockToInfo[useBlock];
                useBlockInfo->use->set(defBit);
            }
        }
    }
}

void Analysis::initBlocksUse(BlockInfoMapping &blockToInfo,
    LatticeEncoding &instToLatticeBit,
    llvm::Function &F)
{
    for (llvm::inst_iterator defIt = inst_begin(F); defIt != inst_end(F); defIt++)
    {
        llvm::Instruction *defInst = &*defIt;
        markUses(defIt->use_begin(), defIt->use_end(), blockToInfo, defInst, instToLatticeBit[defInst]);
    }
    for (llvm::Function::arg_iterator defIt = F.arg_begin(); defIt != F.arg_end(); defIt++)
    {
        markUses(defIt->use_begin(), defIt->use_end(), blockToInfo, NULL, instToLatticeBit[&*defIt]);
    }
}

void Analysis::initMask(FlowMask &mask,
    BlockInfoMapping &blockToInfo,
    LatticeEncoding &instToLatticeBit,
    llvm::Function &F)
{
    for (llvm::inst_iterator instIt = inst_begin(F); instIt != inst_end(F); instIt++)
    {
        if (!llvm::isa<llvm::PHINode>(&*instIt))
            continue;

        llvm::PHINode *phiNode = llvm::dyn_cast<llvm::PHINode>(&*instIt);
        llvm::BasicBlock *phiBlock = phiNode->getParent();

        for (unsigned crtIncomingIdx = 0; crtIncomingIdx < phiNode->getNumIncomingValues(); crtIncomingIdx++)
        {
            llvm::BasicBlock *crtBlock = phiNode->getIncomingBlock(crtIncomingIdx);
            for (unsigned otherIncomingIdx = 0; otherIncomingIdx < phiNode->getNumIncomingValues(); otherIncomingIdx++)
            {
                if (otherIncomingIdx == crtIncomingIdx)
                    continue;
                llvm::Value *otherValue = phiNode->getIncomingValue(otherIncomingIdx);
                unsigned otherBit = instToLatticeBit[otherValue];
                std::pair<llvm::BasicBlock *, llvm::BasicBlock *> maskKey = std::make_pair(crtBlock, phiBlock);
                std::map<std::pair<llvm::BasicBlock *, llvm::BasicBlock *>, llvm::BitVector *>::iterator maskIt =
                    mask.find(maskKey);
                if (maskIt == mask.end())
                {
                    mask[maskKey] = new llvm::BitVector(instToLatticeBit.size(), true);
                }
                mask[maskKey]->reset(otherBit);
            }
        }
    }
}

void Analysis::printBitVector(llvm::BitVector *bv)
{
    for (unsigned bit = 0; bit < bv->size(); bit++)
        llvm::dbgs() << (bv->test(bit) ? '1' : '0');
}

bool Analysis::runOnFunction(llvm::Function &F)
{
    // initialize encoding
    LatticeEncoding instToLatticeBit;
    initInstrToLatticeBit(instToLatticeBit, F);
    unsigned defcount = instToLatticeBit.size();

    // initialize block to info mapping
    BlockInfoMapping blockToInfo;
    initBlocksInfo(blockToInfo, defcount, F);

    // initialize def and use sets
    initEntryArgDef(blockToInfo[F.begin()], F.arg_size());
    for (BlockInfoMapping::iterator it = blockToInfo.begin(); it != blockToInfo.end(); it++) {
        initBlockDef(it->second, instToLatticeBit);
    }
    initBlocksUse(blockToInfo, instToLatticeBit, F);

    // initialize flow mask
    FlowMask mask;
    initMask(mask, blockToInfo, instToLatticeBit, F);

    // compute fixed-point liveness information - no sorting of
    // blocks in quasi-topological order, works anyway
    bool inChanged = true;
    while (inChanged)
    {
        inChanged = false;
        // out[B] = U(in[S] & mask[B][S]) where B < S
        for (llvm::Function::iterator B = F.begin(); B != F.end(); B++)
        {
            (blockToInfo[B]->out)->reset();
            for (llvm::succ_iterator succIt = succ_begin(B); succIt != succ_end(B); succIt++)
            {
                llvm::BasicBlock *S = *succIt;
                std::pair<llvm::BasicBlock *, llvm::BasicBlock *> key =
                    std::make_pair(B, S);

                if (mask.find(key) != mask.end())
                {
                    llvm::BitVector in = (*(blockToInfo[S]->in));
                    in &= *(mask[key]);
                    *(blockToInfo[B]->out) |= in;
                }
                else
                {
                    *(blockToInfo[B]->out) |= *(blockToInfo[S]->in);
                }
            }
            // in[B] = use[B] U (out[B] - def[B])
            llvm::BitVector oldIn = *(blockToInfo[B]->in);
            llvm::BitVector def = (*(blockToInfo[B]->def)).flip();
            llvm::BitVector out = *(blockToInfo[B]->out);
            out &= def;
            llvm::BitVector use = *(blockToInfo[B]->use);
            use |= out;

            *(blockToInfo[B]->in) = use;

            if (*(blockToInfo[B]->in) != oldIn)
            {
                inChanged = true;
            }
        }
    }

    // dump information to stderr
    llvm::dbgs() << "Function name: " << F.getName() << "\n";
    llvm::dbgs() << "-------------------\n";
    llvm::dbgs() << "#blocks = " << blockToInfo.size() << '\n';
    llvm::dbgs() << "#insts = " << defcount << "\n\n";

    for (llvm::Function::iterator blockIt = F.begin(); blockIt != F.end(); blockIt++)
    {
        llvm::dbgs() << "[ " << blockIt->getName() << " ]\n";
        for (llvm::BasicBlock::iterator instIt = blockIt->begin(); instIt != blockIt->end(); instIt++)
        {
            llvm::dbgs() << *instIt << "\n";
        }

        BasicBlockLivenessInfo *info = blockToInfo[blockIt];
        llvm::BitVector *def = info->def;
        llvm::BitVector *use = info->use;
        llvm::BitVector *in = info->in;
        llvm::BitVector *out = info->out;
        llvm::dbgs() << "Def:\t";
        printBitVector(def);
        llvm::dbgs() << '\n';
        llvm::dbgs() << "Use:\t";
        printBitVector(use);
        llvm::dbgs() << '\n';
        llvm::dbgs() << "In:\t";
        printBitVector(in);
        llvm::dbgs() << '\n';
        llvm::dbgs() << "Out:\t";
        printBitVector(out);
        llvm::dbgs() << '\n';
    }

    llvm::dbgs() << "\nMask size: " << mask.size() << '\n';
    for (std::map<std::pair<llvm::BasicBlock *, llvm::BasicBlock *>, llvm::BitVector *>::iterator maskIt = mask.begin(); maskIt != mask.end(); maskIt++)
    {
        llvm::dbgs() << (maskIt->first).first->getName() << " -> " << (maskIt->first).second->getName() << ":\t";
        printBitVector(maskIt->second);
        llvm::dbgs() << '\n';
    }

    llvm::dbgs() << "===================\n";

    // TODO: free memory

    return true;
}
