#ifndef ITRANSFORM_H
#define ITRANSFORM_H

#include "frontend/codegen.h"

class ITransform 
{
public:
	virtual void Execute(IcarusModule& ) = 0;
};

#endif //ITRANSFORM_H
