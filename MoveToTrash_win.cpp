// The MIT License( MIT )
//
// Copyright( c ) 2022 Scott Aron Bloom
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

#include "MoveToTrash.h"

#include <Shobjidl.h> //Required for IFileOperation Interface
#include <shellapi.h> //Required for Flags set in "SetOperationFlags"

#include <windows.h>
#include <shellapi.h>

#include <QString>
#include <QFileInfo>
#include <iostream>

#include <QDir>
#include <functional>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <QDebug>
namespace NSABUtils
{
    namespace NFileUtils
    {
        enum SINK_TYPE_ENUM
        {
            SINK_TYPE_STARTOPERATIONS = 0,
            SINK_TYPE_FINISHOPERATIONS,
            SINK_TYPE_PRECOPYITEM,
            SINK_TYPE_POSTCOPYITEM,
            SINK_TYPE_UPDATEPROGRESS
        };

        const PCWSTR g_rgpszSinkType[] =
        {
        L"StartOperations",
        L"FinishOperations",
        L"PreCopyItem",
        L"PostCopyItem",
        L"UpdateProgress"
        };

        class CFileOpProgSinkApp : public IFileOperationProgressSink
        {
        public:
            CFileOpProgSinkApp( std::shared_ptr< SRecycleOptions > options ) :
                _cRef( 1 ),
                _hwnd( NULL ),
                _hwndLV( NULL ),
                fOptions( options )
            {
            }

            // IUnknown
            IFACEMETHODIMP QueryInterface( REFIID riid, void ** ppv )
            {
                static const QITAB qit[] =
                {
                QITABENT( CFileOpProgSinkApp, IFileOperationProgressSink ),
                {0},
                };
                return QISearch( this, qit, riid, ppv );
            }

            IFACEMETHODIMP_( ULONG ) AddRef()
            {
                return InterlockedIncrement( &_cRef );
            }

            IFACEMETHODIMP_( ULONG ) Release()
            {
                ULONG cRef = InterlockedDecrement( &_cRef );
                if ( 0 == cRef )
                {
                    delete this;
                }
                return cRef;
            }

            QString getPathNameForItem( IShellItem * psiItem, IShellItem * psiDestinationFolder );

            IFACEMETHODIMP StartOperations();
            IFACEMETHODIMP FinishOperations( HRESULT hrResult );

            IFACEMETHODIMP PreRenameItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, PCWSTR /*pszNewName*/ );
            IFACEMETHODIMP PostRenameItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, PCWSTR /*pszNewName*/, HRESULT /*hrRename*/, IShellItem * /*psiNewlyCreated*/ );

