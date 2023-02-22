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
#include "FileUtils.h"

#include <windows.h>

#include <Shobjidl.h>   //Required for IFileOperation Interface
#include <shellapi.h>   //Required for Flags set in "SetOperationFlags"
#include <Shlwapi.h>
#pragma comment( lib, "Shlwapi.lib" )

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include <iostream>
#include <functional>

namespace NSABUtils
{
    namespace NFileUtils
    {
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
            IFACEMETHODIMP QueryInterface( REFIID riid, void **ppv )
            {
                static const QITAB qit[] = {
                    QITABENT( CFileOpProgSinkApp, IFileOperationProgressSink ),
                    { 0 },
                };
                return QISearch( this, qit, riid, ppv );
            }

            IFACEMETHODIMP_( ULONG ) AddRef() { return InterlockedIncrement( &_cRef ); }

            IFACEMETHODIMP_( ULONG ) Release()
            {
                ULONG cRef = InterlockedDecrement( &_cRef );
                if ( 0 == cRef )
                {
                    delete this;
                }
                return cRef;
            }

            QString getPathNameForItem( IShellItem *psiItem ) const;

            bool verbose() const { return fOptions && fOptions->fVerbose; }
            const char *statusPrefix( HRESULT hrRename ) const { return ( SUCCEEDED( hrRename ) ? "" : "Error: " ); }

            QString replaceSource( const QString &msg, IShellItem *psiItem ) const
            {
                auto pathName = getPathNameForItem( psiItem );
                if ( !pathName.isEmpty() )
                {
                    pathName = QString( "%1 '%2'" ).arg( QFileInfo( pathName ).isFile() ? "file" : "directory" ).arg( pathName );
                }
                auto realMsg = msg;
                realMsg.replace( "<SOURCE>", pathName );
                return realMsg;
            }

            IFACEMETHODIMP startStatus( const QString &msg, IShellItem *srcItem = nullptr ) const
            {
                if ( verbose() )
                    std::cout << replaceSource( msg, srcItem ).toStdString() << std::endl;
                return S_OK;
            }

            IFACEMETHODIMP returnFinishedStatus( HRESULT hr, const QString &msg, IShellItem *srcItem = nullptr ) const
            {
                if ( verbose() )
                {
                    ( SUCCEEDED( hr ) ? std::cout : std::cerr ) << statusPrefix( hr ) << replaceSource( msg, srcItem ).toStdString() << std::endl;
                    if ( !SUCCEEDED( hr ) )
                    {
                        ( SUCCEEDED( hr ) ? std::cout : std::cerr ) << statusPrefix( hr ) << "    " << getWindowsError( hr ).toStdString() << std::endl;
                    }
                }
                return hr;
            }

            IFACEMETHODIMP StartOperations();
            IFACEMETHODIMP FinishOperations( HRESULT hrResult );

