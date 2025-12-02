#include <stdint.h>

typedef struct otTimestamp
{
    uint64_t mSeconds;
    uint16_t mTicks;
    bool     mAuthoritative;
} otTimestamp;

void test() {
    otTimestamp t1 = {0, 0, false};
    otTimestamp t2 = {0, 0, false};
    if (t1 == t2) {
        return;
    }
}
