#include <iostream>
#include <unistd.h>
#include <vector>
#include <cmath>
#include <wait.h>
#include <fstream>
#include <chrono>

#define SEED_SPACE ((1LLU<<28U)-1)
#define TRESHOLD 16

#include "main.h"
#include "Random.hpp"

void crack(int thread_id,int processes){
    for (unsigned long long seed=SEED_SPACE/processes*thread_id;seed<SEED_SPACE/processes*(thread_id+1);seed++){
        if(bruteforceSeed(seed)>TRESHOLD){
            std::cout<<seed<<std::endl;
        }
    }
}


int bruteforceSeed(unsigned long long seed) {
    int floorLevel = 62;
    int attemptsCount = 10;

    int heightMap[32 * 32];

    std::fill_n(heightMap,32*32,floorLevel);

    int currentHighestPos = 0;

    for(int i = 0; i < attemptsCount; i++) {
        seed=(seed * 0x5DEECE66DLLU + 0xBLLU)& ((1LLU << 48U) - 1);
        int initialPosX = (seed>> 44u) + 8;
        seed=(seed * 0x5DEECE66DLLU + 0xBLLU)& ((1LLU << 48U) - 1);
        int initialPosZ = (seed>> 44u) + 8;
        int terrainHeight = (heightMap[initialPosX + initialPosZ * 32] + 1) * 2;

        int initialPosY = nextIntUnknown(&seed, terrainHeight);
        generateCactus(&seed, heightMap, initialPosX, initialPosY, initialPosZ, &currentHighestPos);
    }

    return heightMap[currentHighestPos] - floorLevel;
}

void generateCactus(unsigned long long* seed, int* heightMap, int initialPosX, int initialPosY, int initialPosZ, int* currentHighestPos) {
    for(int i = 0; i < 10; i++) {
        int posX = initialPosX + next(seed, 3) - next(seed, 3);
        int posY = initialPosY + next(seed, 2) - next(seed, 2);
        int posZ = initialPosZ + next(seed, 3) - next(seed, 3);

        if(!isAir(heightMap, posX, posY, posZ))continue;

        int offset = 1 + nextIntUnknown(seed, nextInt(seed, 3) + 1);
        int posMap = posX + posZ * 32;

        for(int j = 0; j < offset; j++) {
            if(isAir(heightMap, posX, posY + j - 1, posZ))continue;
            if(!isAir(heightMap, posX + 1, posY + j, posZ))continue;
            if(!isAir(heightMap, posX - 1, posY + j, posZ))continue;
            if(!isAir(heightMap, posX, posY + j, posZ + 1))continue;
            if(!isAir(heightMap, posX, posY + j, posZ - 1))continue;

            heightMap[posMap]++;

            if(heightMap[*currentHighestPos] < heightMap[posMap]) {
                *currentHighestPos = posMap;
            }
        }
    }
}

bool isAir(const int* heightMap, int x, int y, int z) {
    int height = heightMap[x + z * 32];
    return y > height;
}

int next(unsigned long long* seed, int bits) {
    *seed = (*seed * 0x5DEECE66DLLU + 0xBU) & ((1LLU << 48U) - 1);
    return *seed >> (48U - bits);
}

int nextInt(unsigned long long* seed, int bound) {
    int bits, value;
    do {
        *seed = (*seed * 0x5DEECE66DLLU + 0xBU) & ((1LLU << 48U)  - 1);
        bits = *seed >> 17U;
        value = bits % bound;
    } while(bits - value + (bound - 1) < 0);
    return value;
}

int nextIntUnknown(unsigned long long* seed, unsigned int bound) {
    if((bound & -bound) == bound) {
        *seed = (*seed * 0x5DEECE66DLLU + 0xBU) & ((1LLU << 48U) - 1);
        return (int)((bound * (*seed >> 17U)) >> 31U);
    }

    int bits, value;
    do {
        *seed = (*seed * 0x5DEECE66DLLU + 0xBU) & ((1LLU << 48U) - 1);
        bits = *seed >> 17U;
        value = bits % bound;
    } while(bits - value + (bound - 1) < 0);
    return value;
}
void signalHandler(int signum) {
    std::cout << "received SIGTERM " << signum << " on group pid : " << getgid() << std::endl;
    kill(-getgid(), SIGKILL);
}

int return_id(std::vector<pid_t> &array) {
    int retval = 0;
    for (int i = 0; i < (int) array.size(); i++) {
        retval += (array[i] > 0 ? 1 : 0) * (int) pow(2, i);
    }
    return retval;
}

void handler(int processes, pid_t pidMain) {
    std::vector<pid_t> pids;
    switch (processes) {
        case 2: {
            pid_t pid1 = fork();
            pids.push_back(pid1);
            break;
        }
        case 4: {
            pid_t pid1 = fork();
            pid_t pid2 = fork();
            pids.push_back(pid1);
            pids.push_back(pid2);
            break;
        }
        case 8: {
            pid_t pid1 = fork();
            pid_t pid2 = fork();
            pid_t pid3 = fork();
            pids.push_back(pid1);
            pids.push_back(pid2);
            pids.push_back(pid3);
            break;
        }
        default: {
            processes = 1;
        }
    }
    setpgid(getpid(), pidMain);
    crack(return_id(pids),processes);
}

int main() {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    int processes = 8;
    pid_t pidMain = fork();
    if (pidMain == 0) {
        handler(processes, getppid());
    } else {
        setpgid(getpid(), getpid());
        signal(SIGTERM, signalHandler);
        while (waitpid(0, nullptr, WCONTINUED) != -1) {}
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(
            t2 - t1);
    std::cout<<"Done. "+std::to_string(time_span.count())+" seconds." <<std::endl;

}