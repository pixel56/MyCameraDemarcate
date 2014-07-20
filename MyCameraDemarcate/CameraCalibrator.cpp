#include "CameraCalibrator.h"

// 将在图片上找到的棋盘角点添加到对应的成员变量中
int CameraCalibrator::addChessboardPoints(const std::vector<std::string>& filelist, cv::Size &boardSize) 
{
    std::vector<cv::Point2f> imageCorners;
    std::vector<cv::Point3f> objectCorners;

	//世界坐标 (X,Y,Z)= (i,j,0)
	for (int i=0; i<boardSize.height; i++) 
	{
		for (int j=0; j<boardSize.width; j++) 
		{
			objectCorners.push_back(cv::Point3f((float)i, (float)j, 0.0f));
		}
    }

    cv::Mat image;
	// 存放有从多少图片中找到了合适的角点
    int successes = 0;
	int nSize = filelist.size();
    for (int i=0; i<nSize; i++) 
	{
        image = cv::imread(filelist[i],0);

		// 添加成功的棋盘（图片）个数
		if ( NULL == image.data )
		{
			return successes;
		}

        // 下面最好判断一下是否找到角点
        bool found = cv::findChessboardCorners( image, boardSize, imageCorners);

        // 角点的亚像素精度
        cv::cornerSubPix( image, imageCorners,
						  cv::Size(5,5), 
						  cv::Size(-1,-1), 
						  cv::TermCriteria( cv::TermCriteria::MAX_ITER +
											cv::TermCriteria::EPS, 
											30,		// 迭代30次
											0.1));  // 最小精度

          // 找到一个好棋盘，添加到成员变量里面去
		  if (imageCorners.size() == boardSize.area()) 
		  {
			// 把点加入到成员变量里面去存放着
            addPoints(imageCorners, objectCorners);
            successes++;
          }
    }

	return successes;
}

bool CameraCalibrator::IsGoodImage(const cv::Mat &image, cv::Size &boardSize)
{
	if ( !image.data )
	{
		return false;
	}
	std::vector<cv::Point2f> imageCorners;

	// 下面最好判断一下是否找到角点
	bool found = cv::findChessboardCorners( image, boardSize, imageCorners);
	if ( !found )
	{
		return false;
	}

	// 角点的亚像素精度
	cv::cornerSubPix( image, imageCorners, 
					cv::Size(5,5), 
					cv::Size(-1,-1), 
					cv::TermCriteria( cv::TermCriteria::MAX_ITER +
					cv::TermCriteria::EPS, 
					30,		// 迭代30次
					0.1));  // 最小精度

	// 找到一个好棋盘
	if (imageCorners.size() == boardSize.area()) 
	{
		// 把点加入到成员变量里面去存放着
		return true;
	}

	return false;
}

// 将合适的点添加到成员变量中，待会标定好用
void CameraCalibrator::addPoints(const std::vector<cv::Point2f>& imageCorners, const std::vector<cv::Point3f>& objectCorners) 
{
	// 图片中的点
	m_imagePoints.push_back(imageCorners);          
	// 世界坐标的点
	m_objectPoints.push_back(objectCorners);
}

// 开始标定相机
double CameraCalibrator::calibrate(cv::Size &imageSize)
{
	// 这时候，拍摄的照片都是没有校正的
	m_mustInitUndistort= true;

	// 相机的外参数，这里用不着，所以是局部变量
	std::vector<cv::Mat> rvecs, tvecs;

	// 开始标定啦
	return cv::calibrateCamera( m_objectPoints, // 世界坐标的点
								m_imagePoints,  // 图片上对应的点
								imageSize,		// 图片尺寸
								m_cameraMatrix, // 相机的内部参数
								m_distCoeffs,   // 相机的畸变系数
								rvecs, tvecs,	// 相机外参数，图片的偏移与旋转
								m_flag);        // 设置标志
		//					,CV_CALIB_USE_INTRINSIC_GUESS);

}

// 校正图片
cv::Mat CameraCalibrator::remap(const cv::Mat &image) 
{
	cv::Mat undistorted;

	// 要确定相机都被标定过了才能调用该函数
	if (m_mustInitUndistort) 
	{
		cv::initUndistortRectifyMap(
			m_cameraMatrix,    // 内参数
            m_distCoeffs,      // 畸变洗漱
            cv::Mat(),		   // 可选的校正矩阵，这里不需要
			cv::Mat(),		   // 畸变参数矩阵，这里也不需要
			image.size(),	   // 校正后图片大小
            CV_32FC1,		   // 输出矩阵的类型
            m_map1, m_map2);   // x和y方向的映射矩阵

		m_mustInitUndistort= false;
	}

	// 开始校正
    cv::remap(image, undistorted, m_map1, m_map2, cv::INTER_LINEAR);

	return undistorted;
}

