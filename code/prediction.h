/*****************************************************************************
*  HLL Computer Vision Code                                                  *
*  Copyright (C) 2018 HLL  1915589872@qq.com.                                *
*                                                                            *
*  This file is part of HCVC.                                                *
*                                                                            *
*  @file     armour_detector.h                                               *
*  @brief    Detect the armour zone                                          *
*  Details.                                                                  *
*                                                                            *
*  @author   HLL                                                             *
*  @email    1915589872@qq.com                                               *
*  @version  1.0.0.0                                                         *
*  @date     2018.12.1                                                       *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark         : Description                                              *
*----------------------------------------------------------------------------*
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2018/12/1  | 1.0.0.0   | Huang Cao hui  | Create file                     *
*----------------------------------------------------------------------------*
*                                                                            *
*****************************************************************************/

#ifndef PREDICTION_H
#define PREDICTION_H

#include "common.h"

/*OpenCV追踪算法库*/
#include <opencv2/video/tracking.hpp>

namespace HCVC{
//! @addtogroup armour_recognition
//! @{

/**
 * @brief 装甲板追踪模块
 * @details 在装甲板检测得出最佳区域后，对该区域进行预测
 */
class Prediction
{
public:
    /**
     * @brief 初始化
     *
     */
    Prediction();

    /**
     * @brief 创建卡尔曼滤波器并设定参数
     * @return null
     */
    void init();
    
    /**
     * @brief 对装甲板做预测并更新
     * @param[in] srcImage 待检测原图像
     * @param[in] armourblock 待预测装甲板外接矩形
     * @return null
     */
    void predict(Mat &srcImage, Rect2d armourblock);

private:
    //! 卡尔曼滤波器1
    KalmanFilter KF1;

    //! 卡尔曼滤波器2
    KalmanFilter KF2;
    
    //! 初始测量值x'(0)，用于后面更新
    Mat measurement1, measurement2;
};
}

#endif // PREDICTION_H
