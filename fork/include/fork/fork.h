#pragma once 

#include <unistd.h>

//=========================================================

static const __useconds_t Usec_per_digit = 500;

//=========================================================

int fork_sort(const int argc, const char** argv);

