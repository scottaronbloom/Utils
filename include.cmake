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
    BIFFile.cpp
    BIFModel.cpp
    BIFWidget.cpp
    ButtonEnabler.cpp
    DelayComboBox.cpp
    DelayLineEdit.cpp
    DelaySpinBox.cpp
    utils.cpp
    FileUtils.cpp
    FromString.cpp
    JsonUtils.cpp
    MD5.cpp
    MenuBarEx.cpp
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
)

set(qtproject_CPPMOC_SRCS
)

set(qtproject_H
    AutoWaitCursor.h
    BIFFile.h
    BIFModel.h
    BIFWidget.h
    ButtonEnabler.h
    DelayComboBox.h
    DelayLineEdit.h
    DelaySpinBox.h
    MD5.h
    SpinBox64.h
    SpinBox64U.h
    ScrollMessageBox.h
    CollapsableGroupBox.h
    ThreadedProgressDialog.h
    UtilityModels.h
    UtilityViews.h
    MenuBarEx.h
)

set(project_H
    AutoFetch.h
    utils.h
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
    VSInstallUtils.h
)

set(qtproject_UIS
    ScrollMessageBox.ui
)


set(qtproject_QRC
)

