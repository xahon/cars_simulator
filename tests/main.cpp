#include <vector>
#include <gtest/gtest.h>
#include "structs.hpp"

TEST(Rect, RectIntersections)
{
    sRect r1(0, 0, 4, 4);
    sRect r2(0, -2, 1, 2);
    sRect r3(0, -3, 1, 2);

    ASSERT_TRUE(r1.touches(r2));
    ASSERT_FALSE(r1.overlaps(r3));

    sRect r4(1, 0, 2, 1);
    sRect r5(2, -1, 1, 2);
    ASSERT_TRUE(r4.overlaps(r5));

    sRect r6(3, 1, 1, 2);
    sRect r7(3, 2, 2, 1);
    ASSERT_TRUE(r6.overlaps(r7));

    sRect r8(0, 1, 2, 1);
    sRect r9(1, 3, 1, 2);
    ASSERT_FALSE(r8.overlaps(r9));

    sRect r10(7, 12, 4, 2);
    sRect r11(10, 8, 2, 4);
    ASSERT_TRUE(r10.touches(r11));
    ASSERT_FALSE(r10.overlaps(r11));
}