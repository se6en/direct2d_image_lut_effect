
// Direct2DWithImageLUTDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Direct2DWithImageLUT.h"
#include "Direct2DWithImageLUTDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDirect2DWithImageLUTDlg dialog



CDirect2DWithImageLUTDlg::CDirect2DWithImageLUTDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIRECT2DWITHIMAGELUT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDirect2DWithImageLUTDlg::DoDataExchange(CDataExchange* pDX)
{
   DDX_Control(pDX, IDC_STATIC_CTRL, m_ctrlDirect2DImageCtrl);
   DDX_Control(pDX, IDC_BTN_GOLDEN_EFFECT, m_btnGolden);
   DDX_Control(pDX, IDC_BTN_NOIR_EFFECT, m_btnNoir);
   DDX_Control(pDX, IDC_BTN_GOLDEN_NOIR_EFFECT, m_btnGoldenNoir);

	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDirect2DWithImageLUTDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
   ON_WM_SIZE()
   ON_BN_CLICKED(IDC_BTN_GOLDEN_EFFECT, OnBnClickedGoldenEffect)
   ON_BN_CLICKED(IDC_BTN_NOIR_EFFECT, OnBnClickedNoirEffect)
   ON_BN_CLICKED(IDC_BTN_GOLDEN_NOIR_EFFECT, OnBnClickedGoldenNoirEffect)
END_MESSAGE_MAP()


// CDirect2DWithImageLUTDlg message handlers

BOOL CDirect2DWithImageLUTDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
   MoveWindow(CRect(0, 0, 840, 590));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDirect2DWithImageLUTDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDirect2DWithImageLUTDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDirect2DWithImageLUTDlg::OnSize(UINT nType, int cx, int cy)
{
   if (m_ctrlDirect2DImageCtrl.GetSafeHwnd() == nullptr)
   {
      return;
   }

   CRect rcClient;
   GetClientRect(rcClient);

   CRect rcImageCtrl;
   rcImageCtrl.top = rcClient.top;
   rcImageCtrl.left = rcClient.left + (rcClient.Width() - 800) / 2;
   rcImageCtrl.bottom = rcImageCtrl.top + 450;
   rcImageCtrl.right = rcImageCtrl.left + 800;
   m_ctrlDirect2DImageCtrl.MoveWindow(rcImageCtrl);

   CRect rcBtn;
   rcBtn.top = rcImageCtrl.bottom + 20;
   rcBtn.bottom = rcBtn.top + 20;
   rcBtn.left = rcClient.left + 130;
   rcBtn.right = rcBtn.left + 80;
   m_btnGolden.MoveWindow(rcBtn);

   rcBtn.left = rcBtn.right + 130;
   rcBtn.right = rcBtn.left + 80;
   m_btnNoir.MoveWindow(rcBtn);

   rcBtn.left = rcBtn.right + 130;
   rcBtn.right = rcBtn.left + 160;
   m_btnGoldenNoir.MoveWindow(rcBtn);
}

void CDirect2DWithImageLUTDlg::OnBnClickedGoldenEffect()
{
   m_ctrlDirect2DImageCtrl.SetEffect(TRUE, FALSE);
}

void CDirect2DWithImageLUTDlg::OnBnClickedNoirEffect()
{
   m_ctrlDirect2DImageCtrl.SetEffect(FALSE, TRUE);
}

void CDirect2DWithImageLUTDlg::OnBnClickedGoldenNoirEffect()
{
   m_ctrlDirect2DImageCtrl.SetEffect(TRUE, TRUE);
}
