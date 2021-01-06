/*
 * disassembler.cpp
 * Chip8 Disassembler main class
 */

#include <c8dasm/disassembler.h>

/*
 * helper functions
 */

template<typename ... Args>
void Disassembler::parseOpcode(std::string pc, std::string opcode, const std::string& format, Args ... args) {
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if (size <= 0)
        throw std::runtime_error( "Error during formatting." );
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf(buf.get(), size, format.c_str(), args ... );
    std::string newop = std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside

    size_t found  = newop.find(' ');
    QString op    = QString::fromStdString(newop.substr(0, found));
    QString arg   = QString::fromStdString(newop.substr(found+1, newop.size()));

    program.push_back({QString::fromStdString(pc), QString::fromStdString(opcode), op, arg});
}

// opcode to string transformation
// return the string representation of an opcode
void Disassembler::storeOpcode(uint16_t pc, uint16_t opcode)
{
    // decode the opcode
    auto addr = (opcode & 0x0fff);
    auto x = (opcode & 0x0f00) >> 8;
    auto y = (opcode & 0x00f0) >> 4;
    auto value = (opcode & 0x00ff);
    auto group = (opcode & 0xF000);

    // retrieve the instruction from the opcode value
    switch(group)
    {
        case 0x0000:
            if( opcode == 0x00E0) {
//                return string("CLS");
                program.push_back({QString::fromStdString(std::to_string(pc)), QString::fromStdString(std::to_string(opcode)), "CLS"});
            }

            if( opcode == 0x00EE)
                program.push_back({QString::fromStdString(std::to_string(pc)), QString::fromStdString(std::to_string(opcode)), "RETURN"});

            break;

        case 0x1000:    // JP addr
            parseOpcode(std::to_string(pc), std::to_string(opcode), "JP L%003X", addr);
            break;

        case 0x2000:    // CALL addr
            parseOpcode(std::to_string(pc), std::to_string(opcode), "CALL L%003X", addr);
            break;

        case 0x3000:    // SE Vx, value
            parseOpcode(std::to_string(pc), std::to_string(opcode), "SE V%X, #%02X", x, value);
            break;

        case 0x4000:    // SNE Vx, value
            parseOpcode(std::to_string(pc), std::to_string(opcode), "SNE V%X, #%02X", x, value);
            break;

        case 0x5000:    // SE Vx, Vy
            parseOpcode(std::to_string(pc), std::to_string(opcode), "SE V%X, V%X", x, y);
            break;

        case 0x6000:    // LD Vx, value
            parseOpcode(std::to_string(pc), std::to_string(opcode), "LD V%X, #%02X", x, value);
            break;

        case 0x7000:    // ADD Vx, value
            parseOpcode(std::to_string(pc), std::to_string(opcode), "ADD V%X, #%02X", x, value);
            break;

        case 0x8000:
            switch(opcode & 0x000F)
            {
                case 0x0000:    // LD Vx, Vy
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "LD V%X, V%X", x, y);
                    break;
                case 0x0001:    // OR Vx, Vy
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "OR V%X, V%X", x, y);
                    break;
                case 0x0002:    // AND Vx, Vy
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "AND V%X, V%X", x, y);
                    break;
                case 0x0003:    // XOR Vx, Vy
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "XOR V%X, V%X", x, y);
                    break;
                case 0x0004:    // ADC Vx, Vy
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "ADD V%X, V%X", x, y);
                    break;
                case 0x0005:    // SBC Vx, Vy
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "SUB  V%X, V%X", x, y);
                    break;
                case 0x0006:    // SHR Vx, 1
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "SHR V%X, #1", x);
                    break;
                case 0x0007:    // SUBN Vx, Vy
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "SUBN V%X, V%X", x, y);
                    break;
                case 0x000E:    // SHL Vx, 1
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "SHL V%X, #1", x);
                    break;
            }
            break;

        case 0x9000:    // SNE Vx, Vy
            parseOpcode(std::to_string(pc), std::to_string(opcode), "SNE V%X, V%X", x, y);
            break;

        case 0xA000:    // LD I, addr
            parseOpcode(std::to_string(pc), std::to_string(opcode), "LD I, L%003X", addr);
            break;

        case 0xB000:    // JP V0, addr
            parseOpcode(std::to_string(pc), std::to_string(opcode), "JP V0, L%003X", addr);
            break;

        case 0xC000:    // RND Vx, value
            parseOpcode(std::to_string(pc), std::to_string(opcode), "RND V%X, #%02X", x, value);
            break;

        case 0xD000:    // DRW Vx, Vy, n
            parseOpcode(std::to_string(pc), std::to_string(opcode), "DRW V%X, V%X, #%02X", x, y, value & 0x000F);
            break;

        case 0xE000:
            // SKP Vx
            if( (opcode & 0x00FF) == 0x009E)
                parseOpcode(std::to_string(pc), std::to_string(opcode), "SKP V%X", x);

            // SKNP Vx
            if( (opcode & 0x00FF) == 0x00A1)
                parseOpcode(std::to_string(pc), std::to_string(opcode), "SKNP V%X", x);
            break;

        case 0xF000:
            switch(opcode & 0x00FF)
            {
                case 0x0007:    // LD Vx, DT
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "LD V%X, DT", x);
                    break;
                case 0x000A:    // LD Vx, K
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "LD V%X, K", x);
                    break;
                case 0x0015:    // LD DT, Vx
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "LD DT, V%X", x);
                    break;
                case 0x0018:    // LD ST, Vx
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "LD ST, V%X", x);
                    break;
                case 0x001E:    // ADD I, Vx
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "ADD I, V%X", x);
                    break;
                case 0x0029:    // LD F, Vx
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "LD F, V%X", x);
                    break;
                case 0x0033:    // LD B, Vx
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "LD B, V%X", x);
                    break;
                case 0x0055:    // LD [I], Vx
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "LD [I], V%X", x);
                    break;
                case 0x0065:    // LD Vx, [I]
                    parseOpcode(std::to_string(pc), std::to_string(opcode), "LD V%X, [I]", x);
                    break;
            }
            break;
    }
}

