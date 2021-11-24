#include <assert.h>
#include <iostream>
#include <cstdint>
#include <string.h>
#include <stdio.h>
#include <bitset>

int32_t log_roundup(int32_t size) {
    int32_t log = 0;
    bool deductOne = false;
    if (size <= 0) {
        return 0;
    }
    if ((size & (size-1)) == 0) {
        deductOne = true;
    }
    while (size) {
        size /= 2;
        ++log;
    }
    if (deductOne) {
        --log;
    }
    return log;
};

class bitBuddy {
    public:
        bitBuddy(int32_t leafBits) : numLeafBits(leafBits), externalMap(false) {
            levels = log_roundup(leafBits);
            //std::cout << "Levels: " << levels << std::endl;
            int32_t numChars = leafBits / 8;
            if (numChars * 8 < leafBits) {
                ++numChars;
            }
            //std::cout << "Leaf bytes: " << numChars << std::endl;
            byteAdjLeafSize = numChars;
            int32_t metaBits = (1 << levels) / 8;
            if (metaBits * 8 < (1 << levels)) {
                ++metaBits;
            }
            //std::cout << "Meta bytes: " << metaBits << std::endl;
            numChars += metaBits;
            bitBuddyMap = new uint8_t[numChars];
            memset(bitBuddyMap, 0, numChars);
        }
        bitBuddy(int32_t leafBits, uint8_t *bitMap, int32_t byteSize) : externalMap(true), numLeafBits(leafBits), bitBuddyMap(bitMap) {
            levels = log_roundup(leafBits);
            int32_t numChars = leafBits / 8;
            if (numChars * 8 < leafBits) {
                ++numChars;
            }
            byteAdjLeafSize = numChars;
            int32_t metaBits = (1 << levels) / 8;
            if (metaBits * 8 < (1 << levels)) {
                ++metaBits;
            }
            numChars += metaBits;
            if (byteSize >= numChars) {
                memset(bitBuddyMap, 0, numChars);
            } else {
                bitBuddyMap = new uint8_t[numChars];
                memset(bitBuddyMap, 0, numChars);
                externalMap = false;
            }
        }
        ~bitBuddy() {
            if (externalMap) {
                return;
            }
            delete[] bitBuddyMap;
        }
        bool setBit(int32_t pos) {
            if (pos > numLeafBits) {
                return false;
            }
            int32_t bitValue = 1 << (pos % 8);
            if (bitBuddyMap[pos/8] & bitValue) {
                return false;
            } else {
                bitBuddyMap[pos/8] |= bitValue;
                if (((pos) & 0x01 && (bitBuddyMap[pos/8] & (bitValue >> 1))) ||
                        ((((pos) & 0x01) == 0) && (bitBuddyMap[pos/8] & (bitValue << 1)))) {
                    propagateMetaBitSet(pos/2, 1, byteAdjLeafSize*8);
                }
                return true;
            }
        }
        bool getBit(int32_t pos) {
            if (pos > numLeafBits) {
                return false;
            }
            int32_t bitValue = 1 << (pos % 8);
            if (bitBuddyMap[pos/8] & bitValue) {
                return true;
            }
            return false;
        }
        bool getBitAtLevel(int32_t pos, int32_t level) {
            if (level == 0) {
                return getBit(pos);
            }
            if (level > levels) {
                return false;
            }
            int32_t bitsAtLevel = (1 << (levels - level)); // number of bits in this level
            if (pos > bitsAtLevel) {
                return false;
            }
            int32_t levelBitOffset = byteAdjLeafSize*8 + ((1 << levels) - 1) - ((1 << level) - 1);
            if (getBit(levelBitOffset+pos)) {
                return true;
            }
            return false;
        }
        bool getFirstZeroBitFromLeft(int32_t &pos) {
            int32_t level = levels;
            pos = 0;

            if (getBitAtLevel(pos, level)) {
                return false;
            } else {
                while (level) {
                    --level;
                    if (getBitAtLevel(pos*2, level)) {
                        pos *= 2;
                    } else {
                        assert(getBitAtLevel(pos*2+1, level) == true);
                        pos = pos*2 +1;
                    }
                }
            }
            return true;
        }
        bool resetBit(int32_t pos) {
            if (pos > numLeafBits) {
                return false;
            }
            int32_t bitValue = 1 << (pos % 8);
            if (bitBuddyMap[pos/8] & bitValue == 0) {
                return false;
            } else {
                bitBuddyMap[pos/8] ^= bitValue;
                if ((pos & 0x01 && (bitBuddyMap[pos/8] & (bitValue >> 1))) ||
                        (((pos & 0x01) == 0) && (bitBuddyMap[pos/8] & (bitValue << 1)))) {
                    propagateMetaBitReset(pos/2, 1, byteAdjLeafSize*8);
                }
                return true;
            }
        }
        void print() {
            std::cout << "Bits on level " << 0 << ": " << numLeafBits << " equating to " << byteAdjLeafSize << " bytes " << std::endl;
            for (int32_t i = 0; i < numLeafBits; ++i) {
                if (i && (i % 8 == 0)) {
                    std::cout << " ";
                }
                if (bitBuddyMap[i/8] & (1 << (i % 8))) {
                    std::cout << "1";
                } else {
                    std::cout << "0";
                }
            }
            std::cout << std::endl;
            printLevel(1, byteAdjLeafSize*8);
        }
        void printLevel(int32_t level, int32_t bitOffset) {
            int32_t bits = (1 << (levels - level)); // number of bits in this level
            for (int32_t i = 0; i < bits; ++i) {
                if (i && (i % 8 == 0)) {
                    std::cout << " ";
                }
                if (bitBuddyMap[(i+bitOffset)/8] & (1 << ((i+bitOffset) % 8))) {
                    std::cout << "1";
                } else {
                    std::cout << "0";
                }
            }
            std::cout << std::endl;
            if (level == levels) {
                return;
            }
            printLevel(level + 1, bitOffset + bits);
        }
    private:
        bool propagateMetaBitSet(int32_t pos, int32_t level, int32_t bitOffset) {
            int32_t idx = (pos + bitOffset)/8;
            int32_t bitValue = 1 << ((pos+bitOffset) % 8);
            if (bitBuddyMap[idx] & bitValue) {
                return false;
            }
            bitBuddyMap[idx] |= bitValue;
            if (level == levels) {
                return true;
            }
            if (((pos) & 0x01 && (bitBuddyMap[idx] & (bitValue >> 1))) ||
                    ((((pos) & 0x01) == 0) && (bitBuddyMap[idx] & (bitValue << 1)))) {
                return propagateMetaBitSet(pos/2, level+1, bitOffset + (1 << (levels - level)));
            }
            return false;

        }
        bool propagateMetaBitReset(int32_t pos, int32_t level, int32_t bitOffset) {
            int32_t idx = (pos + bitOffset)/8;
            int32_t bitValue = 1 << ((pos+bitOffset) % 8);
            if (bitBuddyMap[idx] & bitValue == 0) {
                return false;
            }
            bitBuddyMap[idx] ^= bitValue;
            if (level == levels) {
                return true;
            }
            if ((pos & 0x01 && (bitBuddyMap[idx] & (bitValue >> 1))) ||
                    (((pos & 0x01) == 0) && (bitBuddyMap[idx] & (bitValue << 1)))) {
                return propagateMetaBitReset(pos/2, level+1, bitOffset + (1 << (levels - level)));
            }
            return false;
        }
        uint8_t *bitBuddyMap;
        int32_t numLeafBits;
        bool externalMap;
        int32_t levels;
        int32_t byteAdjLeafSize;

};

