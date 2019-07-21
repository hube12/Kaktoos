

#ifndef KAKTOOS_MAIN_H
#define KAKTOOS_MAIN_H

int bruteforceSeed(unsigned long long seed);

void generateCactus(unsigned long long seed, int *heightMap, int initialPosX, int initialPosY, int initialPosZ,
                    int *currentHighestPos);

int nextInt(unsigned long long*seed, int bound);

int next(unsigned long long *seed, int bits);

int nextIntUnknown(unsigned long long *seed, unsigned int bound);

#endif //KAKTOOS_MAIN_H
