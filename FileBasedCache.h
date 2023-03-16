// The MIT License( MIT )
//
// Copyright( c ) 2020-2022 Scott Aron Bloom
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
#ifndef __FILEBASEDCACHE_H
#define __FILEBASEDCACHE_H

#include "SABUtilsExport.h"

#include <QDateTime>
#include <QFileInfo>
#include <unordered_map>
#include <cstdint>
#include <memory>

namespace NSABUtils
{
    struct SABUTILS_EXPORT SFileBasedCacheNode
    {
        explicit SFileBasedCacheNode( const QString & path );
        explicit SFileBasedCacheNode( const QFileInfo &fileInfo );

        bool operator==( const SFileBasedCacheNode &rhs ) const;

        std::size_t hash() const;

        void setPathOnlySearch( bool value ) { fPathOnlySearch = value;  }
        bool pathOnlySearch() const { return fPathOnlySearch; }
    private:
        QFileInfo fFileInfo;
        QDateTime fDateTime;
        uint64_t fSize{ 0 };
        bool fPathOnlySearch{ false };
    };

    //template< typename T >
    //class CHasFileName
    //{
    //private:
    //    using TYes = char[ 1 ];
    //    using TNo  = char[ 2 ];

    //    template< typename C >
    //    static TYes & test( decltype( &C::fileName) );

    //    template< typename C >
    //    static TNo & test( ... );

    //public:
    //    enum
    //    {
    //        value = sizeof( test< T >( 0 ) ) == sizeof( TYes )
    //    };
    //};

    template< typename, typename = void >
    constexpr bool has_fileNamePtr{};
    
    template< typename T >
    constexpr bool has_fileNamePtr< T, std::void_t< decltype( std::declval< T >()->fileName() ) > > = true;

    // a cache that can be used for items that are filebased
    // but a miss should happen if the file has changed (checks size and timestamp)
    template< typename T >
    class CFileBasedCache
    {
    public:
        CFileBasedCache(){}

        T find( const QString &path ) const 
        {
            return find( QFileInfo( path ) );
        }
        T find( const QFileInfo & fi ) const
        {
            auto node = NSABUtils::SFileBasedCacheNode( fi );
            auto pos = fCache.find( node );
            if ( pos == fCache.end() )
                return {};
            return ( *pos ).second;
        }

        bool contains( const QString &path, bool pathOnlySearch=false ) const { return contains( QFileInfo( path ), pathOnlySearch ); }
        bool contains( const QFileInfo &fi, bool pathOnlySearch = false ) const
        {
            auto node = NSABUtils::SFileBasedCacheNode( fi );
            node.setPathOnlySearch( pathOnlySearch );
            auto pos = fCache.find( node );
            return ( pos != fCache.end() );
        }

        
        typename std::enable_if < has_fileNamePtr< T >, void >::type 
        add( const T &object )
        {
            return add( object->fileName(), object );
        }

        void add( const QString &path, const T &object )
        {
            if ( contains( path, true ) )
            {
                clear( path, true );
            }
            fCache[ NSABUtils::SFileBasedCacheNode( path ) ] = object;
        }

    private:
        void clear( const QString &path, bool pathOnlySearch )
        {
            auto node = NSABUtils::SFileBasedCacheNode( path );
            node.setPathOnlySearch( pathOnlySearch );
            auto pos = fCache.find( node );
            if ( pos != fCache.end() )
                fCache.erase( pos );
        }
        std::unordered_map< SFileBasedCacheNode, T > fCache;
    };
}

namespace std
{
    template<>
    struct hash< NSABUtils::SFileBasedCacheNode >
    {
        std::size_t operator()( const NSABUtils::SFileBasedCacheNode & key ) const { return key.hash(); }
    };

    template<>
    struct hash< const NSABUtils::SFileBasedCacheNode * >
    {
        std::size_t operator()( NSABUtils::SFileBasedCacheNode *ii ) const
        {
            if ( ii )
                return ii->hash();
            else
                return 0;
        }
    };

    template<>
    struct hash< std::shared_ptr< NSABUtils::SFileBasedCacheNode > >
    {
        std::size_t operator()( std::shared_ptr< NSABUtils::SFileBasedCacheNode > ii ) const { return hash< NSABUtils::SFileBasedCacheNode * >()( ii.get() ); }
    };
}

#endif
