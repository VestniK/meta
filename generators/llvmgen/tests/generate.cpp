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

// Functions from *.meta files
extern "C" {

int test_intops_constFoo();

int test_intops_poly(int x);

int test_intops_distCall(int x);

int test_intops_distCallFull(int x);

int test_intops_varsUsage(int x);

int test_intops_assigment(int x);

int test_intops_abs(int x);

int test_intops_sign(int x);

bool test_boolops_less(int x, int y);

bool test_boolops_bound(int left, int right, int val);
bool test_boolops_outOf(int left, int right, int val);

// Import tests:
int test_imports_foo(int x);
int test_imports_impl_fooExport(int x);

// C call tests:
int test_ccall_caller(int x);

}

// Functions exported to the *.meta tests files
extern "C" {

int test_ccall_sqr(int x)
{
    return x*x;
}

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

int abs(int x)
{
    auto res = x;
    if (x < 0)
        res = -x;
    return res;
}

int sign(int x)
{
    if (x < 0)
        return -1;
    else if (x > 0)
        return +1;
    return 0;
}

bool less(int x, int y)
{
    return x < y;
}

bool bound(int left, int right, int val)
{
    auto lcheck = !(val < left);
    auto rcheck = val < right;
    return lcheck && rcheck;
}

bool outOf(int left, int right, int val)
{
    auto lcheck = val < left;
    auto rcheck = val >= right;
    return lcheck || rcheck;
}

// Functions from imports.meta and importsImpl.meta
namespace imports {

int fooProtected(int x);

int fooPrivate(int x)
{
    return x*fooProtected(x);
}

int fooExport(int x)
{
    return fooPrivate(2*x);
}

int fooPublic(int x)
{
    return fooPrivate(x*x);
}

int fooProtected(int x)
{
    return x*x - 2*x;
}

int foo(int x)
{
    if (x < 0)
        return fooExport(x);
    if (x == 0)
        return fooPublic(x);
    return fooProtected(x);
}

} // namespace imports

namespace ccall {

int caller(int x)
{
    return test_ccall_sqr(x);
}

}

} // namespace local

TEST(BuilderTests, constFunc)
{
    ASSERT_EQ(test_intops_constFoo(), 5);
}

TEST(BuilderTests, oneArg)
{
    for (int i = -50; i < 50; ++i)
        ASSERT_EQ(test_intops_poly(i), local::poly(i));
}

TEST(BuilderTests, functionWithCall)
{
    for (int x = -50; x < 50; ++x) {
        ASSERT_EQ(test_intops_distCall(x), local::distCall(x));
        ASSERT_EQ(test_intops_distCallFull(x), local::distCallFull(x));
    }
}

TEST(BuilderTests, varsUsage)
{
    for (int x = -50; x < 50; ++x)
        ASSERT_EQ(test_intops_varsUsage(x), local::varsUsage(x));
}

TEST(BuilderTests, assigment)
{
    for (int x = -50; x < 50; ++x)
        ASSERT_EQ(test_intops_assigment(x), local::assigment(x));
}

TEST(BuilderTests, abs)
{
    for (int x = -50; x < 50; ++x)
        ASSERT_EQ(test_intops_abs(x), local::abs(x));
}

TEST(BuilderTests, sign)
{
    for (int x = -50; x < 50; ++x)
        ASSERT_EQ(test_intops_sign(x), local::sign(x));
}

TEST(BuilderTests, boolFuncs)
{
    for (int x = -5; x < 5; ++x) {
        for (int y = -5; y < 5; ++y) {
            ASSERT_EQ(test_boolops_less(x, y), local::less(x, y));
            for (int val = -10; val < 10; ++ val) {
                ASSERT_EQ(test_boolops_bound(x, y, val), local::bound(x, y, val));
                ASSERT_EQ(test_boolops_outOf(x, y, val), local::outOf(x, y, val));
            }
        }
    }
}

TEST(BuilderTests, imports)
{
    for (int x = -50; x < 50; ++x) {
        ASSERT_EQ(test_imports_foo(x), local::imports::foo(x)) << "x: " << x;
        ASSERT_EQ(test_imports_impl_fooExport(x), local::imports::fooExport(x)) << "x: " << x;
    }
}

TEST(BuilderTests, ccall)
{
    for (int x = -50; x < 50; ++x)
        ASSERT_EQ(test_ccall_caller(x), local::ccall::caller(x)) << "x: " << x;
}