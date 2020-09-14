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
    // this is seeded with
    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte;

    // CONSTRUCTORS
    Chip8();

    // METHODS
    void loadROM(char const* filename);
};

#endif