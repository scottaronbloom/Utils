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

#include "uiUtils.h"
#include "utils.h"

#include <QFontMetrics>
#include <QDesktopServices>
#include <QUrl>

namespace NSABUtils
{
    bool launchIfURLClicked(const QString& title, const QPoint& pt, const QFont& font)
    {
        int urlStart;
        int urlLength;
        auto hasUrl = NSABUtils::isValidURL(title, &urlStart, &urlLength);
        if (hasUrl)
        {
            auto xLoc = pt.x();
            if (xLoc >= 30)
            {
                xLoc -= 30;
                QFontMetrics fm(font);

                auto preURL = title.left(urlStart);
                auto url = title.mid(urlStart, urlLength);

                auto preRect = fm.boundingRect(preURL);
                if (xLoc >= preRect.width())
                {
                    xLoc -= preRect.width();
                    auto urlRect = fm.boundingRect(url);
                    if (xLoc <= urlRect.width())
                    {
                        QDesktopServices::openUrl(url);
                        return true;
                    }
                }
            }
        }
        return false;
    }
}
