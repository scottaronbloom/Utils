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

#include "QtUtils.h"
#include "FileUtils.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDir>
#include <QLocale>
#include <QRegularExpression>
#include <QAbstractItemModel>
#include <QSettings>
#include <QSize>
#include <QPoint>
#include <QColor>
#include <QTreeView>
#include <QTimer>
#include <QLayout>
#include <QPlainTextEdit>

#ifdef QT_XMLPATTERNS_LIB
#include <QXmlQuery>
#endif

namespace NQtUtils
{
    QString allFilesFilter()
    {
        QString retVal = QObject::tr("All Files");
#ifdef WIN32
        retVal += " (*.*)";
#else
        retVal += " (*)";
#endif
        return retVal;
    }

    QString defaultFileDialogDir()
    {
        QString retVal;
#ifndef WIN32
        retVal += ".";
#endif
        return retVal;
    }

#ifdef QT_XMLPATTERNS_LIB
    QString getString(QXmlQuery & query, const QString & queryString, bool * aOK /*= nullptr */)
    {
        query.setQuery(queryString);
        if (!query.isValid())
        {
            if (aOK)
                *aOK = false;
            return QString();
        }
        QString retVal;
        query.evaluateTo(&retVal);
        retVal = retVal.trimmed();
        if (aOK)
            *aOK = true;
        return retVal;
    }

    QStringList getStringList(QXmlQuery & query, const QString & queryString, bool * aOK /*= nullptr */)
    {
        query.setQuery(queryString);
        if (!query.isValid())
        {
            if (aOK)
                *aOK = false;
            return QStringList();
        }
        QStringList retVal;
        query.evaluateTo(&retVal);
        for (auto && ii : retVal)
            ii = ii.trimmed();
        if (aOK)
            *aOK = true;
        return retVal;
    }

    std::set< QString > getStringSet(QXmlQuery & query, const QString & queryString, bool * aOK /*= nullptr */)
    {
        auto tmp = getStringList(query, queryString, aOK /*= nullptr */);
        std::set< QString > retVal = { tmp.begin(), tmp.end() };
        return retVal;
    }

    int getInt(QXmlQuery & query, const QString & queryString, bool * aOK /*= nullptr */)
    {
        bool lclAOK = false;
        auto str = getString(query, queryString, &lclAOK);
        if (aOK)
            *aOK = lclAOK;
        if (!lclAOK)
            return 0;
        return getInt(str, aOK);
    }

    bool getBool(QXmlQuery & query, const QString & queryString, bool defaultVal/*=false*/)
    {
        bool lclAOK = false;
        auto str = getString(query, queryString, &lclAOK);
        if (!lclAOK)
            return defaultVal;
        return getBool(str, defaultVal);
    }

    double getDouble(QXmlQuery & query, const QString & queryString, bool * aOK /*= nullptr */)
    {
        bool lclAOK = false;
        auto str = getString(query, queryString, &lclAOK);
        if (aOK)
            *aOK = lclAOK;
        if (!lclAOK)
            return 0.0;
        return getDouble(str, aOK);
    }


    std::list< std::pair< QString, QString > > getStringPairs(QXmlQuery & query, const QString & queryString1, const QString & queryString2, bool * aOK)
    {
        auto tmp = getStrings(query, QStringList() << queryString1 << queryString2, aOK);
        std::list< std::pair< QString, QString > > retVal;
        for (auto && ii : tmp)
        {
            retVal.push_back(std::make_pair(ii.front(), ii.back()));
        }
        return retVal;
    }

    std::list< std::list< QString > > getStrings(QXmlQuery & query, const QStringList & queries, bool * aOK)
    {
        std::list< std::list< QString > > retVal;
        size_t sz = std::string::npos;
        for (auto && currQuery : queries)
        {
            bool localAOK;
            auto currList = getStringList(query, currQuery, &localAOK);
            if (!localAOK)
            {
                if (aOK)
                    *aOK = false;
                return{};
            }
            if (sz != std::string::npos)
            {
                if (sz != currList.count())
                {
                    if (aOK)
                        *aOK = false;
                    return{};
                }
            }
            else
                sz = currList.size();

            auto curr = retVal.begin();
            for (int ii = 0; ii < currList.count(); ++ii)
            {
                if (curr == retVal.end())
                {
                    retVal.push_back({ currList[ii] });
                    curr = retVal.end();
                }
                else
                {
                    (*curr).push_back(currList[ii]);
                    curr++;
                }
            }
        }

        if (aOK)
            *aOK = true;
        return retVal;
    }

