#include "x86defines.h"

//////////////////////////////// X86 Registers ///////////////////////////////////
// General Registers
BackendRegister EAX("eax");
BackendRegister EBX("ebx");
BackendRegister ECX("ecx");
BackendRegister EDX("edx");

// Segment Registers
BackendRegister CS("cs");
BackendRegister DS("ds");
BackendRegister ES("es");
BackendRegister FS("fs");
BackendRegister GS("gs");
BackendRegister SS("ss");

// Index and pointers
BackendRegister ESI("esi");
BackendRegister EDI("edi");
BackendRegister EBP("ebp");
BackendRegister EIP("eip");
BackendRegister ESP("esp");

///////////////////////////////// X64 Registers ////////////////////////////////////
// General Registers
BackendRegister RAX("rax");
BackendRegister RBX("rbx");
BackendRegister RCX("rcx");
BackendRegister RDX("rdx");

BackendRegister RSI("rsi");
BackendRegister RDI("rdi");

BackendRegister RSP("rsp");
BackendRegister RBP("rbp");

BackendRegister R8("r8");
BackendRegister R9("r9");
BackendRegister R10("r10");
BackendRegister R11("r11");
BackendRegister R12("r12");
BackendRegister R13("r13");
BackendRegister R14("r14");
BackendRegister R15("r15");