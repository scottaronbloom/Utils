#include "FileSIDInfo.h"
#include "utils.h"
#include <QFileInfo>

#include <tuple>
#include <cstdio>
#include <windows.h>
#include <tchar.h>
#include <accctrl.h>
#include <aclapi.h>
#include <sddl.h>
#pragma comment(lib, "advapi32.lib")

namespace NSABUtils
{
    namespace NFileUtils
    {
        std::wstring getWinPath( const std::wstring & pathName )
        {
            auto winPath = pathName;
            std::replace( winPath.begin(), winPath.end(), '/', '\\' );
            winPath = LR"(\\?\)" + winPath;
            return winPath;
        }

        std::wstring sidToString( PSID sid )
        {
            // Convert the data to a string.
            wchar_t * sid_string;
            if ( ::ConvertSidToStringSid( sid, &sid_string ) )
            {
                std::wstring sidString = sid_string;
                ::LocalFree( sid_string );
                return sidString;
            }
            return {};
        }

        // returns SID, domain, account
        std::tuple< std::wstring, std::wstring, std::wstring > getSIDAccountInfo( PSID sid, std::pair< bool, std::wstring > * statusMsg /*= nullptr*/ )
        {
            if ( sid == nullptr )
            {
                if ( statusMsg )
                {
                    *statusMsg = std::make_pair( false, L"null sid pointer" );
                }
                return {};
            }

            DWORD acctLen = 0;
            std::vector< wchar_t > account;
            DWORD domainLen = 0;
            std::vector< wchar_t > domain;
            SID_NAME_USE use = SidTypeUnknown;

            *statusMsg = std::make_pair( true, std::wstring() );

            // First call to LookupAccountSid to get the buffer sizes.
            do
            {
                if ( !LookupAccountSid( nullptr, sid,
                     account.data(), &acctLen,
                     domain.data(), &domainLen,
                     &use ) )
                {
                    if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
                    {
                        account.resize( acctLen + 1 );
                        domain.resize( domainLen + 1 );
                    }
                    else if ( GetLastError() == ERROR_NONE_MAPPED )
                    {
                        if ( statusMsg )
                        {
                            *statusMsg = { false, QString( "Could not find SID info" ).toStdWString() };
                        }
                        break;
                    }
                    else
                    {
                        if ( statusMsg )
                        {
                            auto msg = QString( "Error getting SID information: %1" ).arg( getLastError() ).toStdWString();
                            *statusMsg = std::make_pair( false, msg );
                        }
                        break;
                    }
                }
                else
                    break;
            } while ( true );

            while ( !account.empty() && account.back() == 0 )
                account.pop_back();
            while ( !domain.empty() && domain.back() == 0 )
                domain.pop_back();

            auto domainStr = std::wstring( domain.begin(), domain.end() );
            auto acctStr = std::wstring( account.begin(), account.end() );
            auto sidString = sidToString( sid );
            return { sidString, domainStr, acctStr };
        };

        SSIDInfo::SSIDInfo( PSID sid )
        {
            std::tie( fSID, fDomain, fOwner ) = getSIDAccountInfo( sid, &fErrorMsg );
            fErrorMsg.first &= ( fSID != L"S-1-0-0" );
        }
 
        std::wstring SSIDInfo::dispName() const
        {
            auto retVal = fDomain;
            if ( !retVal.empty() )
                retVal += L"\\";
            retVal += fOwner;
            if ( retVal.empty() )
                retVal = fSID;
            return retVal;
        }

        CSIDFileInfo::CSIDFileInfo( const std::wstring & pathName, bool onlyInvalidDACL /*= false*/ ) :
            fFileName( getWinPath( pathName ) ),
            fOnlyInvalidDACL( onlyInvalidDACL )
        {
            PSID owner = 0;
            PSID group = 0;
            PACL dacl = 0;

            PSECURITY_DESCRIPTOR pSD = nullptr;
            auto status = GetNamedSecurityInfo( fFileName.c_str(),
                                                SE_FILE_OBJECT,
                                                OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                                                &owner,
                                                &group,
                                                &dacl, 0, &pSD );
            if ( status != ERROR_SUCCESS )
            {
                fMsg.emplace_back( QString( "Error getting Security Info: %1" ).arg( getLastError( status ) ).toStdWString() );
                return;
            }
            LocalFree( pSD );

            fOwner = SSIDInfo( owner );
            if ( !fOwner.fErrorMsg.second.empty() )
                fMsg.push_back( fOwner.fErrorMsg.second );

            fGroup = SSIDInfo( group );
            if ( !fGroup.fErrorMsg.second.empty() )
                fMsg.push_back( fGroup.fErrorMsg.second );

            ACL_SIZE_INFORMATION aclSizeInfo = { sizeof( ACL ) };
            if ( !GetAclInformation( dacl, &aclSizeInfo, sizeof( ACL_SIZE_INFORMATION ), ACL_INFORMATION_CLASS::AclSizeInformation ) )
            {
                fMsg.emplace_back( QString( "Error getting Security Info: %1" ).arg( getLastError() ).toStdWString() );
                return;
            }
            for ( DWORD ii = 0; ii < aclSizeInfo.AceCount; ++ii )
            {
                LPVOID curr = nullptr;
                if ( !::GetAce( dacl, ii, &curr ) )
                {
                    fMsg.emplace_back( QString( "Error getting Security Info: %1" ).arg( getLastError() ).toStdWString() );
                    continue;
                }
                PSID pSid = &( (ACCESS_ALLOWED_ACE *)curr )->SidStart;

                auto currSID = SSIDInfo( pSid );
                if ( !fOnlyInvalidDACL || !currSID.fErrorMsg.first )
                {
                    fDACLs.insert( currSID );
                    if ( !currSID.fErrorMsg.second.empty() )
                        fMsg.push_back( currSID.fErrorMsg.second );
                }
            }
            fAOK = true;
        }

        CSIDFileInfo::CSIDFileInfo( const std::string & pathName, bool onlyInvalidDACL ) :
            CSIDFileInfo( std::move( std::wstring( pathName.begin(), pathName.end() ) ), onlyInvalidDACL )
        {
        }

        CSIDFileInfo::CSIDFileInfo( const QString & pathName, bool onlyInvalidDACL ) :
            CSIDFileInfo( pathName.toStdWString(), onlyInvalidDACL )
        {
        }

        CSIDFileInfo::CSIDFileInfo( const QFileInfo & fi, bool onlyInvalidDACL ) :
            CSIDFileInfo( fi.absoluteFilePath(), onlyInvalidDACL )
        {
        }

        std::wstring CSIDFileInfo::daclsString() const
        {
            std::wstring retVal;
            for ( auto && ii : fDACLs )
            {
                auto curr = ii.dispName();
                if ( curr.empty() )
                    continue;
                if ( !retVal.empty() )
                    retVal += L", ";
                retVal += curr;
            }
            return retVal;
        }

        bool CSIDFileInfo::hasInvalidSID() const
        {
            if ( !fOwner.fErrorMsg.first || !fGroup.fErrorMsg.first )
                return true;

            for ( auto && ii : fDACLs )
            {
                if ( !ii.fErrorMsg.first )
                    return true;
            }
            return false;
        }

    }
}