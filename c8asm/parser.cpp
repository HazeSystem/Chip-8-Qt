/*
 * parser.cpp
 * Parse a Chip8 assembly file and create the corresponding flow of tokens
 */

// includes
#include <cstring>

#include "parser.h"

// constructor
Parser::Parser(std::string instruction) :
    inst(instruction)
{
}

// destructor
Parser::~Parser()
{
}

// return the next character
char Parser::get()
{
    char c;
    inst.get(c);
    if( inst.eof() )
        throw StopIteration();

    return c;
}

// peek the next character in the file
char Parser::peek()
{
    if( inst.eof() )
        throw StopIteration();

    return inst.peek();
}

t_token Parser::next()
{
    try
    {
        ::memset(token_, 0, 20);
        offset_ = 0;
        while(true)
        {
            char car = get();

            // end of string
            if( car == '\n' ) {
                return std::make_pair(TOKEN_EOL, "");
            }

            // detect tokens
            if( offset_ ) {

                // operand separator
                if( car == ' ' || car == ',' || peek() == '\n') {
                    // insert the character
                    if(peek() == '\n')
                        token_[offset_] = toupper(car);

                    // token is a value
                    if( token_[0] == '#' ) {
                        return std::make_pair(TOKEN_VALUE, &token_[1]);
                    }

                    // token is a Vx register
                    if( token_[0] == 'V' ) {
                        return std::make_pair(TOKEN_REGISTER, &token_[1]);
                    }

                    if( ::strcmp(token_, "DB") == 0) {
                        return std::make_pair(TOKEN_BYTE, token_);
                    }

                    return std::make_pair(TOKEN_OPERAND, token_);
                }
            }

            // record the character
            token_[offset_++] = toupper(car);
            if( offset_ > MAX_TOKEN_LENGTH )
                throw std::string("The token is too big.");
        }

    } catch(StopIteration) {
        return std::make_pair(TOKEN_END, "");
    }
}