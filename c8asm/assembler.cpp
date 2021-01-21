/*
 * assembler.cpp
 * Chip8 Assembler main class
 */

// includes
#include <iostream>
#include <cstdint>
#include <cstring>      // ::memset
#include <list>         // std::list
#include <map>          // std::map


#include "assembler.h"

#include "types.h"
#include "parser.h"
#include "comphelpers.h"


// opcodes/byte codes functions mapping table
typedef uint16_t (*callback)(t_token, Parser*);
std::map<std::string, callback> opcodes_map;

extern uint16_t convert(std::string);


// constructor
Assembler::Assembler()
{
    // prepare the mapping table
    opcodes_map["JP"]   = &jumpOrCall;
    opcodes_map["CALL"] = &jumpOrCall;

    opcodes_map["LD"]   = &load;

    opcodes_map["SE"]   = &skip;
    opcodes_map["SNE"]  = &skip;

    opcodes_map["NOP"]  = &misc;
    opcodes_map["CLS"]  = &misc;
    opcodes_map["RET"]  = &misc;
    opcodes_map["SKP"]  = &misc;
    opcodes_map["SKNP"] = &misc;
    opcodes_map["RND"]  = &misc;
    opcodes_map["DRW"]  = &misc;

    opcodes_map["AND"]  = &logic;
    opcodes_map["OR"]   = &logic;
    opcodes_map["XOR"]  = &logic;
    opcodes_map["SHR"]  = &logic;
    opcodes_map["SHL"]  = &logic;

    opcodes_map["ADD"]  = &add;
    opcodes_map["SUB"]  = &sub;
    opcodes_map["SUBN"] = &sub;
}

// destructor
Assembler::~Assembler()
{
}

// compile an ASM file
uint16_t Assembler::compile(std::string instruction)
{
    // create a new parser
    Parser* p = new (std::nothrow) Parser(instruction);
    if( p == nullptr )
        throw std::string("Unable to allocate memory for the parser.");

    // token & output buffer
    t_token t{TOKEN_INIT, ""};

    // compile the source
    try
    {
        // retrieve the next token
        t = p->next();

        // token is an operand
        if( t.first == TOKEN_OPERAND ) {
            // lookup for the operand in the map
            auto it = opcodes_map.find(t.second);

            // found it
            if( it != opcodes_map.end()) {
                // call the function
                uint16_t value = (it->second)(t, p);

                // copy the assembled byte for return
                output = value;
            }
            else {
                throw std::string("Unknown instruction.");
            }
        }

        // token is a series of bytes
        if( t.first == TOKEN_BYTE ) {
            // retrieve all the values associated with "DATA BYTE"
            t_token x = p->next();
            while( (x.first != TOKEN_EOL) && (x.first != TOKEN_END)) {
                uint8_t value = convert(x.second) & 0xFF;
                output = value;
                x = p->next();
            }
        }
    } catch(std::string& e) {
        std::cout << e << std::endl;
        delete p;
        return false;
    }

    // release parser memory
    delete p;

    return output;
}
