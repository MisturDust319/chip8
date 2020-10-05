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

uint8_t VIDEO_WIDTH = 32;
uint8_t VIDEO_HEIGHT = 64;

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
    registers[Vx] |= registers[Vy];
}

void Chip8::OP_8xy2()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    // get Vy, which is store in the 4th nibble
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // set Vx = Vx & Vy
    registers[Vx] &= registers[Vy];
}

void Chip8::OP_8xy3()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    // get Vy, which is store in the 4th nibble
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // set Vx = Vx ^ Vy
    registers[Vx] ^= registers[Vy];
}

void Chip8::OP_8xy4()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // get the sum of Vx and Vy
    uint16_t sum = registers[Vx] + registers[Vy];

    // if the sum is greater than something that can fit in one bit...
    if (sum > 255u)
    {
        // set the carry bit as 1 so it may be used later
        registers[0xF] = 1; // 0xF is the address of VF, the carry register
    }
    else
    {
        // otherwise, set the carry to 0
        registers[0xF] = 0;
    }

    // set Vx with the sum
    registers[Vx] = sum & 0xFFu;
    // masking sum with FF ensures sum can't surpass 255
}

void Chip8::OP_8xy5()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // if Vx > Vy, then set VF to be the NEGATION of the carry value
    if (registers[Vx] > registers[Vy])
    {
        // so if we DON'T carry,
        // set carry to 1
        registers[0xF] = 1;
    }
    // otherwise set the carry bit to 0
    else
    {
        registers[0xF] = 0;
    }

    // then set the final register value
    registers[Vx] -= registers[Vy];
}

void Chip8::OP_8xy6()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // save LSB in VF
    registers[0xF] = (registers[Vx] & 0x1u);

    // right shift Vx
    registers[Vx] >>= 1;
}

void Chip8::OP_8xy7() {
    // get Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    // get Vy
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    // if Vy > Vx, then set VF to be the NEGATION of the carry value
    if (registers[Vy] > registers[Vx])
    {
        // so if we DON'T carry,
        // set carry to 1
        registers[0xF] = 1;
    }
    // otherwise set the carry bit to 0
    else
    {
        registers[0xF] = 0;
    }

    // then set the final register value
    registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8xyE()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // save MSB in VF
    registers[0xF] = (registers[Vx] & 0x80u);

    // left shift Vx
    registers[Vx] <<= 1;
}

void Chip8::OP_9xy0()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // get Vy
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    // if Vx != Vy, skip the next instruction
    if (registers[Vx] != registers[Vy])
    {
        // this is done by incrementing the PC by 2
        // the number of bytes an instruction occupies in memory
        pc += 2;
    }
}

void Chip8::OP_Annn()
{
    // get the memory address from the opcode
    uint16_t address = opcode & 0x0FFFu;
    // store it in index, the I register
    index = address;
}

void Chip8::OP_Bnnn()
{
    // grab address nnn
    uint16_t address = opcode & 0x0FFFu;
    // increment the PC by V0 + address
    pc = registers[0] + address;
}

void Chip8::OP_Cxkk()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // get kk
    uint8_t kk = opcode & 0x00FFu;

    // set Vx to a random byte of max size kk
    registers[Vx] = randByte(randGen) & kk;
}

