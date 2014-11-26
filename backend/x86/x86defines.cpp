#include "x86defines.h"

// General Registers
BackendRegister EAX("%eax");
BackendRegister EBX("%ebx");
BackendRegister ECX("%ecx");
BackendRegister EDX("%edx");

// Segment Registers
BackendRegister CS("%cs");
BackendRegister DS("%ds");
BackendRegister ES("%es");
BackendRegister FS("%fs");
BackendRegister GS("%gs");
BackendRegister SS("%ss");

// Index and pointers
BackendRegister ESI("%esi");
BackendRegister EDI("%edi");
BackendRegister EBP("%ebp");
BackendRegister EIP("%eip");
BackendRegister ESP("%esp");