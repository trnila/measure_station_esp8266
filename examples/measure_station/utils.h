#pragma once
#include <stdio.h>

#define panic(...) printf(__VA_ARGS__); for(;;);
