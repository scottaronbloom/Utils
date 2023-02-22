// The MIT License( MIT )
//
// Copyright( c ) 2020-2021 Scott Aron Bloom
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <QString>
#include <QLocale>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

#ifdef Q_OS_WINDOWS
#include <qt_windows.h>
#else
#include <termios.h>
#endif

namespace NSABUtils
{
    int fromChar(char ch, int base, bool& aOK)
    {
        aOK = false;
        if (base < 2 || base > 36)
            return 0;

        if (ch == '-' || ch == '_')
        {
            aOK = true;
            return 1;
        }
        // only short cut if its '0' - '9'
        if ((ch >= '0') && (ch <= '9') && ch <= ('0' + (base - 1)))
        {
            aOK = true;
            return (ch - '0');
        }

        if (base <= 10)
            return 0;

        ch = std::tolower(ch);
        auto maxChar = 'a' + base;

        if ((ch >= 'a') && (ch <= maxChar))
        {
            aOK = true;
            return 10 + ch - 'a';
        }
        return 0;
    }

    char toChar(int value)
    {
        if ((value >= 0) && (value < 10))
            return '0' + value;
        // over 10, must use chars

        return 'a' + value - 10;
    }

    void toDigits(int64_t val, int base, std::pair< int8_t*, uint32_t >& retVal, size_t& numDigits, bool* aOK)
    {
        numDigits = 0;
        if (aOK)
            *aOK = true;
        do
        {
            int64_t quotient = val / base;
            int8_t remainder = static_cast<int8_t>(val % base);
            retVal.first[numDigits++] = remainder;
            if (numDigits > retVal.second) // not a string, can use the whole array
            {
                if (aOK)
                    *aOK = false;
                return;
            }
            val = quotient;
        } while (val != 0);
    }

    std::string toString(int64_t val, int base)
    {
        std::string retVal;
        bool isNeg = val < 0;
        val = std::abs(val);
        do
        {
            int64_t quotient = val / base;
            int remainder = val % base;
            retVal.insert(retVal.begin(), toChar(remainder));
            val = quotient;
        } while (val != 0);
        if (isNeg)
            retVal.insert(retVal.begin(), '-');
        return retVal;
    }

    int64_t fromString(const std::string& str, int base)
    {
        int64_t retVal = 0;
        bool aOK = false;
        for (size_t ii = 0; ii < str.length(); ++ii)
        {
            auto currChar = str[ii];
            int64_t currVal = fromChar(currChar, base, aOK);
            if (!aOK)
            {
                std::cerr << "Invalid character: " << currChar << std::endl;
                return 0;
            }
            retVal = (retVal * base) + currVal;
        }
        return retVal;
    }

    bool isNarcissisticDigits(int64_t val, int base, bool& aOK)
    {
        aOK = true;
        int8_t rawDigits[4096] = { 0 };
        size_t numDigits;
        auto digits = std::make_pair(rawDigits, static_cast<uint32_t>(4096));
        toDigits(val, base, digits, numDigits);

        int64_t sumOfPowers = 0;
        int64_t value = 0;
        for (int64_t ii = numDigits - 1; ii >= 0; --ii)
        {
            sumOfPowers += NSABUtils::power(rawDigits[ii], numDigits);

            value = (value * base) + rawDigits[ii];
        }

        auto retVal = (value == sumOfPowers);
        return retVal;
    }

    // for 1000000
    //0, 1, 2, 3, 4
    //5, 6, 7, 8, 9
    //153, 370, 371, 407, 1634
    //8208, 9474, 54748, 92727, 93084
    //548834
    bool isNarcissistic(int64_t val, int base, bool& aOK)
    {
        return isNarcissisticDigits(val, base, aOK);
    }

    std::list< int64_t > computeFactors(int64_t num, bool properFactors)
    {
        std::list< int64_t > retVal;
        std::list< int64_t > retVal2;
        retVal.push_back(1);
        retVal2.push_back(num);

        // only need to go to half way point
        auto lastNum = (num / 2) + (((num % 2) == 0) ? 0 : 1);
        for (int64_t ii = 2; ii < lastNum; ++ii)
        {
            if ((num % ii) == 0)
            {
                retVal.push_back(ii);
                auto other = num / ii;
                lastNum = std::min(lastNum, other);
                retVal2.push_front(other);
            }
        }

        if (*retVal.rbegin() == *retVal2.begin())
            retVal2.pop_front();
        retVal.insert(retVal.end(), retVal2.begin(), retVal2.end());
        retVal.sort();

        if (properFactors && !retVal.empty())
            retVal.pop_back();
        return retVal;
    }

