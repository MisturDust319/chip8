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
    // 16 bit index register
    // used as index for memory addresses
    uint16_t index{};
    // program counter register
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
};

#endif