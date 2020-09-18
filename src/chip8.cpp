#include <cstdint>
#include <cstring>
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

    // the byte will be given a random int in the range [0,255]
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
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

void Chip8::OP_00E0()
{
    // memset will fill the video memory with 0s
    memset(video, 0, sizeof(video));
}

void Chip8::OP_00EE()
{
    // decrement the stack pointer so that it points to the last
    // PC pushed onto it
    --sp;
    // set the PC to the old PC now stored on the stack top
    pc = stack[sp];
}

void Chip8::OP_1nnn()
{
    // note that the opcode is set outside this function
    uint16_t address = opcode & 0xFFFu; // the u signifies an unsigned int
    // using the bitwise and with 0xFFF ensures nnn will never be greater than FFF

    // set the program counter to the address
    // note we don't need to save the previous PC with a jump
    pc = address;
}

void Chip8::OP_2nnn()
{
    // note that the opcode is set outside this function
    uint16_t address = opcode & 0xFFFu; // the u signifies an unsigned int
    // using the bitwise and with 0xFFF ensures nnn will never be greater than FFF

    // unlike jump, calling a subrouting stores the current PC value on the stack
    // push the current PC onto the stack, and increment the stack pointer
    stack[pc] = pc; 
    ++sp;
    pc = address;
}

void Chip8::OP_3xkk()
{
    // the data for this operation is stored in the opcode and must be extracted
    // The first four bits (nibble) of the first byte is 3
    // the second nibble is Vx
    //  it needs to be right shifted 8 times to fit into a single byte
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // the whole second byte is used to store kk
    uint8_t kk = opcode & 0x00FFu;

    // if Vx and kk are equal, skip the next instruction
    if (registers[Vx] == kk)
    {
        pc += 2;
    }
}

void Chip8::OP_4xkk()
{
    // grab Vx
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // grab kk
    uint8_t kk = opcode & 0x00FFu;

    // if Vx and kk are equal, skip the next instruction
    if (registers[Vx] != kk)
    {
        pc += 2;
    }
}

void Chip8::OP_5xy0()
{
    // the first nibble is 5
    // the second nibble is Vx, it must be shifted 8 times to fit into a byte
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // the third nibble is Vy, 
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] == registers[Vy])
    {
        pc += 2;
    }
}

void Chip8::OP_6xkk()
{
    // isolate the register from the opcode
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // grab the byte
    uint8_t kk = opcode & 0x00FFu;

    // store kk in the register
    registers[Vx] = kk;
}

void Chip8::OP_7xkk()
{
    // grab Vx
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // grab byte kk
    uint8_t kk = opcode & 0x00FFu;

    // add kk to register Vx
    registers[Vx] += kk;
}

void Chip8::OP_8xy0()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    // get Vy, which is store in the 4th nibble
    // and must be shifted to properly fit in a byte
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // set Vx = Vy
    registers[Vx] = registers[Vy];
}

void Chip8::OP_8xy1()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    // get Vy, which is store in the 4th nibble
    // and must be shifted to properly fit in a byte
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // set Vx = Vx | Vy
    registers[Vx] |= register[Vy];
}