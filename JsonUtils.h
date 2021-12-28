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

#ifndef __JSONUTILS_H
#define __JSONUTILS_H

#include <set>
#include <list>
#include <unordered_map>
#include <QJsonObject>
#include <QJsonArray>

namespace NSABUtils
{
    template< typename T >
    void ToJson(const T& value, QJsonValue& obj)
    {
        obj = QJsonValue(value);
    }

    void ToJson(const QStringList& value, QJsonValue& obj);

    template< typename T >
    void ToJson(const std::list< T > & value, QJsonValue& obj)
    {
        QJsonArray retVal;
        for (auto && ii : value)
        {
            QJsonValue curr;
            ToJson(ii, curr);
            retVal.append(curr);
        }
        obj = retVal;
    }

    template< typename T >
    void ToJson(const std::set< T >& value, QJsonValue& obj)
    {
        QJsonArray retVal;
        for (auto&& ii : value)
        {
            QJsonValue curr;
            ToJson(ii, curr);
            retVal.append(curr);
        }
        obj = retVal;
    }
    template< typename T1, typename T2 >
    void ToJson(const std::pair< T1, T2 >& value, QJsonValue& obj)
    {
        QJsonArray retVal;

        QJsonValue first;
        ToJson(value.first, first);
        retVal.append(first);

        QJsonValue second;
        ToJson(value.second, second);
        retVal.append(second);

        obj = retVal;
    }

    template< typename T2 >
    void ToJson(const std::unordered_map< QString, T2 >& value, QJsonValue& obj)
    {
        QJsonObject retVal;
        for (auto&& ii : value)
        {
            QJsonValue curr;
            ToJson(ii.second, curr);
            retVal[ii.first] = curr;
        }
        obj = retVal;
    }

    void FromJson(QStringList& value, const QJsonValue& obj);
    void FromJson(bool& value, const QJsonValue& obj);
    void FromJson(double& value, const QJsonValue& obj);
    void FromJson(QString& value, const QJsonValue& obj);
    void FromJson(int& value, const QJsonValue& obj);

    template< typename T >
    void FromJson(std::list< T >& value, const QJsonValue& obj)
    {
        value.clear();
        if (!obj.isArray())
            return;
        auto array = obj.toArray();
        for (int ii = 0; ii < array.count(); ++ii)
        {
            QJsonValue currValue = array.at(ii);
            T currObj;
            FromJson(currObj, currValue);
            value.push_back(currObj);
        }
    }

    template< typename T >
    void FromJson(std::set< T >& value, const QJsonValue& obj)
    {
        value.clear();
        if (!obj.isArray())
            return;
        auto array = obj.toArray();
        for (int ii = 0; ii < array.count(); ++ii)
        {
            QJsonValue currValue = array.at(ii);
            T currObj;
            FromJson(currObj, currValue);
            value.insert(currObj);
        }
    }

    template< typename T1, typename T2 >
    void FromJson(std::pair< T1, T2 >& value, const QJsonValue& obj)
    {
        value = std::make_pair(T1(), T2());
        if (!obj.isArray())
            return;
        auto array = obj.toArray();
        if (array.count() != 2)
            return;
        auto first = array.at(0);
        FromJson(value.first, first);

        auto second = array.at(1);
        FromJson(value.second, second);
    }

    template< typename T2 >
    void FromJson(std::unordered_map< QString, T2 >& value, const QJsonValue& obj)
    {
        value.clear();
        if (!obj.isObject())
            return;
        auto map = obj.toObject();
        for (auto && ii = map.constBegin(); ii != map.constEnd(); ++ii)
        {
            auto key = ii.key();
            auto currValue = ii.value();
            T2 currObj;
            FromJson(currObj, currValue);
            value[key] = currObj;
        }
    }
}
#endif 
