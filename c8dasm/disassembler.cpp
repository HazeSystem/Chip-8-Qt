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
void Disassembler::storeOpcode(uint16_t pc, uint16_t opcode) {
    // decode the opcode
    auto addr = (opcode & 0x0fff);
    auto x = (opcode & 0x0f00) >> 8;
    auto y = (opcode & 0x00f0) >> 4;
    auto value = (opcode & 0x00ff);
    auto group = (opcode & 0xF000);

    // retrieve the instruction from the opcode value
    switch (group) {
        case 0x0000:
            switch (opcode & 0x00FF) {
            case 0x0000:    // NOP
                program.push_back({QString::fromStdString(std::to_string(pc)), QString::fromStdString(std::to_string(opcode)), "NOP", ""});
                break;
            case 0x00E0:    // CLS
                program.push_back({QString::fromStdString(std::to_string(pc)), QString::fromStdString(std::to_string(opcode)), "CLS", ""});
                break;
            case 0x00EE:   // RET
                program.push_back({QString::fromStdString(std::to_string(pc)), QString::fromStdString(std::to_string(opcode)), "RET", ""});
                break;
        default:
            parseOpcode(std::to_string(pc), std::to_string(opcode), "DB %004X", opcode);
            break;
        }
        break;

        case 0x1000:    // JP addr
            parseOpcode(std::to_string(pc), std::to_string(opcode), "JP %003X", addr);
            break;

        case 0x2000:    // CALL addr
            parseOpcode(std::to_string(pc), std::to_string(opcode), "CALL %003X", addr);
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
            default:
                parseOpcode(std::to_string(pc), std::to_string(opcode), "DB %004X", opcode);
                break;
            }
            break;

        case 0x9000:    // SNE Vx, Vy
            parseOpcode(std::to_string(pc), std::to_string(opcode), "SNE V%X, V%X", x, y);
            break;

        case 0xA000:    // LD I, addr
            parseOpcode(std::to_string(pc), std::to_string(opcode), "LD I, %003X", addr);
            break;

        case 0xB000:    // JP V0, addr
            parseOpcode(std::to_string(pc), std::to_string(opcode), "JP V0, %003X", addr);
            break;

        case 0xC000:    // RND Vx, value
            parseOpcode(std::to_string(pc), std::to_string(opcode), "RND V%X, #%02X", x, value);
            break;

        case 0xD000:    // DRW Vx, Vy, n
            parseOpcode(std::to_string(pc), std::to_string(opcode), "DRW V%X, V%X, #%02X", x, y, value & 0x000F);
            break;

        case 0xE000:
            switch (opcode & 0x00FF) {
            // SKP Vx
            case 0x009E:
                parseOpcode(std::to_string(pc), std::to_string(opcode), "SKP V%X", x);

            // SKNP Vx
            case 0x00A1:
                parseOpcode(std::to_string(pc), std::to_string(opcode), "SKNP V%X", x);
                break;

            default:
                parseOpcode(std::to_string(pc), std::to_string(opcode), "DB %004X", opcode);
                break;
            }

        case 0xF000:
            switch (opcode & 0x00FF)
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
            default:
                parseOpcode(std::to_string(pc), std::to_string(opcode), "DB %004X", opcode);
                break;
            }
//    default:
//        parseOpcode(std::to_string(pc), std::to_string(opcode), "DB %004X", opcode);
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
    return ((rom_[PC_] << 8) | rom_[PC_ + 1]);
}

// render the code
void Disassembler::render() {
    std::vector<std::string> vecbuf;
    PC_ = 0;

    // loop through the whole ROM
    while (PC_ < max_address_) {
        // parse and store opcode
        uint16_t opcode = next();
        storeOpcode(PC_, opcode);

        // move PC to the next opcode
        PC_ += 2;
    }
}
