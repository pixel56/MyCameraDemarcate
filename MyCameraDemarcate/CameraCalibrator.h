
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
	// 自定义的世界坐标点的容器
    std::vector<std::vector<cv::Point3f>> m_objectPoints;
	// 对应于世界坐标的图片上的坐标点
    std::vector<std::vector<cv::Point2f>> m_imagePoints;
    // 相机内部参数
    cv::Mat m_cameraMatrix;
	// 相机畸变系数
    cv::Mat m_distCoeffs;
	// 怎样完成标定的标志
	int m_flag;
	// 畸变的映射矩阵 
    cv::Mat m_map1,m_map2; 
	// 图片是否被校正的判断
	bool m_mustInitUndistort;

  public:
	CameraCalibrator() : m_flag(0), m_mustInitUndistort(true) {};
	// 检查图片上是否有符合要求的图片
	bool CameraCalibrator::IsGoodImage(const cv::Mat &image, cv::Size &boardSize);
	// 从棋盘中获取角点
	int addChessboardPoints(const std::vector<std::string>& filelist, cv::Size & boardSize);
	// 将找到的合适的点放入到对应的成员变量中
    void addPoints(const std::vector<cv::Point2f>& imageCorners, const std::vector<cv::Point3f>& objectCorners);
	// 相机标定函数
	double calibrate(cv::Size &imageSize);
	// 根据相机内部矩阵和畸变矩阵对图片校正
	cv::Mat CameraCalibrator::remap(const cv::Mat &image);
	// 计算相机内部参数
	bool CalculateCameraInnerParameters();
	// 计算相机外部参数
	void CalculateCameraOuterParameters(CameraCalibrator &cameraCalibrator);

	// 误差计算
	double calculateDeviation(const cv::Mat &IH);
    // get方法
    cv::Mat getCameraMatrix() { return m_cameraMatrix; }
    cv::Mat getDistCoeffs()   { return m_distCoeffs; }
};

#endif // CAMERACALIBRATOR_H
