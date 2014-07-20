#include "CameraCalibrator.h"

// ����ͼƬ���ҵ������̽ǵ���ӵ���Ӧ�ĳ�Ա������
int CameraCalibrator::addChessboardPoints(const std::vector<std::string>& filelist, cv::Size &boardSize) 
{
    std::vector<cv::Point2f> imageCorners;
    std::vector<cv::Point3f> objectCorners;

	//�������� (X,Y,Z)= (i,j,0)
	for (int i=0; i<boardSize.height; i++) 
	{
		for (int j=0; j<boardSize.width; j++) 
		{
			objectCorners.push_back(cv::Point3f((float)i, (float)j, 0.0f));
		}
    }

    cv::Mat image;
	// ����дӶ���ͼƬ���ҵ��˺��ʵĽǵ�
    int successes = 0;
	int nSize = filelist.size();
    for (int i=0; i<nSize; i++) 
	{
        image = cv::imread(filelist[i],0);

		// ��ӳɹ������̣�ͼƬ������
		if ( NULL == image.data )
		{
			return successes;
		}

        // ��������ж�һ���Ƿ��ҵ��ǵ�
        bool found = cv::findChessboardCorners( image, boardSize, imageCorners);

        // �ǵ�������ؾ���
        cv::cornerSubPix( image, imageCorners,
						  cv::Size(5,5), 
						  cv::Size(-1,-1), 
						  cv::TermCriteria( cv::TermCriteria::MAX_ITER +
											cv::TermCriteria::EPS, 
											30,		// ����30��
											0.1));  // ��С����

          // �ҵ�һ�������̣���ӵ���Ա��������ȥ
		  if (imageCorners.size() == boardSize.area()) 
		  {
			// �ѵ���뵽��Ա��������ȥ�����
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

	// ��������ж�һ���Ƿ��ҵ��ǵ�
	bool found = cv::findChessboardCorners( image, boardSize, imageCorners);
	if ( !found )
	{
		return false;
	}

	// �ǵ�������ؾ���
	cv::cornerSubPix( image, imageCorners, 
					cv::Size(5,5), 
					cv::Size(-1,-1), 
					cv::TermCriteria( cv::TermCriteria::MAX_ITER +
					cv::TermCriteria::EPS, 
					30,		// ����30��
					0.1));  // ��С����

	// �ҵ�һ��������
	if (imageCorners.size() == boardSize.area()) 
	{
		// �ѵ���뵽��Ա��������ȥ�����
		return true;
	}

	return false;
}

// �����ʵĵ���ӵ���Ա�����У�����궨����
void CameraCalibrator::addPoints(const std::vector<cv::Point2f>& imageCorners, const std::vector<cv::Point3f>& objectCorners) 
{
	// ͼƬ�еĵ�
	m_imagePoints.push_back(imageCorners);          
	// ��������ĵ�
	m_objectPoints.push_back(objectCorners);
}

// ��ʼ�궨���
double CameraCalibrator::calibrate(cv::Size &imageSize)
{
	// ��ʱ���������Ƭ����û��У����
	m_mustInitUndistort= true;

	// �����������������ò��ţ������Ǿֲ�����
	std::vector<cv::Mat> rvecs, tvecs;

	// ��ʼ�궨��
	return cv::calibrateCamera( m_objectPoints, // ��������ĵ�
								m_imagePoints,  // ͼƬ�϶�Ӧ�ĵ�
								imageSize,		// ͼƬ�ߴ�
								m_cameraMatrix, // ������ڲ�����
								m_distCoeffs,   // ����Ļ���ϵ��
								rvecs, tvecs,	// ����������ͼƬ��ƫ������ת
								m_flag);        // ���ñ�־
		//					,CV_CALIB_USE_INTRINSIC_GUESS);

}

// У��ͼƬ
cv::Mat CameraCalibrator::remap(const cv::Mat &image) 
{
	cv::Mat undistorted;

	// Ҫȷ����������궨���˲��ܵ��øú���
	if (m_mustInitUndistort) 
	{
		cv::initUndistortRectifyMap(
			m_cameraMatrix,    // �ڲ���
            m_distCoeffs,      // ����ϴ��
            cv::Mat(),		   // ��ѡ��У���������ﲻ��Ҫ
			cv::Mat(),		   // ���������������Ҳ����Ҫ
			image.size(),	   // У����ͼƬ��С
            CV_32FC1,		   // ������������
            m_map1, m_map2);   // x��y�����ӳ�����

		m_mustInitUndistort= false;
	}

	// ��ʼУ��
    cv::remap(image, undistorted, m_map1, m_map2, cv::INTER_LINEAR);

	return undistorted;
}

bool CameraCalibrator::CalculateCameraInnerParameters()
{
	cv::Mat image;
	std::vector<std::string> filelist;

	// ��ͼƬ��·���Լ��ļ������뵽������
	for (int i=1; i<=17; i++) 
	{
		std::stringstream str;
		str << "" << i << ".jpg";
		filelist.push_back(str.str());
	}

	// ����������ӽǵ�
	// ���̴�С��10*10����ʾÿ��û�ж���10���ǵ�
	cv::Size boardSize(10,10);
	// С��12��ͼƬʱ���궨���ɹ�
	if ( 12 == addChessboardPoints( filelist, boardSize) )
	{
		return false;
	}

	// �궨ͼƬ��ʱ����ҪͼƬ�Ĵ�С
	image = cv::imread(filelist[0]);
	if ( NULL == image.data )
	{
		return false;
	}
	// ��ʼ���б궨
	calibrate(image.size());
	

	std::ofstream outfile("caliberation_result.txt");
	FILE *pFile;
	fopen_s(&pFile,"result.txt","w");

	// ����ڲ���д���ļ�
	for ( int i=0; i<m_cameraMatrix.rows; i++ )
	{
		for ( int j=0; j<m_cameraMatrix.cols; j++)
		{
			outfile << m_cameraMatrix.at<double>(i,j) << '\n';
			fprintf_s(pFile,"%lf\n",m_cameraMatrix.at<double>(i,j));
		}
	}

	// �������ϵ��
	for ( int j=0; j<m_distCoeffs.cols; j++)
	{
		outfile << m_distCoeffs.at<double>(j) << '\n';
		fprintf_s(pFile,"%lf\n",m_distCoeffs.at<double>(j));
	}
	outfile.close();
	fclose(pFile);
	return true;
}

// ��������ⲿ����
// ����Ŀ�У������������ñ궨
void CameraCalibrator::CalculateCameraOuterParameters(CameraCalibrator &cameraCalibrator)
{
	cv::Mat image;
	CameraCalibrator outerParameters;
	cv::Size boardSize(10,10);

	std::stringstream str;
	str << "D:\\img\\mychessboards\\17.bmp";
	std::cout << str.str() << std::endl;
	image = cv::imread(str.str(),0);

	std::vector<cv::Point2f> imageCorners;	// ���̽ǵ�
	std::vector<cv::Point3f> objectCorners;	// ��Ӧ����������

	// ��������(X,Y,Z)= (i,j,0)
	for (int i=0; i<boardSize.height; i++) 
	{
		for (int j=0; j<boardSize.width; j++) 
		{
			objectCorners.push_back(cv::Point3f((float)i, (float)j, 0.0f));
		}
	}

	// �ҵ����̽ǵ�
	bool found = cv::findChessboardCorners(image, boardSize, imageCorners);

	// ��ȡ���̽ǵ�������ؾ���
	cv::cornerSubPix(image, imageCorners, 
					 cv::Size(5,5), 
					 cv::Size(-1,-1), 
					 cv::TermCriteria(cv::TermCriteria::MAX_ITER +
						cv::TermCriteria::EPS, 
						30,		// ����������
						0.1));     // ��С����

	// �ҵ��õĽǵ������
	if (imageCorners.size() == boardSize.area()) 
	{
		outerParameters.addPoints(imageCorners, objectCorners);
	}
	else
	{
		std::cout << "û���ҵ���Ӧ�Ľǵ㣡���������գ�" << std::endl;
		return;
	}

	cv::Mat rvec, tvec;
	// �����ͼƬ���µ�����������rvec��ת������tvecת������
	cv::solvePnP(outerParameters.m_objectPoints[0],
				 outerParameters.m_imagePoints[0],
				 cameraCalibrator.m_cameraMatrix,
				 cameraCalibrator.m_distCoeffs,
				 rvec,
				 tvec);

	// ��ת����
	cv::Mat rt;
	// ��һ��3x1������ת��Ϊ3x3�ľ���matR
	cv::Rodrigues(rvec,rt);

	//---------------------------------
	for (int i=0; i<3; i++)
	{
		rt.at<double>(i,2) = tvec.at<double>(i);
	}
	//---------------------------------
	
	double s=0.0;	// �������������һ���ж��ٸ�����
	
	cv::Point2f p1 = outerParameters.m_imagePoints[0][0];
	cv::Point2f p2 = outerParameters.m_imagePoints[0][9];

	// ��ñ����ߣ�������һ��С�����α䳤��Ӧ�����ص�ĸ���
	// �������ߣ�Ȼ��ȡƽ��ֵ�ͺ���
	s = sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y))/9.0;

	// H = M*[r1 r2 t]
	cv::Mat H = cameraCalibrator.m_cameraMatrix * rt;
//	H = H * s;
	cv::Mat IH;
	invert(H,IH);

	// �������
//	cout << outerParameters.calculateDeviation(IH) << endl;
}

// ������
double CameraCalibrator::calculateDeviation(const cv::Mat &IH)
{
	// ����ͼƬ�е�ʮ���㵽��һ����ľ���
	cv::Point2f pointImg = m_imagePoints[0][9] - m_imagePoints[0][0];

	double leng = sqrt(pointImg.x*pointImg.x+pointImg.y*pointImg.y);

// 	cout << "ͼ�����������곤�ȣ�" << pointImg << endl;
// 	cout << "�������������㳤�ȣ�" << (Mat)pointImg << endl;
// 	cout << "��������֮�������ٺ��ף�" << (double)(leng/(9*12.0)) << endl;

	// test
	cv::Mat imgPoint1(3,1,cv::DataType<double>::type);
	cv::Mat imgPoint2(3,1,cv::DataType<double>::type);

	// ȡ�����10���ֱ�ߣ������
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

	// ȡ�����10���ֱ�ߣ������
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

	// ��������һ������
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
	// ��������һ������ ����

	return averageDeviation/20.0;
}
