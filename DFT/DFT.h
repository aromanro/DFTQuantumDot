
// DFT.h : main header file for the DFT application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "Options.h"


// CDFTApp:
// See DFT.cpp for the implementation of this class
//

class CDFTApp : public CWinAppEx
{
public:
	CDFTApp();

	Options options;
// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CDFTApp theApp;

