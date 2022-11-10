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

#include "FromString.h"
#include "StringUtils.h"
#include <float.h>
#include <algorithm>
#include <cerrno>
#include <limits.h>
#include <cstring>
#include <string>
#include <cctype>

namespace NSABUtils
{
    bool restIsSpace( const std::string & str, size_t sz )
    {
        auto pos = std::find_if( str.begin() + sz, str.end(), []( char value ) { return !std::isspace( value ); } );
        return pos == str.end();
    }

    bool fromString( long & retVal, const std::string & arg, int base )
    {
        retVal = 0;
        if ( arg.empty() )
            return false;

        size_t sz;
        try
        {
            retVal = std::stol( arg, &sz, base );
        }
        catch ( ... )
        {
            return false;
        }
        auto aOK = restIsSpace( arg, sz );

        if ( !aOK )
            retVal = 0;

        return aOK;
    }

    bool fromString( long & retVal, const std::string & arg )
    {
        return fromString( retVal, arg, 10 );
    }

    bool fromString( long & retVal, const char * arg, int base )
    {
        return fromString( retVal, std::string( arg ), base );
    }

    bool fromString( int & retVal, const std::string & arg, int base )
    {
        long tmp;
        bool aOK = fromString( tmp, arg, base );
        if ( !aOK )
            return false;
        if ( ( tmp > INT_MAX ) || ( tmp < INT_MIN ) )
            return false;

        retVal = static_cast<int>( tmp );
        return true;
    }

    bool fromString( int & retVal, const std::string & arg )
    {
        return fromString( retVal, arg, 10 );
    }


    bool fromString( int & retVal, const char * arg, int base )
    {
        return fromString( retVal, std::string( arg ), base );
    }

    bool fromString( int & retVal, const char * arg )
    {
        return fromString( retVal, std::string( arg ) );
    }

    bool fromString( double & retVal, const std::string & arg )
    {
        retVal = 0;
        if ( arg.empty() )
            return false;
        std::string::size_type sz;
        try
        {
            retVal = std::stod( arg, &sz );
        }
        catch ( ... )
        {
            return false;
        }
        bool aOK = restIsSpace( arg, sz );
        if ( !aOK )
            retVal = 0.0;
        return aOK;
    }

    bool fromString( double & retVal, const char * arg )
    {
        return fromString( retVal, std::string( arg ) );
    }

    bool fromString( float & retVal, const std::string & arg )
    {
        double tmp;
        bool aOK = fromString( tmp, arg );
        if ( !aOK )
            return false;
        if ( ( tmp > FLT_MAX ) || ( tmp < FLT_MIN ) )
            return false;

        retVal = static_cast<float>( tmp );
        return true;
    }

    bool fromString( float & retVal, const char * arg )
    {
        return fromString( retVal, std::string( arg ) );
    }

    bool fromString( bool & retVal, const std::string & arg )
    {
        retVal = false; // default value;
        if ( arg.length() == 0 )
            return false;

        double doubleVal = 0.0;
        bool isDouble = fromString( doubleVal, arg );
        if ( isDouble )
        {
            retVal = doubleVal != 0.0;
            return true;
        }

        int intVal = 0;
        bool isInt = fromString( intVal, arg );
        if ( isInt )
        {
            retVal = intVal != 0;
        }

        std::string lc = arg;
        std::transform( lc.begin(), lc.end(), lc.begin(), ::tolower );

        retVal = ( lc != "false" ) && ( lc != "no" ) && ( lc != "off" ) && ( lc != "n" ) && ( lc != "f" );
        return true;
    }

    bool fromString( bool & retVal, const char * arg )
    {
        return fromString( retVal, std::string( arg ) );
    }
}

