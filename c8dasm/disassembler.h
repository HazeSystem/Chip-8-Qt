/*
 * disassembler.h
 * Chip8 Disassembler main class
 */
#ifndef CHIP8_DISASSEM_H
#define CHIP8_DISASSEM_H

#include <QString>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <queue>
#include <vector>

// main disassembler class
class Disassembler
{
    public:     // public methods
        Disassembler();                                 // constructor
        ~Disassembler();                                // destructor

        // disable move/copy semantics
        Disassembler(const Disassembler&) = delete;
        Disassembler(Disassembler&&) = delete;
        Disassembler& operator=(const Disassembler&) = delete;
        Disassembler& operator=(Disassembler&&) = delete;

        void discover();                                // discover all the segments of code
        void render();                                  // render the disassembled program

        std::vector<std::vector<QString>> program;
        std::vector<unsigned char> rom_;       // the memory where the ROM is loaded
        size_t max_address_;                       // maximum address allowed for PC

    private:    // private methods
        uint16_t next();                                // return the opcode at the current PC
        void storeOpcode(uint16_t pc, uint16_t opcode);
        template<typename ... Args>
        void parseOpcode(std::string pc, std::string opcode, const std::string& format, Args ... args);

    private:    // private members
        uint16_t PC_{0};                                // program counter
        std::set<uint16_t> labels_;                     // list of labels
        std::set<uint16_t> codemap_;                    // codemap for this ROM

//        const int ROM_CODE_BEGIN{0x0200};               // code start address
};

#endif  // CHIP8_DISASSEM_H
