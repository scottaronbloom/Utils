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

set(_PROJECT_NAME bif)
set(USE_QT TRUE)
set(FOLDER_NAME SharedLibs)

IF(WIN32)
    set( OS_SRCS ../MoveToTrash_win.cpp ../WindowsError.cpp )
    set( OS_HEADERS  )
ELSE()
    set( OS_SRCS ../MoveToTrash_linux.cpp)
ENDIF()

set(qtproject_SRCS
    BIFPlugin.cpp
    ../BIFFile.cpp
    ../FindAllFiles.cpp
    ../BackupFile.cpp
    ../MoveToTrash.cpp
    ../FileUtils_Remove.cpp
    BIFIOHandler.cpp
    ${OS_SRCS}
)

set(qtproject_CPPMOC_SRCS
)

set(qtproject_H
    BIFPlugin.h
)

set(project_H
    BIFIOHandler.h
    ../FileUtils.h
    ../BackupFile.h
    ../BIFFile.h
)

set(qtproject_UIS
)


set(qtproject_QRC
)
