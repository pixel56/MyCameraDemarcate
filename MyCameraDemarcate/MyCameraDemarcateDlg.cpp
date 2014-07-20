
// MyCameraDemarcateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MyCameraDemarcate.h"
#include "MyCameraDemarcateDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMyCameraDemarcateDlg dialog




CMyCameraDemarcateDlg::CMyCameraDemarcateDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMyCameraDemarcateDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CMyCameraDemarcateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMyCameraDemarcateDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_TAKE_PHOTOS, &CMyCameraDemarcateDlg::OnBnClickedTakePhotos)
	ON_WM_TIMER()
	ON_BN_CLICKED(BNT_CAMERACALOBRATOR, &CMyCameraDemarcateDlg::OnBnClickedCameracalobrator)
	ON_BN_CLICKED(BTN_OPENCAMERA, &CMyCameraDemarcateDlg::OnBnClickedOpencamera)
END_MESSAGE_MAP()


// CMyCameraDemarcateDlg message handlers

BOOL CMyCameraDemarcateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMyCameraDemarcateDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMyCameraDemarcateDlg::OnPaint()
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
HCURSOR CMyCameraDemarcateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMyCameraDemarcateDlg::InitCamera()
{
	// �������ͷ�Ƿ��
	if (!m_capture.isOpened())
		return;

	// ��ͣ�Ķ�ȡͼƬ
	while (true) 
	{
		// ��ȡ��һ֡ͼƬ
		if (!m_capture.read(m_frame))
			break;

		DrawPicToHDC(&(IplImage)m_frame, IDC_STATIC_PIC);
		
	}

	m_capture.release();
}

// ��ͼ���Ի�����ȥ
void CMyCameraDemarcateDlg::DrawPicToHDC(IplImage *img, UINT ID)
{
	CDC *pDC = GetDlgItem(ID)->GetDC();
	HDC hDC=  pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	CvvImage cimg;

	cimg.CopyOf(img);
	cimg.DrawToHDC(hDC,&rect);
	ReleaseDC(pDC);
}

void CMyCameraDemarcateDlg::OnBnClickedTakePhotos()
{
	if ( !m_capture.isOpened() )
	{
		MessageBox("��������ͷ�Ƿ�װ��ȷ��");
		return;
	}

	// ������Ƭ����ǰ����·��
	static int i=1;
	char buffer[10];
	sprintf_s(buffer,"%d",i++);
	std::string str;
	str.append(buffer);
	str.append(".jpg");

	cv::Size boardSize(10,10);

	// ���һ���Ƿ��ܹ��ҵ����ʵĽǵ�
	if ( true == m_theCameraCalibrator.IsGoodImage(m_frame, boardSize ))
	{
		cv::imwrite(str,m_frame);
		return;
	}
	
	MessageBox("�����ڸ�ͼƬ���ҵ����ʵ�ͼƬ�����������㣡");
}


void CMyCameraDemarcateDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case 1:
		{
			if (!m_capture.isOpened())
				break;

			// read next frame if any
			if (!m_capture.read(m_frame))
				break;

			DrawPicToHDC(&(IplImage)m_frame, IDC_STATIC_PIC);
		}
		break;
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CMyCameraDemarcateDlg::OnBnClickedCameracalobrator()
{
	// ��������궨����
	if ( false == m_theCameraCalibrator.CalculateCameraInnerParameters() )
	{
		MessageBox("û���ҵ��㹻�ĺ��ʵ���Ƭ��");
	}
}


void CMyCameraDemarcateDlg::OnBnClickedOpencamera()
{
	static bool isOpen = false;
	if ( !isOpen )
	{
		m_capture.open(0);
		isOpen = true;

	}
	
	static bool flag = true;

	if ( flag )
	{
		if ( !m_capture.isOpened() )
		{
			MessageBox("��������ͷ�Ƿ�װ�ã�");
			return;
		}

		GetDlgItem(BTN_OPENCAMERA)->SetWindowText("�ر�����ͷ");
		flag = false;
		SetTimer(1,50,0);
		return;
	}

	GetDlgItem(BTN_OPENCAMERA)->SetWindowText("������ͷ");
	flag = true;

	KillTimer(1);
}