/*
 * class methods
 */

// constructor
Disassembler::Disassembler()
{ }

// destructor
Disassembler::~Disassembler()
{ }

// return the next opcode pointed by the PC
uint16_t Disassembler::next( )
{
    // don't forget to remove the offset
    return ((rom_[PC_ - ROM_CODE_BEGIN] << 8) | rom_[PC_ - ROM_CODE_BEGIN + 1]);
}

// discover the code
void Disassembler::discover()
{
    // queue to register all the code segments to identify
    std::queue<uint16_t> segments;

    // push the code begin segment in the queue
    segments.push(ROM_CODE_BEGIN);

    // mainloop
    while( !segments.empty() )
    {
        // retrieve the next element from the queue
        PC_ = segments.front();
        segments.pop();

        // walk through this segment of code
        while( (PC_ < max_address_) && (codemap_.find(PC_) == codemap_.end()))
        {
            // retrieve the next opcode
            uint16_t opcode = next();

            // mark this address as code
            codemap_.insert(PC_);

            // move the PC forward
            PC_ += 2;

            /*-------------------
             * analyze the opcode
             *-------------------*/

            // this is a RET so we stop here
            if( opcode == 0x00EE )
                break;

            // these opcodes move the PC_ and branch out the code to a different segment
            switch(opcode & 0xF000)
            {
                case 0x1000:    // JUMP
                    PC_ = opcode & 0x0FFF;      // unconditional jump
                    labels_.insert(PC_);        // record the destination label
                    break;

                case 0x2000:    // CALL
                    segments.push(PC_);         // add a new segment to check
                    PC_ = opcode & 0x0FFF;
                    labels_.insert(PC_);        // record the destination label
                    break;

                case 0x3000:    // SE
                case 0x4000:    // SNE
                case 0x5000:    // SE
                case 0x9000:    // SNE
                    segments.push(PC_ + 2);     // add a new segment to check
                    break;

                case 0xE000:    // SKP & SKNP
                    switch(opcode & 0xF0FF)
                    {
                        case 0xE09E:    // SKP
                        case 0xE0A1:    // SKNP
                            segments.push(PC_ + 2);
                            break;
                    }
                    break;

                // this is to record the address for the Index register
                case 0xA000:    // LD I, addr
                    labels_.insert(opcode & 0x0FFF);
                    break;

                // this opcode is very problematic for the disassembler
                // so we just fail here.
                case 0xB000:
                    throw std::string("Encounter instruction 'JP V0, addr'. Unable to disassemble the code.");
            }
        }
    }
}

// render the code
void Disassembler::render() {
    std::vector<std::string> vecbuf;
    PC_ = ROM_CODE_BEGIN;

    // loop through the whole ROM
    while (PC_ < max_address_) {
        // check if this address is in the labels list
        if (labels_.find(PC_) != labels_.end())
            parseOpcode(std::to_string(PC_), "label", "L%003X", PC_);

        // process data block
        if (codemap_.find(PC_) == codemap_.end()) {

            // format the data
            parseOpcode(std::to_string(PC_), std::to_string(rom_[PC_ - ROM_CODE_BEGIN] & 0xFFFF), "DB #%02X", rom_[PC_ - ROM_CODE_BEGIN] & 0xFFFF);

            // for data, we move the PC to the next byte
            PC_ += 1;
            continue;
        }

        // parse and store opcode
        uint16_t opcode = next();
        storeOpcode(PC_, opcode);

        // move PC to the next opcode
        PC_ += 2;
    }
}