void Chip8::OP_Dxyn()
{
    // the draw function 
    
    // Get Vx, Vy
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    uint8_t Vy = (opcode & 0x00F0) >> 4u;
    // The final nibble is the height
    uint8_t n = opcode & 0x000Fu;

    // we wrap the sprite if it extends beyond the screen boundaries
    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    // by default, set the collision register to 0
    registers[0xF] = 0;

    for (unsigned int row = 0; row < n; ++row)
    {
        // grab the current byte from memory
        // we start at the position in memory pointed to by the I register
        // offset by the current row
        uint8_t spriteByte = memory[index + row];
        
        // now we iterate over the sprite byte
        // in chip 8, all sprites are 8 pixels wide
        // which is why they fit in a single byte
        for (unsigned int col = 0; col < 8; ++col)
        {
            // isolate the pixel for the current row and column
            // note how we do this by shifting the masking constant
            // NOT the sprite byte
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            // calculate the screen pixel we will display the sprite pixel on
            // NOTE: the screen pixel is a pointer, hence the dereference operator
            uint32_t* screenPixel = &video[(xPos + col) + (yPos + row) * VIDEO_WIDTH];
            
            // if the sprite pixel is to be turned on,
            // we need to turn it off
            // we may need to set the collision register
            if (spritePixel)
            {
                // check if the screen pixel is on or off
                // remember that we use 0xFFFFFFFF for on, and 0x00000000 for off
                // this is to simplify working with SDL, the rendering API
                if (*screenPixel == 0xFFFFFFFF)
                {
                    // if there is a pixel, then we have a collision
                    // and we should set the collision bit to 1
                    registers[0xF] = 1;
                }

                // toggle the sprite pixel with XOR
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void Chip8::OP_Ex9E()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // get the value of Vx
    // this will be used to index the keypad
    uint8_t key = registers[Vx];

    // check the keypad
    // the corresponding key should be pressed
    if (keypad[key])
    {
        pc += 2;
    }
}

void Chip8::OP_ExA1()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // get the value of Vx
    // this will be used to index the keypad
    uint8_t key = registers[Vx];

    // check the keypad
    // the corresponding key should NOT be pressed
    if (!keypad[key])
    {
        pc += 2;
    }
}

void Chip8::OP_Fx07()
{
    // get Vx
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // set it to the value of the delay timer    
    registers[Vx] = delayTimer;
}

void Chip8::OP_Fx0A()
{
    // this implements the stalls the program until a key is pressed
    // by repeatedly decrementing the PC by 2 when a keypad value is detected
    // this repeatedly reruns this operation until input is given
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // this else/if chain checks if they key is pressed
    // if it is, the corresponding key number is stored in Vx
    // otherwise, this instruction is rerun until input is given
    if (keypad[0])
    {
        registers[Vx] = 0;
    }
    else if (keypad[1])
    {
        registers[Vx] = 1;
    }
    else if (keypad[2])
    {
        registers[Vx] = 2;
    }
    else if (keypad[3])
    {
        registers[Vx] = 3;
    }
    else if (keypad[4])
    {
        registers[Vx] = 4;
    }
    else if (keypad[5])
    {
        registers[Vx] = 5;
    }
    else if (keypad[6])
    {
        registers[Vx] = 6;
    }
    else if (keypad[7])
    {
        registers[Vx] = 7;
    }
    else if (keypad[8])
    {
        registers[Vx] = 8;
    }
    else if (keypad[9])
    {
        registers[Vx] = 9;
    }
    else if (keypad[10])
    {
        registers[Vx] = 10;
    }
    else if (keypad[11])
    {
        registers[Vx] = 11;
    }
    else if (keypad[12])
    {
        registers[Vx] = 12;
    }
    else if (keypad[13])
    {
        registers[Vx] = 13;
    }
    else if (keypad[14])
    {
        registers[Vx] = 14;
    }
    else if (keypad[15])
    {
        registers[Vx] = 15;
    }
    else
    {
        pc -= 2;
    }
}

void Chip8::OP_Fx15()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    delayTimer = registers[Vx];
}

void Chip8::OP_Fx18()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    index += registers[Vx];
}

void Chip8::OP_Fx29()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // get the number stored in Vx
    uint8_t digit = registers[Vx];

    // All font sprites are stored in a dedicated range in memory
    // and each sprite is 5 bytes each
    // so, to get a char, we multiply the digit by 5,
    // and offset it by the start address
    index = FONTSET_START_ADDRESS + (5 * digit);
}

void Chip8::OP_Fx33()
{
    // this stores a BCD (Binary Coded Decimal) representation in memory
    // at I, I+1, and I+2
    // for reference: https://en.wikipedia.org/wiki/Binary-coded_decimal
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = registers[Vx];

    // Vx is an 8 bit value of maximum value of 255
    // this means all we need to do to get the BCD value
    // is to separate the values in the 1s, 10s, and 100s column
    // and store them in memory
    // with the 100s going to I, 10s to I+1, and 1s to I+1

    // get the 1s place
    memory[index + 2] = value % 10;
    // dividing by ten is a quick way to remove the least sig digit
    value /= 10;

    // 10s place
    memory[index + 1] = value % 10;

    // 100s place
    memory[index] = value % 10;
}