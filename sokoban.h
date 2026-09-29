
// sokoban.h : main header file for the sokoban application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CsokobanApp:
// See sokoban.cpp for the implementation of this class
//

class CsokobanApp : public CWinApp
{
public:
	CsokobanApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CsokobanApp theApp;
