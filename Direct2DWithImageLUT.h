
// Direct2DWithImageLUT.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CDirect2DWithImageLUTApp:
// See Direct2DWithImageLUT.cpp for the implementation of this class
//

class CDirect2DWithImageLUTApp : public CWinApp
{
public:
	CDirect2DWithImageLUTApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDirect2DWithImageLUTApp theApp;
