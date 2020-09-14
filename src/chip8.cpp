#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>

#include "chip8.h"

// ROM DATA
// the ROM is loaded into memory starting at memory address 0x200
const unsigned int START_ADDRESS = 0x200;
// FONT DATA
// the font is stored in a specific range of memory
// it starts at address 0x50
const unsigned int FONTSET_START_ADDRESS = 0x50;
// each character in the font is 5B
// there are 16 chars
// for a reserved range of 80 chars:
const unsigned int FONTSET_SIZE = 80;
// the actual font 
uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8()
    : randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
    // initialize the PC
    // it must point to the starting range for the ROM memory space
    // that is, 0x200
    pc = START_ADDRESS;

    // load the font into memory
    for(unsigned int i = 0; i < FONTSET_SIZE; ++i)
    {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    randByte = std::uniform_int_distribution<uint8_t>(0, 255);
}

void Chip8::loadROM(char const* filename)
{
    // open a filestream of the ROM binary and move the pointer to the end
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open())
    {
        // get file size, allocate buffer
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        // fill the buffer
        // first go to the befinning of the file
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        // load the ROM in the Chip8's mem, starting at 0x200
        for(long i = 0; i < size; ++i)
        {
            memory[START_ADDRESS + i] = buffer[i];
        }

        // delete the buffer
        delete[] buffer;
    }
}