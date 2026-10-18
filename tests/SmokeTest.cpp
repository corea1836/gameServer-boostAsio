#include <gtest/gtest.h>

#include "RecvBuffer.h"

// 테스트 빌드가 serverCore와 링크되고 ctest로 돌아가는지만 확인한다
TEST(Smoke, LinksServerCore)
{
    RecvBuffer buffer(10);

    EXPECT_TRUE(buffer.OnWrite(4));
    EXPECT_EQ(buffer.GetDataSize(), 4);
}
