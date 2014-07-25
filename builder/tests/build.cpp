#include <gtest/gtest.h>

// Functions from test.meta
extern "C" {

int constFoo();

int poly(int x);

int dist(int x, int y);

int distCall(int x);

}

// Functions with the same body as above funcs
namespace local {

int poly(int x)
{
    int common = x + 3;
    return 5*common*common - 6*common + 3;
}

int dist(int x, int y)
{
    return x*x + y*y;
}

int distCall(int x)
{
    return dist(x, x);
}

}

TEST(BuilderTests, constFunc)
{
    ASSERT_EQ(constFoo(), 5);
}

TEST(BuilderTests, oneArg)
{
    for (int i = -50; i < 50; ++i)
        ASSERT_EQ(poly(i), local::poly(i));
}

TEST(BuilderTests, twoArgs)
{
    for (int x = -5; x < 5; ++x) {
        for (int y = -5; y < 5; ++y)
            ASSERT_EQ(dist(x, y), local::dist(x, y));
    }
}

TEST(BuilderTests, functionWithCall)
{
    for (int x = -50; x < 50; ++x)
        ASSERT_EQ(distCall(x), local::distCall(x));
}
