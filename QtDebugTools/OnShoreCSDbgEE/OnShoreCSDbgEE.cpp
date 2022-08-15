#include <windows.h>
#include <QDate>

#define ADDIN_API    __declspec(dllexport)

typedef struct tagDEBUGHELPER
{
    DWORD dwVersion;
    BOOL (WINAPI *ReadDebuggeeMemory)( struct tagDEBUGHELPER *pThis, DWORD dwAddr, DWORD nWant, VOID* pWhere, DWORD *nGot );
    // from here only when dwVersion >= 0x20000
    DWORDLONG (WINAPI *GetRealAddress)( struct tagDEBUGHELPER *pThis );
    BOOL (WINAPI *ReadDebuggeeMemoryEx)( struct tagDEBUGHELPER *pThis, DWORDLONG qwAddr, DWORD nWant, VOID* pWhere, DWORD *nGot );
    int (WINAPI *GetProcessorType)( struct tagDEBUGHELPER *pThis );
} DEBUGHELPER;

extern "C" ADDIN_API  HRESULT WINAPI OnShoreCSDbgEE_QDate( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t max, DWORD reserved )
{
	QDate FAR date;
	DWORD nGot=0;
	pHelper->ReadDebuggeeMemory(pHelper,dwAddress,sizeof(date),&date,&nGot);

	char tmp[ 255 ];
	strcpy( tmp, date.toString().toAscii() );
	wsprintf(pResult,"QDate: '%s'", tmp);
	return S_OK;
}

extern "C" ADDIN_API  HRESULT WINAPI OnShoreCSDbgEE_QTime( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t max, DWORD reserved )
{
	QTime FAR time;
	DWORD nGot=0;
	pHelper->ReadDebuggeeMemory(pHelper,dwAddress,sizeof(time),&time,&nGot);

	char tmp[ 255 ];
	strcpy( tmp, time.toString().toAscii() );
	wsprintf(pResult,"QTime: '%s'", tmp);
	return S_OK;
}

extern "C" ADDIN_API  HRESULT WINAPI OnShoreCSDbgEE_QDateTime( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t max, DWORD reserved )
{
	QDateTime FAR dt;
	DWORD nGot=0;
	pHelper->ReadDebuggeeMemory(pHelper,dwAddress,sizeof(dt),&dt,&nGot);

	char tmp[ 255 ];
	strcpy( tmp, dt.toString().toAscii() );
	wsprintf(pResult,"QDateTime: '%s'", tmp);
	return S_OK;
}