bool CameraCalibrator::CalculateCameraInnerParameters()
{
	cv::Mat image;
	std::vector<std::string> filelist;

	// 将图片的路径以及文件名加入到容器里
	for (int i=1; i<=17; i++) 
	{
		std::stringstream str;
		str << "" << i << ".jpg";
		filelist.push_back(str.str());
	}

	// 从棋盘中添加角点
	// 棋盘大小（10*10）表示每行没列都有10个角点
	cv::Size boardSize(10,10);
	// 小于12张图片时，标定不成功
	if ( 12 == addChessboardPoints( filelist, boardSize) )
	{
		return false;
	}

	// 标定图片的时候需要图片的大小
	image = cv::imread(filelist[0]);
	if ( NULL == image.data )
	{
		return false;
	}
	// 开始进行标定
	calibrate(image.size());
	

	std::ofstream outfile("caliberation_result.txt");
	FILE *pFile;
	fopen_s(&pFile,"result.txt","w");

	// 相机内参数写如文件
	for ( int i=0; i<m_cameraMatrix.rows; i++ )
	{
		for ( int j=0; j<m_cameraMatrix.cols; j++)
		{
			outfile << m_cameraMatrix.at<double>(i,j) << '\n';
			fprintf_s(pFile,"%lf\n",m_cameraMatrix.at<double>(i,j));
		}
	}

	// 相机畸变系数
	for ( int j=0; j<m_distCoeffs.cols; j++)
	{
		outfile << m_distCoeffs.at<double>(j) << '\n';
		fprintf_s(pFile,"%lf\n",m_distCoeffs.at<double>(j));
	}
	outfile.close();
	fclose(pFile);
	return true;
}

// 计算相机外部参数
// 本项目中，相机外参数不用标定
void CameraCalibrator::CalculateCameraOuterParameters(CameraCalibrator &cameraCalibrator)
{
	cv::Mat image;
	CameraCalibrator outerParameters;
	cv::Size boardSize(10,10);

	std::stringstream str;
	str << "D:\\img\\mychessboards\\17.bmp";
	std::cout << str.str() << std::endl;
	image = cv::imread(str.str(),0);

	std::vector<cv::Point2f> imageCorners;	// 棋盘角点
	std::vector<cv::Point3f> objectCorners;	// 对应的世界坐标

	// 世界坐标(X,Y,Z)= (i,j,0)
	for (int i=0; i<boardSize.height; i++) 
	{
		for (int j=0; j<boardSize.width; j++) 
		{
			objectCorners.push_back(cv::Point3f((float)i, (float)j, 0.0f));
		}
	}

	// 找到棋盘角点
	bool found = cv::findChessboardCorners(image, boardSize, imageCorners);

	// 获取棋盘角点的亚像素精度
	cv::cornerSubPix(image, imageCorners, 
					 cv::Size(5,5), 
					 cv::Size(-1,-1), 
					 cv::TermCriteria(cv::TermCriteria::MAX_ITER +
						cv::TermCriteria::EPS, 
						30,		// 最大迭代次数
						0.1));     // 最小精度

	// 找到好的角点的棋盘
	if (imageCorners.size() == boardSize.area()) 
	{
		outerParameters.addPoints(imageCorners, objectCorners);
	}
	else
	{
		std::cout << "没有找到相应的角点！请重新拍照！" << std::endl;
		return;
	}

	cv::Mat rvec, tvec;
	// 计算该图片的新的外参数：输出rvec旋转向量，tvec转移向量
	cv::solvePnP(outerParameters.m_objectPoints[0],
				 outerParameters.m_imagePoints[0],
				 cameraCalibrator.m_cameraMatrix,
				 cameraCalibrator.m_distCoeffs,
				 rvec,
				 tvec);

	// 旋转矩阵
	cv::Mat rt;
	// 从一个3x1的向量转变为3x3的矩阵matR
	cv::Rodrigues(rvec,rt);

	//---------------------------------
	for (int i=0; i<3; i++)
	{
		rt.at<double>(i,2) = tvec.at<double>(i);
	}
	//---------------------------------
	
	double s=0.0;	// 世界坐标上面的一格有多少个像素
	
	cv::Point2f p1 = outerParameters.m_imagePoints[0][0];
	cv::Point2f p2 = outerParameters.m_imagePoints[0][9];

	// 求得比例尺，棋盘上一个小正方形变长对应的像素点的个数
	// 多求几条线，然后取平均值就好啦
	s = sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y))/9.0;

	// H = M*[r1 r2 t]
	cv::Mat H = cameraCalibrator.m_cameraMatrix * rt;
