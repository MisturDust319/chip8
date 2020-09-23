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

    // CONSTRUCTORS
    Chip8();

    // METHODS
    void loadROM(char const* filename);

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
};

#endif