            IFACEMETHODIMP PreRenameItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, PCWSTR /*pszNewName*/ );
            IFACEMETHODIMP PostRenameItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, PCWSTR /*pszNewName*/, HRESULT /*hrRename*/, IShellItem * /*psiNewlyCreated*/ );

            IFACEMETHODIMP PreMoveItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/ );
            IFACEMETHODIMP PostMoveItem( DWORD /*dwFlags*/, IShellItem * /*psiItem*/, IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/, HRESULT /*hrNewName*/, IShellItem * /*psiNewlyCreated*/ );

            IFACEMETHODIMP PreCopyItem( DWORD dwFlags, IShellItem *psiItem, IShellItem *psiDestinationFolder, PCWSTR pszNewName );
            IFACEMETHODIMP PostCopyItem( DWORD dwFlags, IShellItem *psiItem, IShellItem *psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy, IShellItem *psiNewlyCreated );

            IFACEMETHODIMP PreDeleteItem( DWORD /*dwFlags*/, IShellItem *psiItem );
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

            long _cRef;
            HWND _hwnd;
            HWND _hwndLV;
        };

        QString CFileOpProgSinkApp::getPathNameForItem( IShellItem *psiItem ) const
        {
            if ( !psiItem )
                return {};
            QString retVal;
            PWSTR pszItem;
            HRESULT hr = psiItem->GetDisplayName( SIGDN_FILESYSPATH, &pszItem );
            if ( SUCCEEDED( hr ) )
                retVal = QString::fromWCharArray( pszItem );
            CoTaskMemFree( pszItem );
            return retVal;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::StartOperations()
        {
            return S_OK;   // startStatus( "Starting operation" );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::FinishOperations( HRESULT hr )
        {
            return hr;   // returnFinishedStatus( hr, "Finished operation" );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PreRenameItem( DWORD /*dwFlags*/, IShellItem *psiItem, PCWSTR pszNewName )
        {
            return startStatus( QString( "Renaming <SOURCE> to '%1'." ).arg( pszNewName ), psiItem );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PostRenameItem( DWORD /*dwFlags*/, IShellItem *psiItem, PCWSTR pszNewName, HRESULT hrRename, IShellItem *psiNewlyCreated )
        {
            auto pathName = getPathNameForItem( psiItem );
            return returnFinishedStatus( hrRename, QString( "Renamed <SOURCE> to '%1'.  Final path name: '%2'." ).arg( pszNewName ).arg( getPathNameForItem( psiNewlyCreated ) ) );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PreMoveItem( DWORD /*dwFlags*/, IShellItem *psiItem, IShellItem *psiDestinationFolder, PCWSTR pszNewName )
        {
            return startStatus( QString( "Moving <SOURCE> to directory '%1' as '%3'." ).arg( getPathNameForItem( psiDestinationFolder ) ).arg( pszNewName ), psiItem );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PostMoveItem( DWORD /*dwFlags*/, IShellItem *psiItem, IShellItem *psiDestinationFolder, PCWSTR pszNewName, HRESULT hrNewName, IShellItem *psiNewlyCreated )
        {
            return returnFinishedStatus(
                hrNewName, QString( "Moved <SOURCE> to directory '%1' as '%3'. Final path name '%4'." ).arg( getPathNameForItem( psiDestinationFolder ) ).arg( pszNewName ).arg( getPathNameForItem( psiNewlyCreated ) ), psiItem );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PreCopyItem( DWORD /*dwFlags*/, IShellItem *psiItem, IShellItem *psiDestinationFolder, PCWSTR pszNewName )
        {
            return startStatus( QString( "Copying <SOURCE> to directory '%1' as '%2'." ).arg( getPathNameForItem( psiDestinationFolder ) ).arg( pszNewName ), psiItem );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PostCopyItem( DWORD /*dwFlags*/, IShellItem *psiItem, IShellItem *psiDestinationFolder, PCWSTR pszNewName, HRESULT hrCopy, IShellItem *psiNewlyCreated )
        {
            return returnFinishedStatus( hrCopy, QString( "Copied <SOURCE> to directory '%1' as '%2'. Final path name '%4'." ).arg( getPathNameForItem( psiDestinationFolder ) ).arg( pszNewName ).arg( getPathNameForItem( psiNewlyCreated ) ),
                                         psiItem );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PreDeleteItem( DWORD dwFlags, IShellItem *psiItem )
        {
            return startStatus( QString( "%1 <SOURCE>." ).arg( ( dwFlags & TSF_DELETE_RECYCLE_IF_POSSIBLE ) ? "Recycling" : "Deleting" ), psiItem );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PostDeleteItem( DWORD dwFlags, IShellItem *psiItem, HRESULT hrDelete, IShellItem * /*psiNewlyCreated*/ )
        {
            return returnFinishedStatus( hrDelete, QString( "%1 <SOURCE>." ).arg( ( dwFlags & TSF_DELETE_RECYCLE_IF_POSSIBLE ) ? "Recycled" : "Deleted" ), psiItem );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PreNewItem( DWORD /*dwFlags*/, IShellItem *psiDestinationFolder, PCWSTR pszNewName )
        {
            return startStatus( QString( "Creating '%1' in directory '%2'." ).arg( pszNewName ).arg( getPathNameForItem( psiDestinationFolder ) ) );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PostNewItem( DWORD /*dwFlags*/, IShellItem *psiDestinationFolder, PCWSTR pszNewName, PCWSTR pszTemplateName, DWORD /*dwFileAttributes*/, HRESULT hrNew, IShellItem *psiNewItem )
        {
            return returnFinishedStatus(
                hrNew, QString( "Created '%1' in directory '%2'. Template Name: %3.  Final Path '%4'." ).arg( pszNewName ).arg( getPathNameForItem( psiDestinationFolder ) ).arg( pszTemplateName ).arg( getPathNameForItem( psiNewItem ) ) );
        }

        IFACEMETHODIMP CFileOpProgSinkApp::UpdateProgress( UINT iWorkTotal, UINT iWorkSoFar )
        {
            if ( verbose() )
            {
                if ( iWorkTotal )
                    std::cout << "Status " << iWorkSoFar << " of " << iWorkTotal << std::endl;
            }
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::ResetTimer()
        {
            // if ( verbose() )
            //{
            //     std::cout << "Resetting timer" << std::endl;
            // }
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::PauseTimer()
        {
            // if ( verbose() )
            //{
            //     std::cout << "Pausing timer" << std::endl;
            // }
            return S_OK;
        }

        IFACEMETHODIMP CFileOpProgSinkApp::ResumeTimer()
        {
            // if ( verbose() )
            //{
            //     std::cout << "Resuming timer" << std::endl;
            // }
            return S_OK;
        }

        bool showError( const QString &msg, HRESULT code, std::function< void() > runFunc )
        {
            auto fullMessage = QString( "%1 - %2" ).arg( msg ).arg( getWindowsError( code ) );
            MessageBox( nullptr, fullMessage.toStdWString().c_str(), L"Error", MB_OK | MB_ICONERROR );
            runFunc();
            return false;
        }

        bool moveToTrashImpl( const QString &fileName, std::shared_ptr< SRecycleOptions > options )
        {
            if ( options->fVerbose )
            {
                if ( !QFileInfo( fileName ).exists() )
                {
                    std::cout << "File or Directory '" << fileName.toStdString() << "' does not exist." << std::endl;
                    return false;
                }
            }

            HRESULT hr = CoInitializeEx( nullptr, COINIT_MULTITHREADED );
            if ( FAILED( hr ) )
            {
                // Couldn't initialize COM library - clean up and return
                return showError( "Couldn't initialize COM library", hr, []() { CoUninitialize(); } );
            }
            // Initialize the file operation
            IFileOperation *fileOperation;
            hr = CoCreateInstance( CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_PPV_ARGS( &fileOperation ) );
            if ( FAILED( hr ) )
            {
                // Couldn't CoCreateInstance - clean up and return
                return showError( "Couldn't CoCreateInstance", hr, []() { CoUninitialize(); } );
            }
            hr = fileOperation->SetOperationFlags( FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NO_UI | FOF_NOERRORUI );
            if ( FAILED( hr ) )
            {
                // Couldn't add flags - clean up and return
                return showError( "Couldn't add flags", hr,
                                  [ fileOperation ]()
                                  {
                                      fileOperation->Release();
                                      CoUninitialize();
                                  } );
            }

            // auto path = QDir::current().absoluteFilePath( fileName ).toStdWString();
            auto path = fileName.toStdWString();

            IShellItem *fileOrFolderItem = nullptr;
            hr = SHCreateItemFromParsingName( path.c_str(), nullptr, IID_PPV_ARGS( &fileOrFolderItem ) );
            if ( FAILED( hr ) )
            {
                return showError( "Couldn't create IShellItem from path", hr,
                                  [ fileOrFolderItem, fileOperation ]()
                                  {
                                      if ( fileOrFolderItem )
                                          fileOrFolderItem->Release();
                                      fileOperation->Release();
                                      CoUninitialize();
                                  } );
            }

            auto pSync = new CFileOpProgSinkApp( options );
            // DWORD dwCookie = 0;
            // hr = fileOperation->Advise( pSync, &dwCookie );
            // if ( FAILED( hr ) )
            //{
            //     return showError( "Failed to mark file/folder item for deletion", hr,
            //         [ fileOrFolderItem, pSync ]()
            //         {
            //             fileOrFolderItem->Release();
            //             if ( pSync )
            //                 pSync->Release();
            //             CoUninitialize();
            //         } );
            // }

            hr = fileOperation->DeleteItem( fileOrFolderItem, pSync );
            fileOrFolderItem->Release();
            if ( FAILED( hr ) )
            {
                return showError( "Failed to mark file/folder item for deletion", hr,
                                  [ fileOrFolderItem, fileOperation, pSync ]()
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
                return showError( "Failed to carry out delete", hr, [ fileOrFolderItem, fileOperation ]() {} );
            }
            return true;
        }
    }
}
