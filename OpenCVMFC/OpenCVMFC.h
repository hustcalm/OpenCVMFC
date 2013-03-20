
// OpenCVMFC.h : main header file for the OpenCVMFC application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// COpenCVMFCApp:
// See OpenCVMFC.cpp for the implementation of this class
//

class COpenCVMFCApp : public CWinAppEx
{
public:
	COpenCVMFCApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern COpenCVMFCApp theApp;
