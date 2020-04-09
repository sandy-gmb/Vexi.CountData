// FileVersion.cpp: implementation of the CFileVersion class.  
// by Manuel Laflamme   
//////////////////////////////////////////////////////////////////////  
#include "fileVersion.h"  
#include <Windows.h>
#include <WinVer.h>

#include <functional>

#pragma comment(lib, "version")  

//////////////////////////////////////////////////////////////////////  
CFileVersion::CFileVersion() 
	: m_bInit(false)
{   
}  
CFileVersion::~CFileVersion()   
{   
}   

bool CFileVersion::Init(QString filep)  
{  
	m_bInit = false;

	LPBYTE  m_lpVersionData = NULL;   
	DWORD   m_dwLangCharset = 0;   

	do 
	{
		// Get the version information size for allocate the buffer  
		DWORD dwHandle;       
		DWORD dwDataSize = GetFileVersionInfoSizeA((LPCSTR)filep.toStdString().c_str(), &dwHandle);   
		if ( dwDataSize == 0 )   
			return false;  

		// Allocate buffer and retrieve version information  
		m_lpVersionData = new BYTE[dwDataSize];   
		if (!GetFileVersionInfoA((LPCSTR)filep.toStdString().c_str(), dwHandle, dwDataSize,   
			(void**)m_lpVersionData) )  
		{  
			break;  
		}  

		// Retrieve the first language and character-set identifier  
		UINT nQuerySize;  
		DWORD* pTransTable;  
		if (!VerQueryValueA(m_lpVersionData, "\\VarFileInfo\\Translation",  
			(void **)&pTransTable, &nQuerySize) )  
		{  
			break;  
		}  
		// Swap the words to have lang-charset in the correct format  
		m_dwLangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));  

		auto QueryValue = [&](QString lpszValueName) ->QString
		{
			UINT nQuerySize;  
			LPVOID lpData;  
			QString strValue, strBlockName;  
			strBlockName = QString("\\StringFileInfo\\%1\\%2").arg((int)m_dwLangCharset, 8, 16, QChar('0')).arg( lpszValueName);  
			if ( ::VerQueryValueA((void **)m_lpVersionData, strBlockName.toStdString().c_str(), &lpData, &nQuerySize) == TRUE )  
			{
					strValue = QString::fromLocal8Bit((LPCSTR)lpData);  
			}
			
			return strValue;
		};


		m_strCompanyName = QueryValue("CompanyName");
		m_strProdName = QueryValue("ProductName");

		//get FileVersionInfo Struct to parse all information
		VS_FIXEDFILEINFO vsffi;
		VS_FIXEDFILEINFO* pVsffi;  
		if ( ::VerQueryValueA((void **)m_lpVersionData, ("\\"),  
			(void**)&pVsffi, &nQuerySize) )  
		{  
			vsffi = *pVsffi; 

			m_lFileVer.append(HIWORD(vsffi.dwFileVersionMS));
			m_lFileVer.append(LOWORD(vsffi.dwFileVersionMS));
			m_lFileVer.append(HIWORD(vsffi.dwFileVersionLS));
			m_lFileVer.append(LOWORD(vsffi.dwFileVersionLS));

			m_lProdVer.append(HIWORD(vsffi.dwProductVersionMS));
			m_lProdVer.append(LOWORD(vsffi.dwProductVersionMS));
			m_lProdVer.append(HIWORD(vsffi.dwProductVersionLS));
			m_lProdVer.append(LOWORD(vsffi.dwProductVersionLS));

			m_bInit = true;
		}  

		

	} while (false);

	if(m_lpVersionData != NULL)
	{
		delete []m_lpVersionData;
		m_lpVersionData = NULL;
	}
	
	return m_bInit;  


}  

QString CFileVersion::GetCompanyName()
{
	return m_strCompanyName;
}

QString CFileVersion::GetProductName()
{
	return m_strProdName;
}

QString CFileVersion::GetFixedFileVersion()  
{  
	QString  strVersion = QString("%1.%2.%3.%4")
		.arg(m_lFileVer[0])
		.arg(m_lFileVer[1])
		.arg(m_lFileVer[2])
		.arg(m_lFileVer[3]);  
    return strVersion;  
}  
QString CFileVersion::GetFixedProductVersion()  
{  
	QString  strVersion = QString("%1.%2.%3.%4")
		.arg(m_lProdVer[0])
		.arg(m_lProdVer[1])
		.arg(m_lProdVer[2])
		.arg(m_lProdVer[3]);  
	return strVersion;  
}  