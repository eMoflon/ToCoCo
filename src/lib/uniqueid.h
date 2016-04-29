#ifndef __UNIQUEID_H_
#define __UNIQUEID_H_

#include <stdint.h>

/**
 * Assigns new uniqueid
 *
 * Handling a consistent non-overlapping uniqueid assignment
 * is a complex task with many submodules. For this reason a
 * unique id for any module or operation can be assigned
 * by this authority.
 */
uint8_t uniqueid_assign();

#endif /* __UNIQUEID_H_ */
