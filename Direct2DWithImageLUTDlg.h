
// Direct2DWithImageLUTDlg.h : header file
//

#pragma once

#include "Direct2DImageCtrl.h"

// CDirect2DWithImageLUTDlg dialog
class CDirect2DWithImageLUTDlg : public CDialogEx
{
// Construction
public:
	CDirect2DWithImageLUTDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIRECT2DWITHIMAGELUT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
   afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

private:
   CDirect2DImageCtrl                m_ctrlDirect2DImageCtrl;
};