    QString getFile(QXmlQuery & query, const QDir & relToDir, const QString & queryString, bool * aOK /*= nullptr */)
    {
        bool localAOK = false;
        auto path = getString(query, queryString, &localAOK);
        if (aOK)
            *aOK = localAOK;
        if (!localAOK)
            return QString();

        return getFile(relToDir, path);
    }
#endif

    int getInt(const QStringRef & str, bool * aOK)
    {
        bool lclAOK;
        int retVal = str.toInt(&lclAOK);
        if (!lclAOK)
            retVal = qRound(str.toDouble(&lclAOK));
        if (!lclAOK)
            retVal = str.toInt(&lclAOK, 16);
        if (aOK)
            *aOK = lclAOK;
        if (!lclAOK)
            return 0;
        return retVal;
    }

    int getInt(const QString & str, bool * aOK)
    {
        QStringRef tmp(&str);
        return getInt(tmp, aOK);
    }

    int getInt(const QString & str)
    {
        bool aOK;
        return getInt(str, &aOK);
    }

    int getInt(const QStringRef & str)
    {
        bool aOK;
        return getInt(str, &aOK);
    }

    int getInt(const QString & value, QXmlStreamReader & reader)
    {
        QStringRef tmp(&value);
        return getInt(tmp, reader);
    }

    int getInt(const QString & value, int defaultValue, QXmlStreamReader & reader)
    {
        QStringRef tmp(&value);
        return getInt(tmp, defaultValue, reader);
    }


    int getInt(const QStringRef & value, QXmlStreamReader & reader)
    {
        bool aOK;
        int retVal = getInt(value, &aOK);
        if (!aOK)
        {
            reader.raiseError(QString("Invalid value, expecting integer '%1'").arg(value.toString()));
            return 0;
        }
        return retVal;
    }

    int getInt(const QStringRef & value, int defaultValue, QXmlStreamReader & /*reader*/)
    {
        bool aOK;
        int retVal = getInt(value, &aOK);
        if (!aOK)
        {
            return defaultValue;
        }
        return retVal;
    }

    double getDouble(const QStringRef & str, bool * aOK)
    {
        bool lclAOK;
        double retVal = str.toDouble(&lclAOK);
        if (aOK)
            *aOK = lclAOK;
        if (!lclAOK)
            return 0.0;
        return retVal;
    }

    double getDouble(const QString & str, bool * aOK)
    {
        QStringRef tmp(&str);
        return getDouble(tmp, aOK);
    }

    double getDouble(const QString & str)
    {
        bool aOK;
        return getDouble(str, &aOK);
    }

    double getDouble(const QStringRef & str)
    {
        bool aOK;
        return getDouble(str, &aOK);
    }

    double getDouble(const QString & value, QXmlStreamReader & reader)
    {
        QStringRef tmp(&value);
        return getDouble(tmp, reader);
    }

    double getDouble(const QStringRef & value, QXmlStreamReader & reader)
    {
        bool aOK;
        double retVal = getDouble(value, &aOK);
        if (!aOK)
        {
            reader.raiseError(QString("Invalid value, expecting double '%1'").arg(value.toString()));
            return 0;
        }
        return retVal;
    }

    bool getBool(const QString & str, bool defaultVal)
    {
        if (str.isEmpty())
            return defaultVal;

        QString val = str.toLower();

        return val != "0" && val != "false" && val != "f" && val != "n" && val != "no" && val != "off";
    }

    bool getBool(const QStringRef & str, bool defaultVal)
    {
        return getBool(str.toString(), defaultVal);
    }

    QString getFile(const QString & relToDir, QXmlStreamReader & reader, QString * origFile)
    {
        return getFile(QDir(relToDir), reader, origFile);
    }

    QString getFile(const QDir & relToDir, QXmlStreamReader & reader, QString * origFile)
    {
        QString file = reader.readElementText();
        if (origFile)
            *origFile = file;
        return getFile(relToDir, file);
    }

    QString getFile(const QDir & relToDir, const QString & file)
    {
        QString retVal = file;
        if (!retVal.isEmpty() && QFileInfo(retVal).isRelative())
        {
            retVal = relToDir.absoluteFilePath(retVal);
            if (QFileInfo(retVal).exists())
                retVal = NFileUtils::canonicalFilePath(retVal);
        }
        return retVal;
    }

