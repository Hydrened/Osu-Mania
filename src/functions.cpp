#include "functions.h"

float lerp(float min, float max, float blend) {
    return min + blend * (max - min);
}
