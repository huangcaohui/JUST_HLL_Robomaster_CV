#include "ranging.h"

namespace HCVC
{
Ranging::Ranging()
{

}

bool Ranging::init(string xmlPath)
{
    //读取左摄像头内参与畸变矩阵
    FileStorage cameraParams(xmlPath, FileStorage::READ);

    if (!cameraParams.isOpened())
    {
        cout << "Open " << xmlPath << " file failed" << endl;
        return false;
    }

    //节点
    FileNode _imgSize = cameraParams["imageSize"];
    FileNode _cameraMatrix = cameraParams["camera-matrix"];
    FileNode _distortion = cameraParams["distortion"];
    FileNode _skew = cameraParams["skew"];

    //图像分辨率
    vector<int> distinguishability;
    _imgSize["data"] >> distinguishability;
    Size imgSize(distinguishability[0], distinguishability[1]);

    //X,Y轴倾斜角
    double skew;
    _skew["data"] >> skew;

    //xml写入摄像头分辨率，内参矩阵，畸变矩阵，x,y轴的垂直偏差
    params = {imgSize, writeMatrix(_cameraMatrix), writeMatrix(_distortion), skew};

#ifdef DEBUG
    cout << "imgSize_left:" << endl << params.imageSize << endl;
    cout << "cameraMatrix_left:" << endl << params.cameraMatrix << endl;
    cout << "distortion_left:" << endl << params.distCoeff << endl;
    cout << "skew_left:" << endl << params.skew << endl;
    cout << endl;
#endif

    return true;
}

Mat Ranging::writeMatrix(FileNode node)
{
    int rows, cols;
    vector<double> vec;

    node["rows"] >> rows;
    node["cols"] >> cols;
    node["data"] >> vec;

    //将摄像头数据写入矩阵
    Mat M(rows, cols, CV_64FC1);
    for(int i = 0; i < rows; ++i)
    {
        for(int j = 0; j < cols; ++j)
        {
            M.at<double>(i, j) = (vec[i*cols + j]);
        }
    }

    return M;
}

double Ranging::calDistance(RotatedRect rect)
{
    //定义世界坐标点与像素坐标点
    vector<Point3f> objectPoints;
    vector<Point2f> imagePoints = dividePoints(rect);

    //世界坐标点与像素坐标点装入
    objectPoints.push_back(Point3f(-67.5, -27.5, 0));
    objectPoints.push_back(Point3f(-67.5, 27.5, 0));
    objectPoints.push_back(Point3f(67.5, 27.5, 0));
    objectPoints.push_back(Point3f(67.5, -27.5, 0));

    //定义旋转向量，旋转矩阵，平移矩阵
    Mat rVec, tVec;
    Mat_<float> rMat(3, 3);

    //PNP求解
    solvePnP(objectPoints, imagePoints, params.cameraMatrix, params.distCoeff, rVec, tVec);

    rVec.convertTo(rVec, CV_32F);

    //罗德里格斯变换转为旋转矩阵
    Rodrigues(rVec, rMat);

    double distance = 0;

    for(int j = 0; j < tVec.rows; ++j)
    {
        distance += pow(tVec.at<double>(j, 0), 2);
    }

    distance = sqrt(distance)/2;

    return distance;
}

vector<Point2f> Ranging::dividePoints(RotatedRect rect)
{
    //将点存入数组
    Point2f points[4];
    rect.points(points);

    //求出四个点的横坐标中点
    float centerx = 0;
    for(unsigned int i = 0; i < 4; ++i)
    {
        centerx += points[i].x;
    }
    centerx /= 4;

    //通过横坐标中点将这组点分为左右两对
    Point2f leftPts[2];
    Point2f rightPts[2];
    int leftNum = 0, rightNum = 0;

    for(unsigned int i = 0; i < 4; ++i)
    {
        if(points[i].x < centerx)
        {
            leftPts[leftNum++] = points[i];
        }
        else
        {
            rightPts[rightNum++] = points[i];
        }
    }

    vector<Point2f> imagePoints;

    imagePoints.push_back(leftPts[0].y > leftPts[1].y ? leftPts[0] : leftPts[1]);
    imagePoints.push_back(leftPts[0].y < leftPts[1].y ? leftPts[0] : leftPts[1]);
    imagePoints.push_back(rightPts[0].y < rightPts[1].y ? rightPts[0] : rightPts[1]);
    imagePoints.push_back(rightPts[0].y > rightPts[1].y ? rightPts[0] : rightPts[1]);

    return imagePoints;
}
}
