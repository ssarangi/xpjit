#include "midend/PassManager.h"

void PassManager::run(IcarusModule& m)
{
    std::list<ITransform*>::iterator passIter = m_passes.begin();
    for(; passIter != m_passes.end(); ++passIter)
        (*passIter)->Execute(m);
}
