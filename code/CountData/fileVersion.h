// FileVersion.h: interface for the CFileVersion class.  
// by Manuel Laflamme  
//////////////////////////////////////////////////////////////////////  
#ifndef __FILEVERSION_H_  
#define __FILEVERSION_H_  

#if _MSC_VER >= 1000  
#pragma once  
#endif // _MSC_VER >= 1000  

#include <Windows.h>

#include <QString>
#include <QList>

class CFileVersion  
{   
// Construction  
public:   
	CFileVersion();
	~CFileVersion();     
// Operations     
public:   
    bool    Init(QString filep);  
    QString GetCompanyName(); 
    QString GetProductName();
    QString GetFixedFileVersion();  
    QString GetFixedProductVersion();  
// Attributes  
protected: 
	QString m_strFilePath;
	bool m_bInit;
	QList<int> m_lFileVer;
	QList<int> m_lProdVer;

	QString m_strProdName;
	QString m_strCompanyName;

// Implementation  
public:  
};   
#endif  // __FILEVERSION_H_ 