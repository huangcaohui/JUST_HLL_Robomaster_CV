#include "prediction.h"
#include "math.h"

namespace HCVC
{
Prediction::Prediction()
{
    
}

//初始化卡尔曼滤波器
void Prediction::init()
{
    const int stateNum=4;                                         //状态值4×1向量(x,y,△x,△y)
    const int measureNum=2;                                       //测量值2×1向量(x,y)

    //定义卡尔曼滤波器初始参数值
    KF1.init(stateNum, measureNum, 0);
    KF1.transitionMatrix = (Mat_<float>(4, 4) << 1,0,1,0,0,1,0,1,0,0,1,0,0,0,0,1);   //转移矩阵A
    setIdentity(KF1.measurementMatrix);                                              //测量矩阵H
    setIdentity(KF1.processNoiseCov, Scalar::all(5e-3));                             //系统噪声方差矩阵Q
    setIdentity(KF1.measurementNoiseCov, Scalar::all(5e-3));                         //测量噪声方差矩阵R
    setIdentity(KF1.errorCovPost, Scalar::all(5e-3));                                //后验错误估计协方差矩阵P

    KF2.init(stateNum, measureNum, 0);
    KF2.transitionMatrix = (Mat_<float>(4, 4) << 1,0,1,0,0,1,0,1,0,0,1,0,0,0,0,1);   //转移矩阵A
    setIdentity(KF2.measurementMatrix);                                              //测量矩阵H
    setIdentity(KF2.processNoiseCov, Scalar::all(5e-3));                             //系统噪声方差矩阵Q
    setIdentity(KF2.measurementNoiseCov, Scalar::all(5e-3));                         //测量噪声方差矩阵R
    setIdentity(KF2.errorCovPost, Scalar::all(5e-3));                                //后验错误估计协方差矩阵P

    randn(KF1.statePost, Scalar::all(0), Scalar::all(0.1));
    randn(KF2.statePost, Scalar::all(0), Scalar::all(0.1));

    //初始测量值x'(0)初始化
    measurement1 = Mat::zeros(2, 1, CV_32F);
    measurement2 = Mat::zeros(2, 1, CV_32F);

    const int StateNum1 = 1;
    const int MeasureNum1 = 1;

    //初始化参数
    KF.init(StateNum1, MeasureNum1, 0);
    KF.transitionMatrix = (Mat_<float>(1,  1) << 1.1);                                //转移矩阵
    setIdentity(KF.measurementMatrix);                                                //测量矩阵
    setIdentity(KF.processNoiseCov,Scalar::all(5e-3));                                //系统噪声协方差矩阵
    setIdentity(KF.measurementNoiseCov,Scalar::all(5e-3));                            //测量噪声协方差矩阵
    setIdentity(KF.errorCovPost,Scalar::all(5e-3));

    randn(KF.statePost, Scalar::all(0), Scalar::all(0.1));

    //初始测量值的定义
    measurement = Mat::zeros(1, 1, CV_32F);
}

Rect2d Prediction::predict(const Mat srcImage, Rect2d armourblock)
{
    //定义用来存放两个矩形的点
    Point pt1 = armourblock.tl(), pt2 = armourblock.br();

    //定义反卡尔曼预测参数
    float anti_range = 0.5;
    float threshold = 20;

    //从横坐标开始
    Point2f anti_KalmanPoint1(pt1.x, pt1.y);
    Point2f anti_KalmanPoint2(pt2.x, pt2.y);

    //定义两个矩阵用来表示状态
    Mat state1 = (Mat_<float>(4,1) << armourblock.tl().x, armourblock.tl().y, 0, 0);
    Mat state2 = (Mat_<float>(4,1) << armourblock.br().x, armourblock.br().y, 0, 0);

    //定义噪声
    Mat w_k1(4, 1, CV_32F);
    Mat w_k2(4, 1, CV_32F);

    //预测
    Mat prediction1 = KF1.predict();
    Mat prediction2 = KF2.predict();

    //定义两个点用于保存预测的点坐标
    Point2f predict_pt1 = Point2f(prediction1.at<float>(0), prediction1.at<float>(1));
    Point2f predict_pt2 = Point2f(prediction2.at<float>(0), prediction2.at<float>(1));

    //初始化测量值
    randn(measurement1, Scalar::all(0), Scalar::all(KF1.measurementNoiseCov.at<double>(0)));
    randn(measurement2, Scalar::all(0), Scalar::all(KF2.measurementNoiseCov.at<double>(0)));

    //使用第二个公式更新观测值
    measurement1 += KF1.measurementMatrix * state1 ;
    measurement2 += KF2.measurementMatrix * state2 ;

    //更新至观测值的最后版本，返回矫正后状态值x(k)
    Mat statePost1 = KF1.correct(measurement1);
    Mat statePost2 = KF2.correct(measurement2);

    //对于横坐标的预测值和状态值之间的关系进行一些限制
    //首先是左上角点
    if(state1.at<float>(0) + anti_range * (state1.at<float>(0) - predict_pt1.x) >= 0
            || state1.at<float>(0) + anti_range * (state1.at<float>(0) - predict_pt1.x) <1080)
    {
        if(state1.at<float>(0) - predict_pt1.x > 0)
        {
            if(abs(state1.at<float>(0) - predict_pt1.x) > threshold)
            {
                anti_KalmanPoint1.x = state1.at<float>(0) - anti_range * (state1.at<float>(0) - predict_pt1.x);
            }
            else
            {
                anti_KalmanPoint1.x = state1.at<float>(0);
            }
        }
        if(state1.at<float>(0) - predict_pt1.x < 0)
        {
            if(abs(state1.at<float>(0) - predict_pt1.x) >threshold)
            {
                anti_KalmanPoint1.x = state1.at<float>(0) - anti_range * (state1.at<float>(0) - predict_pt1.x);
            }
            else
            {
                anti_KalmanPoint1.x = state1.at<float>(0);
            }

        }

    }

    //然后是右下角点
    if(state2.at<float>(0) + anti_range * (state2.at<float>(0) - predict_pt2.x) >= 0
            || state2.at<float>(0) + anti_range * (state2.at<float>(0) - predict_pt2.x) <1080)
    {
        if(state2.at<float>(0) - predict_pt2.x > 0)
        {
            if(abs(state2.at<float>(0) - predict_pt2.x) >threshold)
            {
                anti_KalmanPoint2.x = state2.at<float>(0) - anti_range * (state2.at<float>(0) - predict_pt2.x);
            }
            else
            {
                anti_KalmanPoint2.x = state2.at<float>(0);
            }
        }
        if(state2.at<float>(0) - predict_pt2.x < 0)
        {
            if(abs(state2.at<float>(0) - predict_pt2.x) > threshold)
            {
                anti_KalmanPoint2.x = state2.at<float>(0) - anti_range * (state2.at<float>(0) - predict_pt2.x);
            }
            else
            {
                anti_KalmanPoint2.x = state2.at<float>(0);
            }

        }

    }

    //将最后完成修改的预测值保存
    predict_pt1.x = anti_KalmanPoint1.x;
    predict_pt2.x = anti_KalmanPoint2.x;

    //画出最终测量值和预测值
    return correctBorders(srcImage, Rect2d(predict_pt1, predict_pt2));
}

double Prediction::fre_predict(bool *frequency, int count, int n)
{
    //定义概率
    double probability = 0;

    //迭代系数
    double Iteration_coefficient = 0.335;

    //开始进行概率的获取   注意：下标([i])与余数(count%n)相等
    int i ;

    //首先从5帧的最后部分开始
    for(i = count%n; i >= 0; --i)
    {
        probability += frequency[i]*Iteration_coefficient;
        Iteration_coefficient -= 0.067;
    }

    //然后从这5帧的开头开始
    if(count%n != n - 1)
    {
        Iteration_coefficient = 0.067;
        for(i = (count%n + 1); i < n; ++i)
        {
            probability += frequency[i]*Iteration_coefficient;
            Iteration_coefficient += 0.067;
        }
    }

    //下面开始卡尔曼滤波的预测部分
    //首先定义矩阵用来存放概率值
    Mat state = (Mat_<float>(1,1) << probability);

    //预测
    Mat prediction = KF.predict();

    //初始化测量值
    randn(measurement,Scalar::all(0), Scalar::all(KF.measurementNoiseCov.at<double>(0)));

    //使用卡尔曼滤波第二个公式获得测量值
    measurement += KF.measurementMatrix * state;

    //校验更新结果
    Mat statePost = KF.correct(measurement);

    if(probability < 0)
    {
        probability = 0;
    }

    return double(prediction.at<float>(0));
}


void Prediction::fre_fillArmourBlock(Mat image, bool *frequency,int n,int &count,
                                     Rect2d &predictBlock, Rect2d &armourBlock, bool &findArmourBlock)
{
    if(findArmourBlock == true)
    {
        predictBlock = predict(image, armourBlock);
        frequency[count%n] = true;
    }
    else
    {
        if(fre_predict(frequency, count, n) >= 0.3)
        {
            armourBlock = predictBlock;
            findArmourBlock = true;           
        }
        frequency[count%n] = false;
    }

    ++count;
}

void Prediction::fillArmourBlock(Mat image, bool *frequency, int n, int &count,
                                 Rect2d &predictBlock, Rect2d &armourBlock, bool &findArmourBlock)
{
    if(findArmourBlock == true)
    {
        predictBlock = predict(image, armourBlock);
        frequency[count%n] = true;
    }
    else
    {
        int i = 0;

        for(int j = 0; j < n; j++)
        {
            i += frequency[j];
        }

        if(i == n)
        {
            armourBlock = predictBlock;
            findArmourBlock = true;
        }

        frequency[count%n] = false;
    }

    ++count;
}

Rect2d Prediction::correctBorders(const Mat srcImage, Rect2d initRect)
{
    Rect2d region = initRect;

    //对越界矩形边界进行纠正
    if (initRect.x < 0)
    {
        initRect.x = 0;
        initRect.width += region.x;
    }
    if (initRect.x + initRect.width > srcImage.cols)
    {
        initRect.width = srcImage.cols - region.x;
    }
    if (initRect.y < 0)
    {
        initRect.y = 0;
        initRect.height += region.y;
    }
    if ( initRect.y + initRect.height > srcImage.rows)
    {
        initRect.height = srcImage.rows - region.y;
    }

    return initRect;
}
}
