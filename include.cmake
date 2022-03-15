# The MIT License (MIT)
#
# Copyright (c) 2020-2021 Scott Aron Bloom
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set(qtproject_SRCS
    AutoWaitCursor.cpp
    AutoFetch.cpp
    BackgroundFileCheck.cpp
    ButtonEnabler.cpp
    WidgetEnabler.cpp
    ConsoleUtils.cpp
    DelayComboBox.cpp
    DelayLineEdit.cpp
    DelaySpinBox.cpp
    DoubleProgressDlg.cpp
    ImageScrollBar.cpp
    utils.cpp
    FileCompare.cpp
    FileUtils.cpp
    FromString.cpp
    JsonUtils.cpp
    MD5.cpp
    MenuBarEx.cpp
    SelectFileUrl.cpp
    StringUtils.cpp
    StringComparisonClasses.cpp
    QtUtils.cpp
    ScrollMessageBox.cpp
    SpinBox64.cpp
    SpinBox64U.cpp
    CollapsableGroupBox.cpp
    WordExp.cpp
    QtDumper.cpp
    ThreadedProgressDialog.cpp
    UtilityModels.cpp
    UtilityViews.cpp
    VSInstallUtils.cpp
    StayAwake.cpp   
    ValidateOpenSSL.cpp
    SABUtilsResources.cpp
)

set(qtproject_CPPMOC_SRCS
)

set(qtproject_H
    AutoWaitCursor.h
    BackgroundFileCheck.h
    ButtonEnabler.h
    WidgetEnabler.h
    DelayComboBox.h
    DelayLineEdit.h
    DelaySpinBox.h
    DoubleProgressDlg.h
    ImageScrollBar.h
    MD5.h
    SpinBox64.h
    SpinBox64U.h
    ScrollMessageBox.h
    CollapsableGroupBox.h
    ThreadedProgressDialog.h
    UtilityModels.h
    UtilityViews.h
    MenuBarEx.h
    SelectFileUrl.h
)

set(project_H
    AutoFetch.h
    ConsoleUtils.h
    utils.h
    FileCompare.h
    FileUtils.h
    FromString.h
    JsonUtils.h
    SpinBox64_StepType.h
    QtUtils.h
    HashUtils.h
    EnumUtils.h
    StringComparisonClasses.h
    StringUtils.h
    WordExp.h
    QtDumper.h
    RevertValue.h
    VSInstallUtils.h
    StayAwake.h   
    ValidateOpenSSL.h
    SABUtilsExport.h
    SABUtilsResources.h
)

set(qtproject_UIS
    ScrollMessageBox.ui
)


set(qtproject_QRC
    resources/SABUtils.qrc
)

file(GLOB qtproject_QRC_SOURCES "resources/*")

if ( BIFSUPPORT )
    set(qtproject_SRCS
        ${qtproject_SRCS}
        BIFFile.cpp
        BIFModel.cpp
        BIFWidget.cpp
    )
    set(qtproject_H
        ${qtproject_H}
        BIFModel.h
        BIFWidget.h
    )
    set(project_H
        ${project_H}
        BIFFile.h
    )
    set(qtproject_UIS
        ${qtproject_UIS}
        BIFWidget.ui
    )

    set(qtproject_QRC
        ${qtproject_QRC}
        BIFPlayerResources/BIFPlayerResources.qrc
    )

    file(GLOB tmp "BIFPlayerResources/*")
    SET( qtproject_QRC_SOURCES
        ${qtproject_QRC_SOURCES}
        ${tmp}
    )
endif()


if ( GIFSUPPORT )
    set(qtproject_SRCS
        ${qtproject_SRCS}
        GIFWriter.cpp
        GIFWriterDlg.cpp
    )
    set(qtproject_H
        ${qtproject_H}
        GIFWriterDlg.h
    )
    set(project_H
        ${project_H}
        GIFWriter.h
    )
    set(qtproject_UIS
        ${qtproject_UIS}
        GIFWriterDlg.ui
    )
endif()

if ( MKVUTILS )
    set(qtproject_SRCS
        ${qtproject_SRCS}
        MKVUtils.cpp
        SetMKVTags.cpp
    )
    set(qtproject_H
        ${qtproject_H}
        SetMKVTags.h
    )
    set(project_H
        ${project_H}
        MKVUtils.h
    )
    set(qtproject_UIS
        ${qtproject_UIS}
        SetMKVTags.ui
    )
endif()

SET( project_pub_DEPS
     Qt5::Concurrent
     Qt5::Network
     MKVReader
     ${MEDIAINFOLIB}
)

