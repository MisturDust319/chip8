#ifndef CHIP_8
#define CHIP_8

#include <cstdint>
#include <chrono>
#include <random>

class Chip8
{   
public:
    // CPU registers
    uint8_t registers[16]{};
    // system memory
    uint8_t memory[4096]{};
    // 16 bit index register, used as index for memory addresses
    uint16_t index{};
    // the program counter register holds the address of the next instruction in memory
    uint16_t pc{};
    // a call stack that can hold up to 16 PC values
    uint16_t stack[16]{};
    // stack pointer register
    // indexes the call stack
    uint8_t sp{};
    // a timer
    // any non zero value will be decremented at a constant rate
    uint8_t delayTimer{};
    // a buzzer
    // like the timer, but also buzzes when decrementing
    uint8_t soundTimer{};
    // an array that tracks keypresses
    // the chip8 had 16 keys
    uint8_t keypad[16]{};
    // an array representing the display
    uint32_t video[64 * 32]{};
    // value to hold current opcode
    uint16_t opcode;


    // RNG values
    // a basic RNG generator, seeded with the system clock
    std::default_random_engine  randGen;
    // a uniform int distribution will produce ints in a range [a,b]
    // we use [0,255]
    std::uniform_int_distribution<uint8_t> randByte;


    // FUNCTION TABLES
    // We maintain a handful of tables that are used to figure out which
    // opcode function to execute
    // One main table, and several smaller tables for special opcodes
    // All tables hold function pointers
    // and all are one element bigger than needed because it makes indexing easier
    // (no need to alter the opcode to access the right table)
    void (Chip8::*table [0xF + 1])(){ NULL };
    void (Chip8::*table0 [0xE + 1])(){ NULL };
    void (Chip8::*table8 [0xE + 1])(){ NULL };
    void (Chip8::*tableE [0xE + 1])(){ NULL };
    void (Chip8::*tableF [0x65 + 1])(){ NULL };


    // CONSTRUCTORS
    Chip8();


    // TABLE FUNCTIONS
    // These are used to figure out which operation to execute
    
    // For opcodes beginning with 00E
    void Table0();
    // For opcodes beginning with 8
    void Table8();
    // For opcodes beginning with E
    void TableE();
    // For opcodes
    void TableF();
    // a dummy null function to initialize the opcode function tables with


    // Load a ROM from disk into memory
    // filename: a C string representing a file name
    void loadROM(char const* filename);

    
    // Execute a single cycle of activity on the CPU
    // This includes fetching, decoding, and executing an instruction
    void Cycle();


    // OPCODES

    // CLS: Clear the screen
    void OP_00E0();
    // RET: Return from a subroutine
    void OP_00EE();
    // JP: jumps to location nnn, where nnn are the last three values of the opcode
    void OP_1nnn();
    // CALL: call a subroutine at nnn
    void OP_2nnn();
    // SE Vx, kk: skip the next instruction if Vx is equal to byte kk
    void OP_3xkk();
    // SNE Vx, kk: skip the next instruction if Vx is NOT equal to byte kk
    void OP_4xkk();
    // SE Vx, Vy: compare register Vx to register Vy
    // if they are equal, skip the next instruction
    void OP_5xy0();
    // LD Vx, kk: load the value in byte kk into Vx
    void OP_6xkk();
    // ADD Vx, kk: Add byte kk to Vx
    void OP_7xkk();
    // LD Vx, Vy: set Vx = Vy
    void OP_8xy0();
    // OR Vx, Vy: Vx = Vx | Vy
    void OP_8xy1();
    // AND Vx, Vy: Vx = Vx & Vy
    void OP_8xy2();
    // XOR Vx, Vy: Vx = Vx ^ Vy
    void OP_8xy3();
    // ADD Vx, Vy: Vx = Vx + Vy, carry is stored in VF
    void OP_8xy4();
    // SUB Vx, Vy: Vx = Vx - Vy, set VF = NOT borrow
    void OP_8xy5();
    // SHR Vx: logical shift right Vx by 1 (div by 2)
    // the least sig bit is saved in VF
    void OP_8xy6();
    // SUBN Vx, Vy: Vx = Vy - Vx, with VF set to NOT borrow
    void OP_8xy7();
    // SHL Vx {, Vy}: Left shift Vx by 1, ignoring Vy
    // the most sig bit is stored in VF
    void OP_8xyE();
    // SNE Vx, Vy: skips the next instruction if Vx != Vy
    void OP_9xy0();
    // LD I, addr: load addr into I, the index register
    void OP_Annn();
    // JP V0, addr: jump to location addr + V0
    void OP_Bnnn();
    // RND Vx, kk: Vx = random byte & byte kk
    void OP_Cxkk();
    // DRW Vx, Vy, n: draw n-byte sprite (n is analogous to the height) 
    // starting in memory location I, at position (Vx, Vy) in the screen
    // set VF = collision
    void OP_Dxyn();
    // SKP Vx: skip the next instruction
    // if the key with value Vx is pressed
    void OP_Ex9E();
    // SKNP Vx: skip the next instruction
    // if the key in register Vx is NOT pressed
    void OP_ExA1();
    // LD Vx, DT: set Vx = delay timer value
    void OP_Fx07();
    // LD Vx, K: wait for a key press
    // store the value in Vx
    void OP_Fx0A();
    // LD DT, Vx: set delay timer = Vx
    void OP_Fx15();
    // LD ST, Vx: set sound timer = Vx
    void OP_Fx18();
    // ADD I, Vx: set I = I + Vx
    void OP_Fx1E();
    // LD F, Vx: point I to the location of the sprite
    // for the digit who's value is in Vx
    void OP_Fx29();
    // LD B, Vx: store the Binary Coded Decimal representation of Vx
    // in I, I+1, and I+2
    // with I being the most significant digit
    void OP_Fx33();
    // LD [I], Vx: store registers V0 to Vx
    // in the memory location starting at I
    void OP_Fx55();
    // LD Vx, [I]: read from location [I, I + Vx] in memory
    // into Vx 
    void OP_Fx65();
};

#endif