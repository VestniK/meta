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
 */
#pragma once

#if defined(NDEBUG)

#define PRECONDITION(cond) (void)(0)
#define POSTCONDITION(cond) (void)(0)

#else // NDEBUG

#include <cassert>
#include <cstdio>
#include <exception>
#include <functional>

#define PRECONDITION(cond) assert(cond)

class PostCondRunner
{
public:
    PostCondRunner(std::function<bool()> cond, const char *func, const char *condStr): mCond(cond), mName(func), mCondStr(condStr) {}
    ~PostCondRunner()
    {
        if (std::uncaught_exception())
            return; // No condtition check on scope failure
        bool res = mCond();
        if (res)
            return;
        fprintf(stderr, "Function '%s' postcondition '%s' failed\n", mName, mCondStr);
        assert(false);
    }

private:
    std::function<bool()> mCond;
    const char *mName;
    const char *mCondStr;
};

#define CONCAT_3_(a, b) a##b
#define CONCAT_2_(a, b) CONCAT_3_(a, b)
#define CONCAT(a, b) CONCAT_2_(a, b)
#define POSTCONDITION(cond) \
    PostCondRunner CONCAT(postcond, __LINE__) ( \
        [&](){return static_cast<bool>(cond);}, \
        __PRETTY_FUNCTION__, \
        #cond \
    )

#endif // NDEBUG