            IFACEMETHODIMP PreMoveItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/ );
            IFACEMETHODIMP PostMoveItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/, HRESULT /*hrNewName*/, IShellItem * /*psiNewlyCreated*/ );

            IFACEMETHODIMP PreCopyItem( DWORD dwFlags, IShellItem * psiItem, IShellItem * psiDestinationFolder, PCWSTR pszNewName );
            IFACEMETHODIMP PostCopyItem( DWORD dwFlags, IShellItem * psiItem, IShellItem * psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy, IShellItem * psiNewlyCreated );

            IFACEMETHODIMP PreDeleteItem( DWORD /*dwFlags*/, IShellItem * psiItem );

            bool verbose();

            IFACEMETHODIMP PostDeleteItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, HRESULT /*hrDelete*/, IShellItem * /*psiNewlyCreated*/ );

            IFACEMETHODIMP PreNewItem( DWORD /*dwFlags*/, IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/ );
            IFACEMETHODIMP PostNewItem( DWORD /*dwFlags*/, IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/, PCWSTR /*pszTemplateName*/, DWORD /*dwFileAttributes*/, HRESULT /*hrNew*/, IShellItem * /*psiNewItem*/ );

            IFACEMETHODIMP UpdateProgress( UINT iWorkTotal, UINT iWorkSoFar );

            IFACEMETHODIMP ResetTimer();
            IFACEMETHODIMP PauseTimer();
            IFACEMETHODIMP ResumeTimer();

        private:
            ~CFileOpProgSinkApp() {}
            std::shared_ptr< SRecycleOptions > fOptions;

            long   _cRef;
            HWND   _hwnd;
            HWND   _hwndLV;
        };

        QString CFileOpProgSinkApp::getPathNameForItem( IShellItem * psiItem, IShellItem * psiDestinationFolder )
        {
            QString retVal;
            PWSTR pszItem;
            HRESULT hr = psiItem->GetDisplayName( SIGDN_FILESYSPATH, &pszItem );
            retVal = QString::fromWCharArray( pszItem );
            qDebug() << retVal;

            if ( SUCCEEDED( hr ) && psiDestinationFolder )
            {
                PWSTR pszDest;
                hr = psiDestinationFolder->GetDisplayName( SIGDN_FILESYSPATH, &pszDest );
                if ( SUCCEEDED( hr ) )
                {
                    retVal = QString::fromWCharArray( pszDest );
                    qDebug() << retVal;
                    CoTaskMemFree( pszDest );
                }
                CoTaskMemFree( pszItem );
            }
            return retVal;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::StartOperations()
        {
            if ( verbose() )
                std::cout << "Starting operation";
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::FinishOperations( HRESULT hr )
        {
            if ( verbose() )
                std::cout << "Finished";
            return SUCCEEDED( hr );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PreRenameItem( DWORD /*dwFlags*/, IShellItem * psiItem, PCWSTR pszNewName )
        {
            if ( verbose() )
            {
                auto pathName = getPathNameForItem( psiItem, nullptr );
                std::cout << "Renaming " << ( QFileInfo( pathName ).isFile() ? "file" : "directory" ) << " '" << pathName.toStdString() << "' to '" << pszNewName << "'." << std::endl;
            }

            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PostRenameItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, PCWSTR /*pszNewName*/, HRESULT /*hrRename*/, IShellItem * /*psiNewlyCreated*/ )
        {
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PreMoveItem( DWORD /*dwFlags*/, IShellItem * psiItem, IShellItem * psiDestinationFolder, PCWSTR pszNewName )
        {
            if ( verbose() )
            {
                auto pathName = getPathNameForItem( psiItem, nullptr );
                auto destFolder = getPathNameForItem( psiDestinationFolder, nullptr );
                std::cout << "Renaming " << ( QFileInfo( pathName ).isFile() ? "file" : "directory" ) << " '" << pathName.toStdString() << "' to directory '" << destFolder.toStdString() << "' with name '" << pszNewName << "'." << std::endl;
            }
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PostMoveItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/, HRESULT /*hrNewName*/, IShellItem * /*psiNewlyCreated*/ )
        {
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PreCopyItem( DWORD /*dwFlags*/, IShellItem * psiItem, IShellItem * psiDestinationFolder, PCWSTR )
        {
            auto pathName = getPathNameForItem( psiItem, psiDestinationFolder );
            qDebug() << pathName;
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PostCopyItem( DWORD /*dwFlags*/, IShellItem * psiItem, IShellItem * psiDestinationFolder, PCWSTR, HRESULT /*hrCopy*/, IShellItem * )
        {
            PWSTR pszItem;
            HRESULT hr = psiItem->GetDisplayName( SIGDN_FILESYSPATH, &pszItem );
            if ( SUCCEEDED( hr ) )
            {
                PWSTR pszDest;
                hr = psiDestinationFolder->GetDisplayName( SIGDN_FILESYSPATH, &pszDest );
                if ( SUCCEEDED( hr ) )
                {
                    qDebug() << pszDest;
                    CoTaskMemFree( pszDest );
                }
                CoTaskMemFree( pszItem );
            }
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PreDeleteItem( DWORD /*dwFlags*/, IShellItem * psiItem )
        {
            if ( verbose() )
            {
                auto pathName = getPathNameForItem( psiItem, nullptr );
                std::cout << "Recycling " << ( QFileInfo( pathName ).isFile() ? "file" : "directory" ) << " '" << pathName.toStdString() << "'" << std::endl;
            }

            return S_OK;
        }

        bool CFileOpProgSinkApp::verbose()
        {
            return fOptions && fOptions->fVerbose;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PostDeleteItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, HRESULT /*hrDelete*/, IShellItem * /*psiNewlyCreated*/ )
        {
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PreNewItem( DWORD /*dwFlags*/, IShellItem * psiDestinationFolder, PCWSTR pszNewName )
        {
            if ( verbose() )
            {
                auto destFolder = getPathNameForItem( psiDestinationFolder, nullptr );
                std::cout << "Creating '" << pszNewName << "' in directory '" << destFolder.toStdString() << "'.'" << std::endl;
            }
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PostNewItem( DWORD /*dwFlags*/, IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/, PCWSTR /*pszTemplateName*/, DWORD /*dwFileAttributes*/, HRESULT /*hrNew*/, IShellItem * /*psiNewItem*/ )
        {
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::UpdateProgress( UINT iWorkTotal, UINT iWorkSoFar )
        {
            if ( verbose() )
            {
                std::cout << "Status " << iWorkSoFar << " of " << iWorkTotal << std::endl;
            }
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::ResetTimer()
        {
            if ( verbose() )
            {
                std::cout << "Resetting timer" << std::endl;
            }
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PauseTimer()
        {
            if ( verbose() )
            {
                std::cout << "Pausing timer" << std::endl;
            }
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::ResumeTimer()
        {
            if ( verbose() )
            {
                std::cout << "Resuming timer" << std::endl;
            }
            return S_OK;
        }

        // List view column labels
        QString getWindowsError( int errorCode )
        {
            QString ret;
#ifndef Q_OS_WINRT
            wchar_t * string = 0;
            FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                            nullptr,
                            errorCode,
                            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                            (LPWSTR)&string,
                            0,
                            nullptr );
            ret = QString::fromWCharArray( string );
            LocalFree( (HLOCAL)string );
#else
            wchar_t errorString[ 1024 ];
            FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                           nullptr,
                           errorCode,
                           MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                           (LPWSTR)&errorString,
                           sizeof( errorString ) / sizeof( wchar_t ),
                           nullptr );
            ret = QString::fromWCharArray( errorString );
#endif  // Q_OS_WINRT

            if ( ret.isEmpty() && errorCode == ERROR_MOD_NOT_FOUND )
                ret = QString::fromLatin1( "The specified module could not be found." );
            if ( ret.endsWith( QLatin1String( "\r\n" ) ) )
                ret.chop( 2 );
            if ( ret.isEmpty() )
                ret = QString::fromLatin1( "Unknown error 0x%1." )
                .arg( unsigned( errorCode ), 8, 16, QLatin1Char( '0' ) );
            return ret;
        }

        bool showError( const QString & msg, HRESULT code, std::function< void() > runFunc )
        {
            auto fullMessage = QString( "%1 - %2" ).arg( msg ).arg( getWindowsError( code ) );
            MessageBox( nullptr, fullMessage.toStdWString().c_str(), L"Error", MB_OK | MB_ICONERROR );
            runFunc();
            return false;
        }

        bool moveToTrashImpl( const QString & fileName, std::shared_ptr< SRecycleOptions > options )
        {
            if ( options->fVerbose )
            {
                if ( !QFileInfo( fileName ).exists() )
                    std::cout << "File or Directory '" << fileName.toStdString() << "' does not exist." << std::endl;
            }

            HRESULT hr = CoInitializeEx( nullptr, COINIT_MULTITHREADED );
            if ( FAILED( hr ) )
            {
            //Couldn't initialize COM library - clean up and return
                return showError( "Couldn't initialize COM library", hr,
                                  []()
                                  {
                                      CoUninitialize();
                                  } );
            }
            //Initialize the file operation
            IFileOperation * fileOperation;
            hr = CoCreateInstance( CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_PPV_ARGS( &fileOperation ) );
            if ( FAILED( hr ) )
            {
            //Couldn't CoCreateInstance - clean up and return
                return showError( "Couldn't CoCreateInstance", hr,
                                  []()
                                  {
                                      CoUninitialize();
                                  } );
            }
            hr = fileOperation->SetOperationFlags( FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NO_UI | FOF_NOERRORUI );
            if ( FAILED( hr ) )
            {
            //Couldn't add flags - clean up and return
                return showError( "Couldn't add flags", hr,
                                  [fileOperation]()
                                  {
                                      fileOperation->Release();
                                      CoUninitialize();
                                  } );
            }

            //auto path = QDir::current().absoluteFilePath( fileName ).toStdWString();
            auto path = fileName.toStdWString();

            IShellItem * fileOrFolderItem = nullptr;
            hr = SHCreateItemFromParsingName( path.c_str(), nullptr, IID_PPV_ARGS( &fileOrFolderItem ) );
            if ( FAILED( hr ) )
            {
                return showError( "Couldn't create IShellItem from path", hr,
                                  [fileOrFolderItem, fileOperation]()
                                  {
                                      if ( fileOrFolderItem )
                                          fileOrFolderItem->Release();
                                      fileOperation->Release();
                                      CoUninitialize();
                                  } );
            }

            auto pSync = new CFileOpProgSinkApp( options );

            hr = fileOperation->DeleteItem( fileOrFolderItem, pSync ); //The second parameter is if you want to keep track of progress
            fileOrFolderItem->Release();
            if ( FAILED( hr ) )
            {
                return showError( "Failed to mark file/folder item for deletion", hr,
                                  [fileOrFolderItem, fileOperation, pSync/*, sink*/]()
                                  {
                                      fileOperation->Release();
                                      if ( pSync )
                                          pSync->Release();
                                      CoUninitialize();
                                  } );
            }
            hr = fileOperation->PerformOperations();
            fileOperation->Release();
            if ( pSync )
                pSync->Release();
            CoUninitialize();
            if ( FAILED( hr ) )
            {
                return showError( "Failed to carry out delete", hr,
                                  [fileOrFolderItem, fileOperation]()
                                  {
                                  } );
            }
            return true;
        }
    }
}


