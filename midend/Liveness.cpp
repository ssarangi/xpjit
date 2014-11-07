
#include "Liveness.h"
#include "common/debug.h"

char Liveness::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<Liveness> X("liveVars", "Live vars analysis", false, false);

Liveness *createNewLivenessPass()
{
    Liveness *pL = new Liveness();
    return pL;
}

void printMatrix(AdjMatrixTy& matrix)
{
    for (unsigned int i = 0; i < matrix.size(); ++i)
    {
        std::string currLine = "";

        for (unsigned int j = 0; j < matrix[i].size(); ++j)
        {
            currLine += std::to_string(matrix[i][j]) + " ";
        }

        currLine += "\n";

        g_outputStream.stream() << currLine;
    }

    g_outputStream.flush();
}

AdjMatrixTy initialize2DMatrix(int size)
{
    AdjMatrixTy matrix2d;
    matrix2d.resize(size);

    for (unsigned int i = 0; i < size; ++i)
        matrix2d[i].resize(size);

    return matrix2d;
}

void Liveness::initializeAdjacencyMatrix(llvm::Function &F)
{
    int numBlocks = F.size();
    // Reset the adjacency matrix to 0
    m_adjacencyMatrix = initialize2DMatrix(numBlocks);

    unsigned int id = 0;
    for (llvm::Function::iterator bb = F.begin(), be = F.end(); bb != be; ++bb)
    {
        m_idToBlock[id] = bb;
        m_blockToId[bb] = id++;
    }

    // loop through the successors and set the adjacency matrix
    for (llvm::Function::iterator bb = F.begin(), be = F.end(); bb != be; ++bb)
    {
        unsigned int bb_id = m_blockToId[bb];

        for (llvm::succ_iterator it = llvm::succ_begin(bb), ie = llvm::succ_end(bb); it != ie; ++it)
        {
            unsigned int it_id = m_blockToId[*it];
            m_adjacencyMatrix[bb_id][it_id] = 1;
        }
    }
}

void Liveness::computeWarshallTransitiveClosure(llvm::Function &F)
{
    // Copy the Adjacency matrix to another matrix
    int numBlocks = F.size();
    AdjMatrixTy Rprev = m_adjacencyMatrix;
    AdjMatrixTy Rcurr = initialize2DMatrix(numBlocks);
    AdjMatrixTy Rnew = initialize2DMatrix(numBlocks);
    for (unsigned int k = 0; k < numBlocks; ++k)
    {
        for (unsigned int i = 0; i < numBlocks; ++i)
        {
            for (unsigned int j = 0; j < numBlocks; ++j)
            {
                Rcurr[i][j] = Rprev[i][j] | (Rprev[i][k] & Rprev[k][j]);

                // Now remove all the back-edges.
                llvm::BasicBlock *pBB1 = m_idToBlock[i];
                llvm::BasicBlock *pBB2 = m_idToBlock[j];

                // If the target dominates the source, then this is a back edge. So remove it.
                if (m_pDT->dominates(pBB2, pBB1))
                    Rcurr[i][j] = 0;
            }

            llvm::BasicBlock *pBB1 = m_idToBlock[k];
            llvm::BasicBlock *pBB2 = m_idToBlock[i];

            std::string bb1 = pBB1->getName();
            std::string bb2 = pBB2->getName();

            // Adjacency matrix check to see if there is a direct path or not.
            // Dominator tree to see if its the back-edge
            if (m_pDT->dominates(pBB2, pBB1) && m_adjacencyMatrix[k][i])
                m_backEdgesMatrix[k][i] = 1;
        }

        Rprev = Rcurr;
        Rcurr = Rnew;
    }

    m_transitiveClosure = Rprev;
}

bool Liveness::isLive(llvm::Value *pQuery, llvm::Instruction *pInstNode)
{
    return false;
}

bool Liveness::isLiveInBlock(const llvm::Value *pQuery, const llvm::BasicBlock *pBlock)
{
    return false;
}

bool Liveness::isLiveOutBlock(const llvm::Value *pQuery, const llvm::BasicBlock *pBlock)
{
    return false;
}

bool Liveness::runOnFunction(llvm::Function &F)
{
    llvm::DominatorTreeWrapperPass *pDTW = &getAnalysis<llvm::DominatorTreeWrapperPass>();
    m_pDT = &pDTW->getDomTree();

    m_backEdgesMatrix = initialize2DMatrix(F.size());
    initializeAdjacencyMatrix(F);
    computeWarshallTransitiveClosure(F);

    printMatrix(m_adjacencyMatrix);

    g_outputStream.stream() << "-------------------------------------- Transitive Closure ------------------------------------\n";
    g_outputStream.flush();

    printMatrix(m_transitiveClosure);

    g_outputStream.stream() << "-------------------------------------- Back Edges ------------------------------------\n";
    g_outputStream.flush();

    printMatrix(m_backEdgesMatrix);

    return false;
}