//	H = H * s;
	cv::Mat IH;
	invert(H,IH);

	// 计算误差
//	cout << outerParameters.calculateDeviation(IH) << endl;
}

// 误差计算
double CameraCalibrator::calculateDeviation(const cv::Mat &IH)
{
	// 测试图片中第十个点到第一个点的距离
	cv::Point2f pointImg = m_imagePoints[0][9] - m_imagePoints[0][0];

	double leng = sqrt(pointImg.x*pointImg.x+pointImg.y*pointImg.y);

// 	cout << "图像上两点坐标长度：" << pointImg << endl;
// 	cout << "世界坐标上两点长度：" << (Mat)pointImg << endl;
// 	cout << "两个像素之间间隔多少毫米：" << (double)(leng/(9*12.0)) << endl;

	// test
	cv::Mat imgPoint1(3,1,cv::DataType<double>::type);
	cv::Mat imgPoint2(3,1,cv::DataType<double>::type);

	// 取横向的10条最长直线，算误差
	double averageDeviation=0.0;
	for (int i=0; i< 10; i++)
	{
		imgPoint1.at<double>(0) = m_imagePoints[0][10*i+9].x;
		imgPoint1.at<double>(1) = m_imagePoints[0][10*i+9].y;
		imgPoint1.at<double>(2) = 1.0;

		imgPoint2.at<double>(0) = m_imagePoints[0][10*i].x;
		imgPoint2.at<double>(1) = m_imagePoints[0][10*i].y;
		imgPoint2.at<double>(2) = 1.0;

		cv::Mat xyz1 = IH*imgPoint1;
		cv::Mat xyz2 = IH*imgPoint2;

		xyz1 = xyz1/xyz1.at<double>(2);
		xyz2 = xyz2/xyz2.at<double>(2);

		cv::Mat xyz = xyz2 - xyz1;
		double tmpDeviation = (double)sqrt(xyz.at<double>(0)*xyz.at<double>(0)+xyz.at<double>(1)*xyz.at<double>(1));
//		cout << tmpDeviation << endl;
		averageDeviation = averageDeviation + tmpDeviation;
	}

	// 取纵向的10条最长直线，算误差
	for (int i=0; i< 10; i++)
	{
		imgPoint1.at<double>(0) = m_imagePoints[0][90+i].x;
		imgPoint1.at<double>(1) = m_imagePoints[0][90+i].y;
		imgPoint1.at<double>(2) = 1.0;

		imgPoint2.at<double>(0) = m_imagePoints[0][i].x;
		imgPoint2.at<double>(1) = m_imagePoints[0][i].y;
		imgPoint2.at<double>(2) = 1.0;

		cv::Mat xyz1 = IH*imgPoint1;
		cv::Mat xyz2 = IH*imgPoint2;

		xyz1 = xyz1/xyz1.at<double>(2);
		xyz2 = xyz2/xyz2.at<double>(2);

		cv::Mat xyz = xyz2 - xyz1;
		double tmpDeviation = (double)sqrt(xyz.at<double>(0)*xyz.at<double>(0)+xyz.at<double>(1)*xyz.at<double>(1));
//		cout << tmpDeviation << endl;
		averageDeviation = averageDeviation + tmpDeviation;
	}

	// 单独测试一个距离
	imgPoint1.at<double>(0) = m_imagePoints[0][1].x;
	imgPoint1.at<double>(1) = m_imagePoints[0][1].y;
	imgPoint1.at<double>(2) = 1.0;

	imgPoint2.at<double>(0) = m_imagePoints[0][0].x;
	imgPoint2.at<double>(1) = m_imagePoints[0][0].y;
	imgPoint2.at<double>(2) = 1.0;

	cv::Mat xyz1 = IH*imgPoint1;
	cv::Mat xyz2 = IH*imgPoint2;

	xyz1 = xyz1/xyz1.at<double>(2);
	xyz2 = xyz2/xyz2.at<double>(2);

	cv::Mat xyz = xyz2 - xyz1;
	double tmpDeviation = (double)sqrt(xyz.at<double>(0)*xyz.at<double>(0)+xyz.at<double>(1)*xyz.at<double>(1));
//	cout << tmpDeviation << endl;
	// 单独测试一个距离 结束

	return averageDeviation/20.0;
}