    QDateTime getDateTime(const QString & dateString)
    {
        QLocale locale;
        QDateTime retVal = QDateTime::fromString(dateString);
        if (!retVal.isValid())
            retVal = QDateTime::fromString(dateString, locale.dateTimeFormat(QLocale::ShortFormat));
        if (!retVal.isValid())
            retVal = QDateTime::fromString(dateString, locale.dateTimeFormat(QLocale::LongFormat));
        if (!retVal.isValid())
            retVal = QDateTime::fromString(dateString, Qt::ISODate);
        if (!retVal.isValid())
            retVal = QDateTime::fromString(dateString, "yyyy-M-dTh:m:s");
        if (!retVal.isValid())
            retVal = QDateTime::fromString(dateString, "yyyy-MM-ddTHH:mm:ss");
        if (!retVal.isValid())
            retVal = QDateTime::fromString(dateString, "yyyy-MM-dd");
        if (!retVal.isValid())
            retVal = QDateTime::fromString(dateString, "yyyy-M-d");
        if (!retVal.isValid())
            retVal = QDateTime::fromString(dateString, "yyyy/MM/dd");
        return retVal;
    }

    QDateTime getDateTime(const QStringRef & value, QXmlStreamReader & reader, bool optional)
    {
        QDateTime retVal = getDateTime(value.toString());
        if (!retVal.isValid())
        {
            if (!optional)
                reader.raiseError(QString("Invalid value, expecting date '%1'").arg(value.toString()));
            return QDateTime();
        }
        return retVal;
    }

    QDateTime getDateTime(const QString & value, QXmlStreamReader & reader, bool optional)
    {
        return getDateTime(QStringRef(&value), reader, optional);
    }

    QStringList splitLineCSV(const QString & line)
    {
        QString temp = line;
        QString field;
        QStringList field_list;

        // regex explaination
        //
        //    /(?:^|,)(\"(?:[^\"]+|\"\")*\"|[^,]*)/g
        //        (?:^|,) Non-capturing group
        //            1st Alternative: ^
        //                ^ assert position at start of the string
        //            2nd Alternative: ,
        //                , matches the character , literally
        //        1st Capturing group (\"(?:[^\"]+|\"\")*\"|[^,]*)
        //            1st Alternative: \"(?:[^\"]+|\"\")*\"
        //                \" matches the character " literally
        //                (?:[^\"]+|\"\")* Non-capturing group
        //                    Quantifier: * Between zero and unlimited times, as many times as possible, giving back as needed [greedy]
        //                    1st Alternative: [^\"]+
        //                        [^\"]+ match a single character not present in the list below
        //                            Quantifier: + Between one and unlimited times, as many times as possible, giving back as needed [greedy]
        //                            \" matches the character " literally
        //                    2nd Alternative: \"\"
        //                        \" matches the character " literally
        //                        \" matches the character " literally
        //                \" matches the character " literally
        //            2nd Alternative: [^,]*
        //                [^,]* match a single character not present in the list below
        //                    Quantifier: * Between zero and unlimited times, as many times as possible, giving back as needed [greedy]
        //                    , the literal character ,
        //        g modifier: global. All matches (don't return on first match)
        //

        QString regex = "(?:^|,)(\"(?:[^\"]+|\"\")*\"|[^,]*)";

        QRegularExpression re(regex);

        if (temp.right(1) == "\n")
            temp.chop(1);

        QRegularExpressionMatchIterator it = re.globalMatch(temp);

        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            if (match.hasMatch())
            {
                field = match.captured(1);
                if (field.left(1) == "\"" && field.right(1) == "\"")
                    field = field.mid(1, field.length() - 2);
                field_list.push_back(field);
            }
        }

