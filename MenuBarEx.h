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

#ifndef __MENUBAREX_H
#define __MENUBAREX_H

#include <QMenuBar>
class QTimer;
namespace NSABUtils
{
    class CMenuBarEx : public QMenuBar
    {
        Q_OBJECT
    public:
        CMenuBarEx(QWidget *parent = nullptr);
        ~CMenuBarEx();

        // the following is simply done to call updateConnections
        using QMenuBar::addAction;
        QAction *addAction(const QString &text) { auto retVal = QMenuBar::addAction(text); updateConnections(); return retVal; }
        QAction *addAction(const QString &text, const QObject *receiver, const char *member) { auto retVal = QMenuBar::addAction(text, receiver, member); updateConnections(); return retVal; }

#ifdef Q_CLANG_QDOC
#else
        // addAction(QString): Connect to a QObject slot / functor or function pointer (with context)
        template<typename Obj, typename Func1>
        inline typename std::enable_if<!std::is_same<const char *, Func1>::value
            &&QtPrivate::IsPointerToTypeDerivedFromQObject<Obj *>::Value, QAction *>::type
            addAction(const QString &text, const Obj *object, Func1 slot)
        {
            QAction *result = QMenuBar::addAction(text, object, slot);
            updateConnections();
            return result;
        }
        // addAction(QString): Connect to a functor or function pointer (without context)
        template <typename Func1>
        inline QAction *addAction(const QString &text, Func1 slot)
        {
            QAction *result = QMenuBar::addAction(text, slot);
            updateConnections();
            return result;
        }
#endif // !Q_CLANG_QDOC

        QAction *addMenu(QMenu *menu) { auto retVal = QMenuBar::addMenu(menu); updateConnections(); return retVal; }
        QMenu *addMenu(const QString &title) { auto retVal = QMenuBar::addMenu(title); updateConnections(); return retVal; }
        QMenu *addMenu(const QIcon &icon, const QString &title) { auto retVal = QMenuBar::addMenu(icon, title); updateConnections(); return retVal; }

        QAction *insertMenu(QAction *before, QMenu *menu) { auto retVal = QMenuBar::insertMenu(before, menu); updateConnections(); return retVal; }

        void addAction(QAction *action) { QMenuBar::addAction(action); updateConnections(); }
        void addActions(QList<QAction *> actions) { QMenuBar::addActions(actions); updateConnections(); }

    Q_SIGNALS:
        void sigAboutToEngage();
        void sigFinishedEngagement();
    private Q_SLOTS:
        void slotMenuToShow();
        void slotMenuToHide();
        void slotMenuTimerExpired();
    private:
        void updateConnections();
        QTimer *fTimer{ nullptr };
        bool fEngaged{ false };
    };
}
#endif 
