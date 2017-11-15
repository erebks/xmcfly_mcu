/**
 * @file     debug_lib.h
 * @version  V0.1
 * @date     Oct 2014
 * @author   M. Horauer
 *
 * @brief   This library retargets the printf() function to
 *          generate output via the SWD DEBUG interface.
 *
 */
#ifndef DEBUG_LIB_H_
#define DEBUG_LIB_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <XMC4500.h>

struct __FILE {
  int handle;
};

/******************************************************** FUNCTION PROTOTYPES */
void initRetargetSwo(void);
int fgetc(FILE *f);
int fputc(int ch, FILE *f);

#endif
/** EOF */
