
// MyCameraDemarcateDlg.h : header file
//

#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "CvvImage.h"
#include <string>
#include "CameraCalibrator.h"

// CMyCameraDemarcateDlg dialog
class CMyCameraDemarcateDlg : public CDialogEx
{
// Construction
public:
	CMyCameraDemarcateDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MYCAMERADEMARCATE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	cv::VideoCapture m_capture;
	CameraCalibrator m_theCameraCalibrator;
	cv::Mat m_frame; // current video/camera frame
	void InitCamera();
	void DrawPicToHDC(IplImage *img, UINT ID);
	afx_msg void OnBnClickedTakePhotos();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCameracalobrator();
	afx_msg void OnBnClickedOpencamera();
};
