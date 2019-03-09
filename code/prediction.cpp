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
}

/*
void Prediction::predict(Mat &srcImage, Rect2d armourblock)
{
    //定义用来存放两个矩形上的点
    Point pt1 = armourblock.tl(), pt2 = armourblock.br();


    //进行预测,返回预测值x'(k)
    Mat prediction1 = KF1.predict();
    Mat prediction2 = KF2.predict();

    //定义两个点用于保存预测的点坐标
    Point predict_pt1 = Point2f(prediction1.at<float>(0), prediction1.at<float>(1));
    Point predict_pt2 = Point2f(prediction2.at<float>(0), prediction2.at<float>(1));

    //更新观测值，将最终识别到的矩形点坐标数据保存为每次预测的观测值
    measurement1 = (Mat_<float>(2,1) << pt1.x , pt1.y);
    measurement2 = (Mat_<float>(2,1) << pt2.x , pt2.y);
    
    //更新至观测值的最后版本，返回矫正后状态值x(k)
    Mat statePost1 = KF1.correct(measurement1);
    Mat statePost2 = KF2.correct(measurement2);

    //将最后更新结束的矩阵元素赋值到点坐标
    Point measure_pt1 = Point2f(statePost1.at<float>(0), statePost1.at<float>(1));
    Point measure_pt2 = Point2f(statePost2.at<float>(0), statePost2.at<float>(1));

   // rectangle(srcImage, measure_pt1 ,measure_pt2, Scalar(0, 0, 255), 2, 1);
    rectangle(srcImage, predict_pt1 ,predict_pt2, Scalar(0, 255, 0), 2, 1);
}
}
*/


/*
void Prediction::predict(Mat &srcImage, Rect2d armourblock)
{


    //定义用来存放两个矩形的点
    Point pt1 = armourblock.tl(), pt2 = armourblock.br();

    int i = 0;

    //定义两个矩阵用来表示状态
    Mat state1 = (Mat_<float>(4,1)<<armourblock.tl().x,armourblock.tl().y,0,0);
    Mat state2 = (Mat_<float>(4,1)<<armourblock.br().x,armourblock.br().y,0,0);

    //定义噪声
    Mat w_k1(4,1,CV_32F);
    Mat w_k2(4,1,CV_32F);

   // Mat v_k1(2,1,CV_32F);
   // Mat v_k2(2,1,CV_32F);


    //randn(w_k1,0,sqrt((double)KF1.processNoiseCov.at<float>(0,0)));
    //randn(w_k1,0,sqrt((double)KF2.processNoiseCov.at<float>(0,0)));

    //预测
    Mat prediction1 = KF1.predict();
    Mat prediction2 = KF2.predict();

    //定义两个点用于保存预测的点坐标
    Point predict_pt1 = Point2f(prediction1.at<float>(0), prediction1.at<float>(1));
    Point predict_pt2 = Point2f(prediction2.at<float>(0), prediction2.at<float>(1));


     randn( measurement1, Scalar::all(0), Scalar::all(KF1.measurementNoiseCov.at<float>(0)));
     randn( measurement2, Scalar::all(0), Scalar::all(KF2.measurementNoiseCov.at<float>(0)));

    // randn(v_k1,0,sqrt((double)KF1.measurementNoiseCov.at<float>(0,0)));
    // randn(v_k1,0,sqrt((double)KF2.measurementNoiseCov.at<float>(0,0)));


      measurement1 += KF1.measurementMatrix * state1 ;
      measurement2 += KF2.measurementMatrix * state2 ;

      Mat statePost1 = KF1.correct(measurement1);
      Mat statePost2 = KF2.correct(measurement2);

      Point measure_pt1 = Point2f(statePost1.at<float>(0), statePost1.at<float>(1));
      Point measure_pt2 = Point2f(statePost2.at<float>(0), statePost2.at<float>(1));

     // rectangle(srcImage, measure_pt1 ,measure_pt2, Scalar(0, 0, 255), 2, 1);
      rectangle(srcImage, predict_pt1 ,predict_pt2, Scalar(0, 255, 0), 2, 1);

     // KF1.correct(measurement1);
     // KF2.correct(measurement2);

      randn( w_k1, 0, sqrt((double)KF1.processNoiseCov.at<float>(0, 0)));
      state1 = KF1.transitionMatrix*state1 + w_k1 ;

      randn( w_k2, 0, sqrt((double)KF2.processNoiseCov.at<float>(0, 0)));
      state2 = KF2.transitionMatrix*state2 + w_k2;



  }
}

*/

void Prediction::predict(Mat &srcImage, Rect2d armourblock)
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

   // Mat v_k1(2,1,CV_32F);
   // Mat v_k2(2,1,CV_32F);

    //randn(w_k1,0,sqrt((double)KF1.processNoiseCov.at<float>(0,0)));
    //randn(w_k1,0,sqrt((double)KF2.processNoiseCov.at<float>(0,0)));

    //预测
    Mat prediction1 = KF1.predict();
    Mat prediction2 = KF2.predict();

    //定义两个点用于保存预测的点坐标
    Point predict_pt1 = Point2f(prediction1.at<float>(0), prediction1.at<float>(1));
    Point predict_pt2 = Point2f(prediction2.at<float>(0), prediction2.at<float>(1));

    //初始化测量值
    randn( measurement1, Scalar::all(0), Scalar::all(KF1.measurementNoiseCov.at<float>(0)));
    randn( measurement2, Scalar::all(0), Scalar::all(KF2.measurementNoiseCov.at<float>(0)));

    // randn(v_k1,0,sqrt((double)KF1.measurementNoiseCov.at<float>(0,0)));
    // randn(v_k1,0,sqrt((double)KF2.measurementNoiseCov.at<float>(0,0)));

    //使用第二个公式更新观测值
    measurement1 += KF1.measurementMatrix * state1 ;
    measurement2 += KF2.measurementMatrix * state2 ;

    //更新至观测值的最后版本，返回矫正后状态值x(k)
    Mat statePost1 = KF1.correct(measurement1);
    Mat statePost2 = KF2.correct(measurement2);

    //保存测量值
    Point measure_pt1 = Point2f(statePost1.at<float>(0), statePost1.at<float>(1));
    Point measure_pt2 = Point2f(statePost2.at<float>(0), statePost2.at<float>(1));

    //对于横坐标的预测值和状态值之间的关系进行一些限制
    //首先是左下角点
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

      //然后是右上角点
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
      //rectangle(srcImage, measure_pt1 ,measure_pt2, Scalar(0, 0, 255), 2, 1);
      rectangle(srcImage, predict_pt1 ,predict_pt2, Scalar(0, 255, 0), 2, 1);

      // KF1.correct(measurement1);
      // KF2.correct(measurement2);

      // randn( w_k1, 0, sqrt((double)KF1.processNoiseCov.at<float>(0, 0)));
      // state1 = KF1.transitionMatrix*state1 + w_k1 ;

      // randn( w_k2, 0, sqrt((double)KF2.processNoiseCov.at<float>(0, 0)));
      // state2 = KF2.transitionMatrix*state2 + w_k2;
  }


}

