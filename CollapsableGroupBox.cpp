// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
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

#include "GroupBox.h"
#include <QChildEvent>

class CGroupBoxImpl 
{
public:

    CGroupBoxImpl();
    bool fCollapsible;
    bool fFlat; // so we can restore it when expanding
};

CGroupBoxImpl::CGroupBoxImpl() : 
    fCollapsible(true), 
    fFlat(false)
{}


CGroupBox::CGroupBox(QWidget* parent)
        : QGroupBox(parent)
{
    fImpl = new CGroupBoxImpl;
    
    setCheckable(true);
    setChecked(true);
    connect(this, &CGroupBox::toggled, this, &CGroupBox::setExpanded);
}

CGroupBox::CGroupBox(const QString& title, QWidget* parent)
        : CGroupBox( parent )
{
    setTitle( title );
}


CGroupBox::~CGroupBox()
{
    delete fImpl;
}

bool CGroupBox::isCollapsible() const
{
    return fImpl->fCollapsible;
}

void CGroupBox::setCollapsible(bool enable)
{
    if (fImpl->fCollapsible != enable)
    {
        fImpl->fCollapsible = enable;
        if (!enable)
            setExpanded(true);
        else if (!isChecked())
            setExpanded(false);
    }
}

void CGroupBox::setCollapsed(bool collapsed)
{
    setExpanded(!collapsed);
}

void CGroupBox::setExpanded(bool expanded)
{
    if (fImpl->fCollapsible || expanded)
    {
        // show/hide direct children
        foreach(QObject* child, children())
        {
            if (child->isWidgetType())
                static_cast<QWidget*>(child)->setVisible(expanded);
        }
        if (expanded) {
          setFlat(fImpl->fFlat);
        } else {
          fImpl->fFlat = isFlat();
          setFlat(true);
        }
    }
}

void CGroupBox::childEvent(QChildEvent* event)
{
    QObject* child = event->child();
    if (event->added() && child->isWidgetType())
    {
        QWidget* widget = static_cast<QWidget*>(child);
        if (fImpl->fCollapsible && !isChecked())
            widget->hide();
    }
}
