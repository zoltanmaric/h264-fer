#pragma once

#define MAX_LEVELCODE_VALUE 5055+1
#define MAX_SUFFIX_VALUE 4095+1

void generate_residual_level_tables();

extern int levelcode_to_outputstream[MAX_LEVELCODE_VALUE][7][4];
extern int inputstream_to_levelcode[16][7][MAX_SUFFIX_VALUE];