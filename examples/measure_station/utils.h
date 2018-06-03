#pragma once
#include <stdio.h>
#include <string.h>

#define panic(...) printf(__VA_ARGS__); for(;;);

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define VA_ARGS(...) , ##__VA_ARGS__
#define debug(fmt, ...) printf("%s:%d " fmt "\n", __FILENAME__, __LINE__ VA_ARGS(__VA_ARGS__));