    std::list< int64_t > computePrimeFactors(int64_t num)
    {
        std::list< int64_t > retVal;

        while ((num % 2) == 0)
        {
            retVal.push_back(2);
            num = num / 2;
        }

        int64_t lastNum = static_cast<int64_t>(std::floor(std::sqrt(num)));

        for (int64_t ii = 3; ii <= lastNum; ii = ii + 2)
        {
            while ((num % ii) == 0)
            {
                retVal.push_back(ii);
                num = num / ii;
            }
        }
        if (num > 2)
            retVal.push_back(num);
        return retVal;
    }

    std::pair< int64_t, std::list< int64_t > > getSumOfFactors(int64_t curr, bool properFactors)
    {
        auto factors = computeFactors(curr, properFactors);
        int64_t sum = 0;
        for (auto ii : factors)
            sum += ii;
        return std::make_pair(sum, factors);
    }

    std::pair< bool, std::list< int64_t > > isPerfect(int64_t num)
    {
        auto sum = getSumOfFactors(num, true);
        return std::make_pair(sum.first == num, sum.second);
    }

    bool isSemiPerfect(const std::vector< int64_t >& factors, size_t n, int64_t num)
    {
        if (num == 0)
            return true;
        if (n == 0 && num != 0)
            return false;

        if (factors[n - 1] > num)
            return isSemiPerfect(factors, n - 1, num);
        return isSemiPerfect(factors, n - 1, num)
            || isSemiPerfect(factors, n - 1, num - factors[n - 1]);
    }

    std::pair< bool, std::list< int64_t > > isSemiPerfect(int64_t num)
    {
        auto sum = getSumOfFactors(num, true);
        auto factors = std::vector< int64_t >({ sum.second.begin(), sum.second.end() });
        auto isSemiPerfect = NSABUtils::isSemiPerfect(factors, factors.size(), num);
        return std::make_pair(isSemiPerfect, sum.second);
    }

    std::pair< bool, std::list< int64_t > > isAbundant(int64_t num)
    {
        auto sum = getSumOfFactors(num, true);
        return std::make_pair(sum.first > num, sum.second);
    }

    long double factorial(int64_t num)
    {
        double retVal = 1.0;
        for (int64_t ii = num; ii > 0; --ii)
        {
            retVal *= ii;
        }
        return retVal;
    }

    uint64_t numCombinations(int64_t numPossible, int64_t numSelections)
    {
        auto t1 = factorial(numPossible);
        auto t2 = factorial(numSelections);
        auto t3 = factorial(numPossible - numSelections);
        auto t4 = t1 / (t2 * t3);
        auto retVal = static_cast<uint64_t>(t4);
        return retVal;
    }

