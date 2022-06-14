# The MIT License( MIT )
#
# Copyright( c ) 2020-2021 Scott Aron Bloom
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files( the "Software" ), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

get_filename_component( QTDIR ${DEPLOYQT_EXECUTABLE} DIRECTORY )

set ( DEBUG_PATH 
    "%PATH%"
    "$<TARGET_FILE_DIR:SABUtils>"
    "${QTDIR}"
    )
if ( MKVUTILS )
    set ( DEBUG_PATH 
        ${DEBUG_PATH}
        "$<TARGET_FILE_DIR:mediainfo>"
    )
endif()

set( testProjectName "" )
SAB_UNIT_TEST(Utils
    Test.cpp
    "SABUtils;gmock;Qt5::Core"
    testProjectName
    )

message( STATUS "DEBUG_PATH=${DEBUG_PATH}" )

message( STATUS "testProjectName=${testProjectName}" )
set_target_properties( ${testProjectName} PROPERTIES 
                                    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${testProjectName}>" 
                                    VS_DEBUGGER_COMMAND "$<TARGET_FILE:${testProjectName}>" 
                                    VS_DEBUGGER_ENVIRONMENT "PATH=${DEBUG_PATH}" 
                     )
