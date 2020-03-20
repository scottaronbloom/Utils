#
# (c) Copyright 2004 - 2019 Blue Pearl Software Inc.
# All rights reserved.
#
# This source code belongs to Blue Pearl Software Inc.
# It is considered trade secret and confidential, and is not to be used
# by parties who have not received written authorization
# from Blue Pearl Software Inc.
#
# Only authorized users are allowed to use, copy and modify
# this software provided that the above copyright notice
# remains in all copies of this software.
#
#
# $Author: scott $ - $Revision: 56205 $ - $Date: 2019-11-21 10:23:08 -0800 (Thu, 21 Nov 2019) $
# $HeadURL: http://bpsvn/svn/trunk/CoreApp/dr/UnitTests/CMakeLists.txt $
#
#
#

SAB_UNIT_TEST(Utils
    Test.cpp
    "${QT_LIBRARIES};SABUtils;gmock"
    )

