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

#include "JsonUtils.h"

#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

namespace NSABUtils
{
    void ToJson(const QStringList& value, QJsonValue & val)
    {
        QJsonArray retVal;
        for (auto&& ii : value)
        {
            QJsonValue curr;
            ToJson(ii, curr);
            retVal.append(curr);
        }
        val = retVal;
    }

    void FromJson(QStringList& value, const QJsonValue & val)
    {
        value.clear();
        if (!val.isArray())
            return;
        auto array = val.toArray();
        for (int ii = 0; ii < array.count(); ++ii)
        {
            auto curr = array.at(ii).toString();
            if (curr.startsWith("{") && curr.endsWith("}"))
                curr = curr.mid(1, curr.length() - 2);
            value << curr;
        }
    }

    void FromJson(bool& value, const QJsonValue & val)
    {
        value = false;
        if (!val.isObject() && !val.isBool())
            return;

        auto realVal = val;
        if (val.isObject())
        {
            if (!val.toObject().contains("value"))
                return;
            realVal = val.toObject()["value"];
        }
        value = realVal.toBool();
    }

    void FromJson(double& value, const QJsonValue & val)
    {
        value = 0.0;
        if (!val.isObject() && !val.isDouble())
            return;

        auto realVal = val;
        if (val.isObject())
        {
            if (!val.toObject().contains("value"))
                return;
            realVal = val.toObject()["value"];
        }
        value = realVal.toDouble();
    }

    void FromJson(QString& value, const QJsonValue & val)
    {
        value.clear();

        auto realVal = val;
        if (val.isObject())
        {
            if (!val.toObject().contains("value"))
                return;
            realVal = val.toObject()["value"];
        }
        value = realVal.toString();
    }


    void FromJson(int& value, const QJsonValue & val)
    {
        value = 0;
        if (!val.isObject() && !val.isString() && !val.isDouble())
            return;

        auto realVal = val;
        if (val.isObject())
        {
            if (!val.toObject().contains("value"))
                return;
            realVal = val.toObject()["value"];
        }

        if (!realVal.isString() && !realVal.isDouble())
            return;
        if (realVal.isString())
        {
            bool aOK;
            int tmp = realVal.toString().toInt(&aOK);
            if (aOK)
                value = tmp;
        }
        if (realVal.isDouble())
        {
            value = static_cast<int>(val.toDouble());
        }
    }
}