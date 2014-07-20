
#pragma once

#ifndef CAMERACALIBRATOR_H
#define CAMERACALIBRATOR_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

class CameraCalibrator 
{
public:
	// �Զ������������������
    std::vector<std::vector<cv::Point3f>> m_objectPoints;
	// ��Ӧ�����������ͼƬ�ϵ������
    std::vector<std::vector<cv::Point2f>> m_imagePoints;
    // ����ڲ�����
    cv::Mat m_cameraMatrix;
	// �������ϵ��
    cv::Mat m_distCoeffs;
	// ������ɱ궨�ı�־
	int m_flag;
	// �����ӳ����� 
    cv::Mat m_map1,m_map2; 
	// ͼƬ�Ƿ�У�����ж�
	bool m_mustInitUndistort;

  public:
	CameraCalibrator() : m_flag(0), m_mustInitUndistort(true) {};
	// ���ͼƬ���Ƿ��з���Ҫ���ͼƬ
	bool CameraCalibrator::IsGoodImage(const cv::Mat &image, cv::Size &boardSize);
	// �������л�ȡ�ǵ�
	int addChessboardPoints(const std::vector<std::string>& filelist, cv::Size & boardSize);
	// ���ҵ��ĺ��ʵĵ���뵽��Ӧ�ĳ�Ա������
    void addPoints(const std::vector<cv::Point2f>& imageCorners, const std::vector<cv::Point3f>& objectCorners);
	// ����궨����
	double calibrate(cv::Size &imageSize);
	// ��������ڲ�����ͻ�������ͼƬУ��
	cv::Mat CameraCalibrator::remap(const cv::Mat &image);
	// ��������ڲ�����
	bool CalculateCameraInnerParameters();
	// ��������ⲿ����
	void CalculateCameraOuterParameters(CameraCalibrator &cameraCalibrator);

	// ������
	double calculateDeviation(const cv::Mat &IH);
    // get����
    cv::Mat getCameraMatrix() { return m_cameraMatrix; }
    cv::Mat getDistCoeffs()   { return m_distCoeffs; }
};

#endif // CAMERACALIBRATOR_H
