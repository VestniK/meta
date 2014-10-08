/*
 * Meta language compiler
 * Copyright (C) 2014  Sergey Vidyuk <sir.vestnik@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gtest/gtest.h>

// Functions from test.meta
extern "C" {

int test_constFoo();

int test_poly(int x);

int test_dist(int x, int y);

int test_distCall(int x);

int test_distCallFull(int x);

int test_varsUsage(int x);

int test_assigment(int x);

bool test_less(int x, int y);

bool test_leftBound(int left, int val);

}

// Functions with the same body as above funcs
namespace local {

int poly(int x)
{
    int common = -x + 3;
    return 5*common*common - 6*common + 3;
}

int dist(int x, int y = 3)
{
    return x*x + 2*y*y;
}

int distCall(int x)
{
    return dist(x+5);
}

int distCallFull(int x)
{
    return -dist(x+5, x);
}

int varsUsage(int x)
{
    int y;
    int z = 5;
    y = +x + z;
    z = y*x;
    return x + y + z;
}

int assigment(int x)
{
    int a;
    int b;
    int c = a = b = x + 5;
    return a + b + c;
}

bool less(int x, int y)
{
    return x < y;
}

bool leftBound(int left, int val)
{
    auto lcheck = !(val < left);
    return lcheck;
}

}

TEST(BuilderTests, constFunc)
{
    ASSERT_EQ(test_constFoo(), 5);
}

TEST(BuilderTests, oneArg)
{
    for (int i = -50; i < 50; ++i)
        ASSERT_EQ(test_poly(i), local::poly(i));
}

TEST(BuilderTests, twoArgs)
{
    for (int x = -5; x < 5; ++x) {
        for (int y = -5; y < 5; ++y)
            ASSERT_EQ(test_dist(x, y), local::dist(x, y));
    }
}

TEST(BuilderTests, functionWithCall)
{
    for (int x = -50; x < 50; ++x) {
        ASSERT_EQ(test_distCall(x), local::distCall(x));
        ASSERT_EQ(test_distCallFull(x), local::distCallFull(x));
    }
}

TEST(BuilderTests, varsUsage)
{
    for (int x = -50; x < 50; ++x)
        ASSERT_EQ(test_varsUsage(x), local::varsUsage(x));
}

TEST(BuilderTests, assigment)
{
    for (int x = -50; x < 50; ++x)
        ASSERT_EQ(test_assigment(x), local::assigment(x));
}

TEST(BuilderTests, boolFuncs)
{
    for (int x = -5; x < 5; ++x) {
        for (int y = -5; y < 5; ++y) {
            ASSERT_EQ(test_less(x, y), local::less(x, y));
            ASSERT_EQ(test_leftBound(x, y), local::leftBound(x, y));
        }
    }
}

