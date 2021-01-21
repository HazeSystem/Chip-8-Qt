/*
 * Jump instruction
 */

// includes
#include "comphelpers.h"


// Jump or Call operands
uint16_t jumpOrCall(t_token t, Parser* p)
{
    uint16_t value{0};
    uint16_t r1{0};

    // retrieve the next operand
    t_token op1 = p->next();

    // special case for JP V0, addr
    if( (t.second.compare("JP") == 0) && (op1.second.compare("V0") == 0) ) {
        // retrieve the next operand
        t_token op2 = p->next();
        if( op2.first == TOKEN_OPERAND ) {
            r1 = convert(op2.second);
        }
        return 0xB000 | r1 & 0x0FFF;
    }

    if( op1.first == TOKEN_OPERAND ) {
        r1 = convert(op1.second);
    }

    // Jump
    if( t.second.compare("JP") == 0)
        value = 0x1000 | (r1 & 0x0FFF);

    // Call
    if( t.second.compare("CALL") == 0)
        value = 0x2000 | (r1 & 0x0FFF);

    if( value == 0 )
        throw std::string("Unknown jump instruction!");

    return value;
}

uint16_t skip(t_token t, Parser* p)
{
    uint16_t value{0};

    // retrieve the two operands
    t_token op1 = p->next();
    t_token op2 = p->next();

    uint16_t r1 = convert(op1.second);
    uint16_t r2 = convert(op2.second);

    if( op2.first == TOKEN_VALUE )
    {
        // 3xkk - SE Vx, byte
        if( t.second.compare("SE") == 0 )
            value = 0x3000;

        // 4xkk - SNE Vx, byte
        if( t.second.compare("SNE") == 0 )
            value = 0x4000;

        value = value | (r1 & 0x0F) << 8 | (r2 & 0xFF);
    }

    if( op2.first == TOKEN_REGISTER )
    {
        // 5xy0 - SE Vx, Vy
        if( t.second.compare("SE") == 0 )
            value = 0x5000;

        // 9xy0 - SNE Vx, Vy
        if( t.second.compare("SNE") == 0 )
            value = 0x9000;

        value = value | (r1 & 0x0F) << 8 | (r2 & 0x0F) << 4;
    }

    return value;
}