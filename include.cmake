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

set(_PROJECT_NAME SABUtils)
set(USE_QT TRUE)
if ( SABUTILS_DLL )
    set(FOLDER_NAME SharedLibs)
else()
    set(FOLDER_NAME Libs)
endif()

IF(WIN32)
        set( OS_SRCS 
            SystemInfo_win.cpp 
            MoveToTrash_win.cpp 
            ForceUnbufferedProcessModifier.cpp 
            ConsoleUtils.cpp 
            WindowsError.cpp 
        )
        set( OS_HEADERS 
            ForceUnbufferedProcessModifier.h 
            ConsoleUtils.h 
            WindowsError.h 
        )
ELSE()
        set( OS_SRCS 
            SystemInfo_linux.cpp 
            MoveToTrash_linux.cpp
        )
ENDIF()

set(qtproject_SRCS
    FFMpegFormats.cpp
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
    BackupFile.cpp
    FileBasedCache.cpp
    FileUtils.cpp
    FileUtils_Remove.cpp
    FindAllFiles.cpp
    FileSIDInfo.cpp
    FromString.cpp
    GPUDetect.cpp
    HyperLinkLineEdit.cpp
    ImageScrollBar.cpp
    JsonUtils.cpp
    LineEditWithSuffix.cpp
    MD5.cpp
    MoveToTrash.cpp
    MenuBarEx.cpp
    QtDumper.cpp
    QtUtils.cpp
    RegExUtils.cpp
    SABUtilsResources.cpp
    ScrollMessageBox.cpp
    SelectFileUrl.cpp
    SpinBox64.cpp
    SpinBox64U.cpp
    StayAwake.cpp
    StringComparisonClasses.cpp
    StringUtils.cpp
    SystemInfo.cpp
    UtilityModels.cpp
    UtilityViews.cpp
    utils.cpp
    uiUtils.cpp
    WidgetEnabler.cpp
    WidgetChanged.cpp
    WordExp.cpp
    SystemInfo.cpp
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
    HyperLinkLineEdit.h
    ImageScrollBar.h
    LineEditWithSuffix.h
    MD5.h
    MenuBarEx.h
    ScrollMessageBox.h
    SelectFileUrl.h
    SpinBox64.h
    SpinBox64U.h
    UtilityModels.h
    UtilityViews.h
    WidgetEnabler.h
    ${Q_OS_HEADERS}
)

set(project_H
    AutoFetch.h
    CantorHash.h
    EnumUtils.h
    FileCompare.h
    FFMpegFormats.h
    BackupFile.h
    FileUtils.h
    FileSIDInfo.h
    FileBasedCache.h
    FromString.h
    HashUtils.h
    GPUDetect.h
    QtHashUtils.h
    JsonUtils.h
    MetaUtils.h
    MoveToTrash.h
    QtDumper.h
    QtUtils.h
    RevertValue.h
    RegExUtils.h
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
    ${OS_HEADERS}
)

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
        MediaInfo.cpp
        SetMKVTags.cpp
    )
    set(qtproject_H
        ${qtproject_H}
        SetMKVTags.h
        MediaInfo.h
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
        Qt5::Multimedia
    )
endif()

if ( QAXOBJECT_SUPPORT )
    set(qtproject_SRCS
        ${qtproject_SRCS}
        VSInstallUtils.cpp
    )
    set(project_H
        ${project_H}
        VSInstallUtils.h
    )
    set(qtproject_H
        ${qtproject_H}
    )
    set(QAXOBJECT_LIB
        Qt5::AxContainer
    )
endif()

if ( QNETWORK_SUPPORT )
    set(qtproject_SRCS
        ${qtproject_SRCS}
        DownloadFile.cpp
        GitHubGetVersions.cpp
        ValidateOpenSSL.cpp
    )
    set(project_H
        ${project_H}
    )
    set(qtproject_H
        ${qtproject_H}
        DownloadFile.h
        GitHubGetVersions.h
        ValidateOpenSSL.h
    )
    set(QNETWORK_LIB
        Qt5::Network
    )
endif()

if ( QCONCURRENT_SUPPORT )
    set(qtproject_SRCS
        ${qtproject_SRCS}
        ThreadedProgressDialog.cpp
    )
    set(project_H
        ${project_H}
    )
    set(qtproject_H
        ${qtproject_H}
        ThreadedProgressDialog.h
    )
    set(QCONCURRENT_LIB
        Qt5::Concurrent
    )
endif()

SET( project_pub_DEPS
     ${MKVREADERLIB}
     ${MEDIAINFOLIB}
     ${QAXOBJECT_LIB}
     ${QNETWORK_LIB}
     ${QCONCURRENT_LIB}
)

