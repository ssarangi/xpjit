#ifndef __LLVM_WARNINGS_PUSH__
#define __LLVM_WARNINGS_PUSH__

#undef LLVM_WARNINGS_POP
#define LLVM_WARNINGS_PUSH

#pragma warning (disable : 4146)
#pragma warning (disable : 4355)
#pragma warning (disable : 4800)
#pragma warning (disable : 4482)
#pragma warning (disable : 4244)
#pragma warning (disable : 4242)
#pragma warning (disable : 4996)

#ifdef _WIN64

// 'argument' : conversion from 'size_t' to 'unsigned int', possible loss of data
#   pragma warning( disable : 4267 )

#endif

#endif