int main() {
    bitBuddy bb(6);
    bb.print();
    //std::cout << "Setting bit 0 ";
    //std::cout << ((bb.setBit(0) == true) ? "Success" : "Failure") << std::endl;
    //std::cout << "Set bit 0" << std::endl;
    bb.print();
    //std::cout << "Setting bit 1 ";
    //std::cout << ((bb.setBit(1) == true) ? "Success" : "Failure") << std::endl;
    //std::cout << "Set bit 1" << std::endl;
    bb.print();
    //std::cout << "Setting bit 2 ";
    //std::cout << ((bb.setBit(2) == true) ? "Success" : "Failure") << std::endl;
    //std::cout << "Set bit 2" << std::endl;
    bb.print();
    //std::cout << "Setting bit 3 ";
    //std::cout << ((bb.setBit(3) == true) ? "Success" : "Failure") << std::endl;
    //std::cout << "Set bit 3" << std::endl;
    bb.print();
    //std::cout << "Setting bit 4 ";
    //std::cout << ((bb.setBit(4) == true) ? "Success" : "Failure") << std::endl;
    //std::cout << "Set bit 4" << std::endl;
    bb.print();
    //std::cout << "Setting bit 5 ";
    //std::cout << ((bb.setBit(5) == true) ? "Success" : "Failure") << std::endl;
    //std::cout << "Set bit 5" << std::endl;
    bb.print();

    //std::cout << "Resetting bit 2 ";
    //std::cout << ((bb.resetBit(2) == true) ? "Success" : "Failure") << std::endl;
    //std::cout << "Reset bit 2" << std::endl;
    bb.print();
    //std::cout << "Resetting bit 1 ";
    //std::cout << ((bb.resetBit(1) == true) ? "Success" : "Failure") << std::endl;
    //std::cout << "Reset bit 1" << std::endl;
    bb.print();


    uint32_t leafSize = 67;
    bitBuddy test(leafSize);
    int32_t MAX_SET = 40;
    for (int i = 0; i < MAX_SET; ++i) {
        test.setBit(rand() % leafSize);
    } 
    int32_t MAX_RESET = 20;
    for (int i = 0; i < MAX_RESET; ++i) {
        test.resetBit(rand() % leafSize);
    } 
    test.print();

    return 0;
}
