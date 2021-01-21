/*
 * assembler.h
 * Chip8 Assembler main class
 */
#ifndef CHIP8_ASSEMBLER_H
#define CHIP8_ASSEMBLER_H

// includes
#include <cstdint>
#include <string>

#include "parser.h"


// main disassembler class
class Assembler
{
    public:     // public methods
        Assembler();                                    // constructor
        ~Assembler();                                   // destructor

        // disable move/copy semantics
        Assembler(const Assembler&) = delete;
        Assembler(Assembler&&) = delete;
        Assembler& operator=(const Assembler&) = delete;
        Assembler& operator=(Assembler&&) = delete;

        uint16_t compile(std::string filename);             // compile a ASM file

    private:    // private members
        uint16_t output;                             // the assembled byte
};

#endif  // CHIP8_ASSEMBLER_H