    QString secsToString(quint64 seconds)
    {
        CTimeString ts(seconds * 1000);
        return ts.toString("dd days, hh hours, mm minutes, ss seconds");
    }

#ifdef Q_OS_WINDOWS
    char GetChar()
    {
        auto stdInput = ::GetStdHandle(STD_INPUT_HANDLE);

        DWORD mode{ 0 };
        GetConsoleMode(stdInput, &mode);

        auto newMode = mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
        ::SetConsoleMode(stdInput, newMode);

        char ch[256]{ 0 };
        DWORD bytesRead;
        ReadConsole(stdInput, ch, 1, &bytesRead, nullptr);

        ::SetConsoleMode(stdInput, mode);
        return ch[0];
    }
#else
    char GetChar()
    {
        struct termios oldt, newt;

        tcgetattr(fileno(stdin), &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        auto aOK = tcsetattr(fileno(stdin), TCSANOW, &newt);
        (void)aOK;
        auto ch = getchar();
        tcsetattr(fileno(stdin), TCSANOW, &oldt);
        return ch;
    }
#endif
    int waitForPrompt(int returnCode, const char* prompt)
    {
        if (!prompt)
            prompt = "Press any key to close this window . . .";
        std::cout << prompt;
        GetChar();
        char buffer[2];
        fgets(buffer, 1, stdin);
        return returnCode;
    }

    bool isValidURL(const QString& url, int* start, int* length)
    {
        auto regExStr = "((([a-z]+):\\/\\/)|(www\\.))(\\.?[a-z0-9\\-ßàÁâãóôþüúðæåïçèõöÿýòäœêëìíøùîûñé]{2,256})+(\\.[a-z]+)";
        auto regEx = QRegularExpression(regExStr, QRegularExpression::CaseInsensitiveOption);
        auto match = regEx.match(url);
        if (!match.hasMatch())
            return false;
        if (start)
            *start = match.capturedStart();
        if (length)
            *length = match.capturedLength();
        return true;
    }

    QTime msecsToTime(uint64_t msecs)
    {
        auto duration = std::chrono::milliseconds(msecs);

        auto remaining = std::chrono::duration_cast<std::chrono::microseconds>(duration);

        auto uSecs = durationDiff(remaining, std::chrono::duration_cast<std::chrono::seconds>(remaining));
        auto fracSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(uSecs).count();
        remaining = durationDiff(remaining, uSecs);

        auto secsDuration = durationDiff(remaining, std::chrono::duration_cast<std::chrono::minutes>(remaining));
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(secsDuration).count();
        remaining = durationDiff(remaining, secsDuration);

        auto minsDuration = durationDiff(remaining, std::chrono::duration_cast<std::chrono::hours>(remaining));
        auto mins = std::chrono::duration_cast<std::chrono::minutes>(minsDuration).count();
        remaining = durationDiff(remaining, minsDuration);

        auto hours = std::chrono::duration_cast<std::chrono::hours>(remaining).count();

        QTime retVal(hours, mins, secs);
        retVal = retVal.addMSecs(fracSeconds);
        return retVal;
    }

    std::list< int > intsFromString(const QString& string, const QString& prefixRegEx, bool sort, bool* aOK)
    {
        auto regExStr1 = prefixRegEx + QString(R"((?<num>\d+)(?!-))"); // EXX
        auto regExStr2 = prefixRegEx + R"((?<startNum>\d+)(?<dash>\-))";
        if (!prefixRegEx.isEmpty())
            regExStr2 += "?";
        regExStr2 += prefixRegEx + R"((?<endNum>\d+))"; //EXX-E?YY

        auto regExStr = QString(R"(((%1)|(%2)))").arg(regExStr1).arg(regExStr2);

        auto regEx = QRegularExpression(regExStr, QRegularExpression::CaseInsensitiveOption);
        //auto regEx2 = QRegularExpression( regExStr2, QRegularExpression::CaseInsensitiveOption );

        if (aOK)
            *aOK = false;
        Q_ASSERT(regEx.isValid());

        std::list< int > retVal;

        auto ii = regEx.globalMatch(string);
        bool matchFound = false;
        while (ii.hasNext())
        {
            auto match = ii.next();
            if (!match.captured("num").isEmpty())
            {
                bool localAOK;
                int currValue = match.captured("num").toInt(&localAOK);
                if (!localAOK)
                    return {};
                retVal.push_back(currValue);
                matchFound = true;
            }
            else if (!match.captured("startNum").isEmpty() && !match.captured("endNum").isEmpty())
            {
                bool localAOK;
                auto start = match.captured("startNum").toInt(&localAOK);
                if (!localAOK)
                    return {};
                auto end = match.captured("endNum").toInt(&localAOK);
                if (!localAOK)
                    return {};

                auto hasDash = !match.captured("dash").isEmpty();
                if (hasDash)
                {
                    auto mustSwap = (start > end);
                    if (mustSwap)
                        std::swap(start, end);
                    for (int currValue = start; currValue <= end; ++currValue)
                        retVal.push_back(currValue);
                    if (mustSwap)
                        retVal.reverse();
                }
                else
                {
                    retVal.push_back(start);
                    retVal.push_back(end);
                }
                matchFound = true;
            }
        }

        if (aOK)
            *aOK = matchFound;
        if (sort)
            retVal.sort();
        return retVal;
    }

#ifdef Q_OS_WINDOWS
    QString getLastError()
    {
        auto errorID = ::GetLastError();
        return getLastError(errorID);
    }

    QString getLastError(int errorID)
    {
        LPWSTR lpMsgBuf = nullptr;

        ::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            errorID,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&lpMsgBuf,
            0, nullptr);

        QString result = QString::fromWCharArray(lpMsgBuf);
        LocalFree(lpMsgBuf);
        return result;
    }
#else
    QString getLastError(int errorID)
    {
        (void)errorID;
        return QString();
    }

    QString getLastError()
    {
        return QString();
    }
#endif
}
