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

IF(WIN32)
        set( OS_SRCS SystemInfo_win.cpp MoveToTrash_win.cpp)
ELSE()
        set( OS_SRCS SystemInfo_linux.cpp MoveToTrash_linux.cpp)
ENDIF()

set(qtproject_SRCS
    GitHubGetVersions.cpp
	DownloadFile.cpp
    AutoFetch.cpp
    AutoWaitCursor.cpp
    BackgroundFileCheck.cpp
    ButtonEnabler.cpp
    CollapsableGroupBox.cpp
    DelayComboBox.cpp
    DelayLineEdit.cpp
    DelaySpinBox.cpp
    DoubleProgressDlg.cpp
    FileCompare.cpp
    FileUtils.cpp
    FromString.cpp
    HyperLinkLineEdit.cpp
    ImageScrollBar.cpp
    JsonUtils.cpp
    MD5.cpp
    MoveToTrash.cpp
    MenuBarEx.cpp
    QtDumper.cpp
    QtUtils.cpp
    SABUtilsResources.cpp
    ScrollMessageBox.cpp
    SelectFileUrl.cpp
    SpinBox64.cpp
    SpinBox64U.cpp
    StayAwake.cpp
    StringComparisonClasses.cpp
    StringUtils.cpp
    SystemInfo.cpp
    ThreadedProgressDialog.cpp
    UtilityModels.cpp
    UtilityViews.cpp
    utils.cpp
    uiUtils.cpp
    ValidateOpenSSL.cpp
    VSInstallUtils.cpp
    WidgetEnabler.cpp
    WidgetChanged.cpp
    WordExp.cpp
    ${OS_SRCS}
)

set(qtproject_CPPMOC_SRCS
)

set(qtproject_H
    AutoWaitCursor.h
    BackgroundFileCheck.h
    ButtonEnabler.h
    CollapsableGroupBox.h
    DelayComboBox.h
    DelayLineEdit.h
    DelaySpinBox.h
    DoubleProgressDlg.h
    GitHubGetVersions.h
	DownloadFile.h
    HyperLinkLineEdit.h
    ImageScrollBar.h
    MD5.h
    MenuBarEx.h
    ScrollMessageBox.h
    SelectFileUrl.h
    SpinBox64.h
    SpinBox64U.h
    ThreadedProgressDialog.h
    UtilityModels.h
    UtilityViews.h
    WidgetEnabler.h
)

set(project_H
    AutoFetch.h
    EnumUtils.h
    FileCompare.h
    FileUtils.h
    FromString.h
    HashUtils.h
    JsonUtils.h
    MoveToTrash.h
    QtDumper.h
    QtUtils.h
    RevertValue.h
    SABUtilsExport.h
    SABUtilsResources.h
    SpinBox64_StepType.h
    StayAwake.h   
    StringComparisonClasses.h
    StringUtils.h
    SystemInfo.h
    utils.h
    uiUtils.h
    WidgetChanged.h
    WordExp.h
    ValidateOpenSSL.h
    VSInstallUtils.h
)

if(WIN32)
    set(qtproject_SRCS ${qtproject_SRCS} ConsoleUtils.cpp)
    set(project_H ${project_H} ConsoleUtils.h)
endif()

set(qtproject_UIS
    ScrollMessageBox.ui
	DownloadFile.ui
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
    set(MKVREADERLIB  
        MKVReader
    )
endif()

SET( project_pub_DEPS
     Qt5::Concurrent
     Qt5::Network
     ${MKVREADERLIB}
     ${MEDIAINFOLIB}
)

