#pragma once
#include "stdafx.h"

extern FILE *yuvoutput;
extern FILE *yuvinput;

void writeToPPM();
void writeToY4M();
void writeToYUV();

int readFromY4M();
void loadY4MHeader();