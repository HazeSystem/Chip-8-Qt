/*
 * comphelpers.h
 * Functions to transform tokens into bytes code
 */

#ifndef CHIP8_ASM_COMPHELPERS_H
#define CHIP8_ASM_COMPHELPERS_H

#include <string>       // std::stoi

#include "types.h"
#include "parser.h"

// convert an hexadecimal string value to its integer value
uint16_t convert(std::string value);


// Jump or Call operands
uint16_t jumpOrCall(t_token, Parser*);

// Load operand
uint16_t load(t_token, Parser*);

// Add operand
uint16_t add(t_token, Parser*);

// Sub operand
uint16_t sub(t_token, Parser*);

// Logic operands
uint16_t logic(t_token, Parser*);

// Skip operands
uint16_t skip(t_token, Parser*);

// Miscellaneous operands
uint16_t misc(t_token, Parser*);


#endif // CHIP8_ASM_COMPHELPERS_H