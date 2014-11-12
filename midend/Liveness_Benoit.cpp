
#include <midend/Liveness_Benoit.h>
#include <common/debug.h>

char Liveness::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<Liveness> X("LivenessAnalysis", "Live vars analysis", false, false);

Liveness *createNewLivenessPass()
{
    Liveness *pL = new Liveness();
    return pL;
}

AdjMatrixTy initialize2DMatrix(int size)
{
    AdjMatrixTy matrix2d;
    matrix2d.resize(size);

    for (unsigned int i = 0; i < size; ++i)
        matrix2d[i].resize(size);

    return matrix2d;
}

AdjMatrixTy CopyFromArray(unsigned int matrix[11][11], int size)
{
    AdjMatrixTy vecMatrix = initialize2DMatrix(size);

    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            vecMatrix[i][j] = matrix[i][j];
        }
    }

    return vecMatrix;
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

void Liveness::addRelevantBackEdges(AdjMatrixTy& reachabilityMatrix, unsigned int queryBBId, unsigned int defBBId)
{
    // Look at the backedges present in the entire graph and add them relevant to this query.
    int size = reachabilityMatrix.size();
    llvm::BasicBlock *pDefBlock = m_idToBlock[defBBId];

    for (int i = 0; i < size; ++i)
    {
        if (m_backEdgesMatrix[queryBBId][i])
        {
            // This means a back edge is present.
            // Condition to add it is that the target of the backedge should be dominated strictly by the def block
            llvm::BasicBlock *pTarget = m_idToBlock[i];
            if (defBBId != i && m_pDT->dominates(pDefBlock, pTarget))
            {
                // Add this to the reachability matrix
                reachabilityMatrix[queryBBId][i] = 1;

                // Now recursively find all the child nodes from this node
                addRelevantBackEdges(reachabilityMatrix, i, defBBId);

                int newTargetAdded = i;

                // Now add the reachable nodes for the newly added target to all the predecessors
                for (int target_predecessor = 0; target_predecessor < size; ++target_predecessor)
                {
                    // If there was already a path from the predecessor to the target then update the reachability matrix of
                    // predecessor
                    if (reachabilityMatrix[target_predecessor][newTargetAdded])
                    {
                        for (int reaches = 0; reaches < size; ++reaches)
                            reachabilityMatrix[target_predecessor][reaches] |= reachabilityMatrix[newTargetAdded][reaches];
                    }
                }
            }
        }
    }
}

bool Liveness::isLive(llvm::Value *pQueryValue, llvm::Instruction *pQueryPoint)
{
    return true;
}

bool Liveness::isLiveInBlock(llvm::BasicBlock *pDefBB, llvm::BasicBlock *pQueryBB)
{
    // The algorithm is as follows.
    // 1) Find the query block
    // 2) Find all the reduced reachable nodes from this query block.

    unsigned int queryBBId = m_blockToId[pQueryBB];
    unsigned int defBBId = m_blockToId[pDefBB];

    // Find out all the reduced reachable nodes for the query BB
    AdjMatrixTy reachabilityMatrix = m_transitiveClosure;

    addRelevantBackEdges(reachabilityMatrix, queryBBId, defBBId);

    return true;
}

bool Liveness::isLiveInBlock(llvm::Value *pQueryV, llvm::BasicBlock *pQueryBB)
{
    if (llvm::isa<llvm::Instruction>(pQueryV))
    {
        llvm::BasicBlock *pDefBB = llvm::cast<llvm::Instruction>(pQueryV)->getParent();
        return isLiveInBlock(pDefBB, pQueryBB);
    }

    return false;
}

bool Liveness::isLiveOutBlock(llvm::Value *pQuery, llvm::BasicBlock *pBlock)
{
    return true;
}

bool Liveness::runOnFunction(llvm::Function &F)
{
    llvm::DominatorTreeWrapperPass *pDTW = &getAnalysis<llvm::DominatorTreeWrapperPass>();
    m_pDT = &pDTW->getDomTree();

    // Compute the Reduced reachable set for each node of the CFG.
    m_backEdgesMatrix = initialize2DMatrix(F.size());
    
    initializeAdjacencyMatrix(F);

    //unsigned int adjmatrix[11][11] = {
    //    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    //    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
    //    { 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
    //    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
    //    { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 },
    //    { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },
    //    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    //    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
    //    { 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0 },
    //    { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
    //    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    //};

    //// Copy adjMatrix for testing.
    //m_adjacencyMatrix = CopyFromArray(adjmatrix, 11);

    computeWarshallTransitiveClosure(F);

    g_outputStream.stream() << "-------------------------------------- Adjacency Matrix ------------------------------------\n";
    g_outputStream.flush();

    printMatrix(m_adjacencyMatrix);

    g_outputStream.stream() << "-------------------------------------- Transitive Closure ------------------------------------\n";
    g_outputStream.flush();

    printMatrix(m_transitiveClosure);

    g_outputStream.stream() << "-------------------------------------- Back Edges ------------------------------------\n";
    g_outputStream.flush();

    printMatrix(m_backEdgesMatrix);

    return false;
}