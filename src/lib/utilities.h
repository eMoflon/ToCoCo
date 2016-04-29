#ifndef __MISC_H_
#define __MISC_H_

#ifndef UTILITIES_CONTIKIRANDOM_STATICSEED
#define UTILITIES_CONTIKIRANDOM_STATICSEED 123456
#endif

void contikirandom_init();

uint32_t random(uint32_t min, uint32_t max);

#endif /* __MISC_H_ */