        return field_list;
    }

    size_t SizeOf(const QDateTime & /*dt*/)
    {
        return sizeof(QDateTime); // NOT ACCURATE
    }

    size_t SizeOf(const QString & str)
    {
        size_t retVal = sizeof(QString);
        retVal += str.capacity() * sizeof(QChar);
        return retVal;
    }

    QString fromHtmlEscaped(const QString & str)
    {
        QString retVal = str;
        QRegularExpression regExp("(?<lt>\\&lt\\;)|(?<gt>\\&gt\\;)|(?<amp>\\&amp\\;)|(?<quot>\\&quot\\;)", QRegularExpression::PatternOption::CaseInsensitiveOption);
        auto match = regExp.match(str, 0);

        while (match.hasMatch())
        {
            if (!match.captured("lt").isEmpty())
            {
                retVal.replace(match.capturedStart("lt"), match.capturedLength("lt"), "<");
            }
            else if (!match.captured("gt").isEmpty())
            {
                retVal.replace(match.capturedStart("gt"), match.capturedLength("gt"), ">");
            }
            else if (!match.captured("amp").isEmpty())
            {
                retVal.replace(match.capturedStart("amp"), match.capturedLength("amp"), "&");
            }
            else if (!match.captured("quot").isEmpty())
            {
                retVal.replace(match.capturedStart("quot"), match.capturedLength("quot"), "\"");
            }
            match = regExp.match(retVal, match.capturedStart() + 1);
        }

        return retVal;
    }

    void move(QSettings & settings, const QString & subGroup, const QString & key, bool overwrite)
    {
        if (settings.contains(key))
        {
            auto value = settings.value(key, true);
            if (!overwrite)
            {
                settings.beginGroup(subGroup);
                bool contains = settings.contains(key);
                settings.endGroup();
                if (contains)
                    return;
            }

            settings.beginGroup(subGroup);
            settings.remove(key);
            settings.setValue(key, value);
            settings.endGroup();
        }
    }

    void copy(QSettings & from, QSettings & to, bool overwrite)
    {
        auto childKeys = from.childKeys();
        for (auto && ii : childKeys)
        {
            if (!overwrite && to.contains(ii))
                continue;
            auto value = from.value(ii);
            to.setValue(ii, value);
        }
        auto childGroups = from.childGroups();
        for (auto && ii : childGroups)
        {
            from.beginGroup(ii);
            to.beginGroup(ii);
            copy(from, to, overwrite);
            from.endGroup();
            to.endGroup();
        }
    }
    int itemCount(const QModelIndex & idx, bool rowCountOnly)
    {
        int retVal = 1; // 1 counts self
        for (int ii = 0; ii < idx.model()->rowCount(idx); ++ii)
        {
            if (rowCountOnly)
            {
                retVal += itemCount(idx.model()->index(ii, 0, idx), rowCountOnly);
            }
            else
            {
                for (int jj = 0; jj < idx.model()->columnCount(); ++jj)
                {
                    retVal += itemCount(idx.model()->index(ii, jj, idx), rowCountOnly);
                }
            }
        }
        return retVal;
    }

    int itemCount(QAbstractItemModel * model, bool rowCountOnly)
    {
        if (!model)
            return 0;
        int retVal = 0;
        for (int ii = 0; ii < model->rowCount(); ++ii)
        {
            if (rowCountOnly)
            {
                retVal += itemCount(model->index(ii, 0, QModelIndex()), rowCountOnly);
            }
            else
            {
                for (int jj = 0; jj < model->columnCount(); ++jj)
                {
                    retVal += itemCount(model->index(ii, jj, QModelIndex()), rowCountOnly);
                }
            }
        }
        return retVal;
    }

    QStringList getHeadersForModel(QAbstractItemModel * model)
    {
        QStringList retVal;
        for (int ii = 0; ii < model->columnCount(); ++ii)
        {
            auto header = model->headerData(ii, Qt::Orientation::Horizontal).toString();
            header = header.replace("/", "");
            if (header.isEmpty())
                header = QString("Col%1").arg(ii);
            retVal << header;
        }
        return retVal;
    }

    void writeModel(QAbstractItemModel * model,
        QXmlStreamWriter & writer,
        const QString & keyName,
        const QString & plauralSuffix,
        const std::function<void(QAbstractItemModel * model, QXmlStreamWriter & writer, const QString & keyName, int rowNum) > & writeRow)
    {
        QStringList headers = getHeadersForModel(model);

        writer.writeStartElement(keyName + plauralSuffix); // base path is singular

        for (int row = 0; row < model->rowCount(); ++row)
        {
            if (writeRow)
                writeRow(model, writer, keyName, row);
            else
            {
                writer.writeStartElement(keyName);
                for (int col = 0; col < model->columnCount(); ++col)
                {
                    auto value = model->index(row, col).data().toString();
                    writer.writeTextElement(headers[col], value);
                }
                writer.writeEndElement();
            }
        }
        writer.writeEndElement();
    }


    void expandAll(QAbstractItemModel * model, const QModelIndex & index, QTreeView * view)
    {
        if (!model || !view)
            return;

        view->setExpanded(index, true);

        auto rowCount = model->rowCount(index);
        auto columnCount = model->columnCount(index);
        for (auto ii = 0; ii < rowCount; ++ii)
        {
            for (auto jj = 0; jj < columnCount; ++jj)
            {
                auto childIndex = model->index(ii, jj, index);
                expandAll(model, childIndex, view);
            }
        }
    }

    size_t CCaseInsensitiveHash::operator()(const QString & str) const
    {
        return qHash(QDir(str).absolutePath().toLower());
    }

    size_t CCaseInsensitiveEqual::operator()(const QString & lhs, const QString & rhs) const
    {
        return QDir(lhs).absolutePath().compare(QDir(rhs).absolutePath(), Qt::CaseInsensitive) == 0;
    }


    QDate findDate(const QString & string, const QStringList & aFormats, const QStringList & bFormats, const QStringList & cFormats)
    {
        static auto separators = QStringList() << ":" << "/" << "-" << "";
        for (auto && ii : aFormats)
        {
            for (auto && jj : bFormats)
            {
                for (auto && kk : cFormats)
                {
                    for (auto && ll : separators)
                    {
                        auto dt = QDate::fromString(string, QString("%1%4%2%4%3").arg(ii).arg(jj).arg(kk).arg(ll));
                        if (dt.isValid())
                        {
                            return dt;
                        }
                    }
                }
            }
        }
        return QDate();
    }

    QDate findDate(const QString & dateString)
    {
        static auto yearFormats = QStringList() << "yyyy" << "yy";
        static auto monthFormats = QStringList() << "M" << "MM";
        static auto dateFormats = QStringList() << "dd" << "d";
        auto dt = findDate(dateString, yearFormats, monthFormats, dateFormats);
        if (!dt.isValid())
            dt = findDate(dateString, yearFormats, dateFormats, monthFormats);
        if (!dt.isValid())
            dt = findDate(dateString, monthFormats, yearFormats, dateFormats);
        if (!dt.isValid())
            dt = findDate(dateString, monthFormats, dateFormats, yearFormats);
        if (!dt.isValid())
            dt = findDate(dateString, dateFormats, yearFormats, monthFormats);
        if (!dt.isValid())
            dt = findDate(dateString, dateFormats, monthFormats, yearFormats);
        return dt;
    }

    void updateTimer(int delayMS, QTimer * timer)
    {
        if (!timer)
            return;

        bool isActive = timer->isActive();
        timer->stop();
        timer->setInterval(delayMS);
        if (isActive)
            timer->start();
    }

    void deleteLayoutAndItems(QLayout * layout)
    {
        if (!layout)
            return;

        QLayoutItem * child = nullptr;
        while ((child = layout->takeAt(0)) != nullptr)
        {
            if (child->widget())
                delete child->widget();
            else if (child->layout())
                deleteLayoutAndItems(child->layout());
            delete child;
        }
        delete layout;
        layout = nullptr;
    }

    void appendToLog(QPlainTextEdit * te, const QString & txt, std::pair< QString, bool > & previousText, QTextStream * ts )
    {
        if (ts)
            *ts << txt;
        auto lineStart = 0;
        if (txt == "\n" || txt == "\r\n")
        {
            te->appendPlainText(previousText.first + txt);
            lineStart = txt.length();
        }
        else
        {
            auto regEx = QRegularExpression("[\r\n]");
            auto pos = txt.indexOf(regEx, lineStart);
            while (pos != -1)
            {
                if (previousText.second)
                {
                    previousText.second = false;
                    te->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                    te->moveCursor(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
                    te->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
                    te->textCursor().removeSelectedText();
                    te->textCursor().deletePreviousChar();
                    te->moveCursor(QTextCursor::End);
                }

                QString tmp = txt.mid(lineStart, pos - lineStart);

                bool changeLineStart = false;
                int skip = 1;
                if (txt.at(pos) == '\n') // just reset lineStart;
                {
                    changeLineStart = true;
                }
                else if (txt.at(pos) == '\r')
                {
                    // if its "\r\n" treat it just like a "\n"
                    if ((pos + 1) < txt.length() && txt.at(pos + 1) == '\n')
                    {
                        changeLineStart = true;
                        skip = 2;
                    }
                    else // just a \r throw the line out
                    {
                        // skip this line
                        changeLineStart = true;
                        previousText.second = true;
                    }
                }
                else
                {
                    changeLineStart = false;
                }

                QString subStr = previousText.first + txt.mid(lineStart, pos - lineStart);
                if (!subStr.isEmpty())
                {
                    //qDebug() << subStr;
                    te->appendPlainText(subStr);
                    previousText.first.clear();
                }
                else
                {
                    te->appendPlainText("\n");
                    previousText.first.clear();
                }

                if (changeLineStart)
                    lineStart = pos + skip;

                pos = txt.indexOf(regEx, pos + skip);
            }
        }
        previousText.first = txt.mid(lineStart);
        te->moveCursor(QTextCursor::End);
    }

    uint8_t * imageToPixels( const QImage & image ) // allocates the space, user is responsible for memory deletion using array delete
    {
        Q_ASSERT( sizeof( uchar ) == sizeof( uint8_t ) );

        auto imageSize = static_cast<size_t>(image.width() * image.height() * 4 * sizeof( uint8_t ));
        auto retVal = new uint8_t[imageSize];
        std::memcpy( retVal, image.bits(), imageSize );
        return retVal;
    }
}
