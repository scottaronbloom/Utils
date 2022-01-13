#ifndef __DELAYLINEEDIT_H
#define __DELAYLINEEDIT_H
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

#include <QLineEdit>
#include <QFile>
class QTimer;

namespace NSABUtils
{
    class CBackgroundFileCheck;
    class CDelayLineEdit : public QLineEdit
    {
        Q_OBJECT;
    public:
        enum class ELineEditStatus
        {
            ePending,
            eOK,
            eNotOK
        };

        explicit CDelayLineEdit(QWidget* parent = nullptr);
        explicit CDelayLineEdit(const QString& text, QWidget* parent = nullptr);
        explicit CDelayLineEdit(const QString& text, int delayMS, QWidget* parent = nullptr);

        virtual ~CDelayLineEdit();
        virtual void keyPressEvent(QKeyEvent *event);
        void setText(const QString &text);
        virtual void setDelay(int delayMS);

        void setLineEditColor(ELineEditStatus status);
        void setLineEditColor(bool aOK);
        void setIsOKFunction(std::function< bool(const QString &text) > func, const QString &errorMsg = {})
        {
            fIsOK.first = func;
            fIsOK.second = errorMsg;
        }
    Q_SIGNALS:
        void sigTextChangedAfterDelay(const QString& text);
        void sigTextEditedAfterDelay(const QString& text);
        void sigFinishedEditingAfterDelay( const QString & text );

    public Q_SLOTS:
        void slotTextChanged();
        void slotTextEdited();
        virtual void slotChangedTimerTimeout();
        virtual void slotEditTimerTimeout();

    protected:
        void editTimeout( bool aOK );
        void changeTimeout( bool aOK );
        void connectToEditor( bool connectOrDisconnect );

        int fDelayMS{ 500 };
        QTimer* fChangedTimer{ nullptr };
        QTimer* fEditedTimer{ nullptr };
        bool fEditingFinished{ false };
        std::pair< std::function< bool(const QString &text) >, QString > fIsOK;
        ELineEditStatus fStatus{ ELineEditStatus::ePending };
    };

    class CPathBasedDelayLineEdit : public CDelayLineEdit
    {
        Q_OBJECT;
    public:
        explicit CPathBasedDelayLineEdit( QWidget * parent = nullptr );
        explicit CPathBasedDelayLineEdit( const QString & text, QWidget * parent = nullptr );
        explicit CPathBasedDelayLineEdit( const QString & text, int delayMS, QWidget * parent = nullptr );

        void init();

        virtual ~CPathBasedDelayLineEdit();
        void setIsOKFunction( std::function< bool( const QString & text ) > func, const QString & errorMsg = {} ) = delete;

        virtual void setDelay( int delayMS ) override;


        bool checkExists() const;
        void setCheckExists( bool val );

        bool checkIsBundle() const;
        void setCheckIsBundle( bool val );

        bool checkIsDir() const;
        void setCheckIsDir( bool val );

        bool checkIsExecutable() const;
        void setCheckIsExecutable( bool val );

        bool checkIsFile() const;
        void setCheckIsFile( bool val );

        bool checkIsHidden() const;
        void setCheckIsHidden( bool val );

        bool checkIsJunction() const;
        void setCheckIsJunction( bool val );

        bool checkIsReadable() const;
        void setCheckIsReadable( bool val );

        bool checkIsShortcut() const;
        void setCheckIsShortcut( bool val );

        bool checkIsSymLink() const;
        void setCheckIsSymLink( bool val );

        bool checkIsSymbolicLink() const;
        void setCheckIsSymbolicLink( bool val );

        bool checkIsWritable() const;
        void setCheckIsWritable( bool val );

        QFile::Permissions checkPermissions() const;
        void setCheckPermissions( QFile::Permissions val );

        bool useNTFSPermissions() const;
        void setUseNTFSPermissions( bool val );
    Q_SIGNALS:

    public Q_SLOTS:
        void slotFileCheckFinished( bool aOK );
        virtual void slotChangedTimerTimeout() override;
        virtual void slotEditTimerTimeout() override;

    private:
        bool fEdited{ false };
        bool fChanged{ false };
        CBackgroundFileCheck * fFileChecker{ nullptr };
    };
}
#endif

