#ifndef C8DASM_H
#define C8DASM_H

#include "disassembler.h"

class C8Dasm {
    public:
        std::vector<std::vector<QString>> Disassemble(std::vector<unsigned char> rom);
};

#endif // C8DASM_H
