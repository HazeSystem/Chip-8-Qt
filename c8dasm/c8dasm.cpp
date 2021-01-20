#include "c8dasm.h"
#include "disassembler.h"

std::vector<std::vector<QString>> C8Dasm::Disassemble(std::vector<unsigned char> rom) {
    Disassembler c8dasm;
    c8dasm.max_address_ = rom.size();

    try {
        // load the ROM
        c8dasm.rom_ = rom;

        // display the code
        c8dasm.render();

    } catch(std::string &e) {
        std::cerr << "An error occurred during disassembly procedure:";
        std::cerr << e << std::endl;
    }

    return c8dasm.program;
}
