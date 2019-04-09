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
     * @return 预测的框
     */
    Rect2d predict(const Mat srcImage, Rect2d armourblock);

    /**
     * @brief 预测画框的概率
     * @param[in] frequency 用于进行概率统计和生成初始概率值的数组
     * @param[in] count 帧数统计
     * @param[in] n 数组长度
     * @return 返回预测框的概率
     */
    double fre_predict(bool *frequency, int count, int n);

    /**
    * @brief  在进行概率预测的同时进行填补
    * @param[in] image 输入图像
    * @param[in] frequency 用于进行概率统计和生成初始概率值的数组
    * @param[in] n 数组长度
    * @param[in] count 帧数统计
    * @param[in] predictBlock 预测暂存框
    * @param[in] armourBlock 用于数据传输的装甲板
    * @param[in] findArmourBlock 是否发现装甲板
    * @return null
    */
    void fre_fillArmourBlock(Mat image, bool *frequency, int n,
                             int &count1, Rect2d &predictBlock, Rect2d &armourBlock, bool &findArmourBlock);

    /**
     * @brief 填补检测与跟踪的空缺帧
     * @param[in] image 输入图像
     * @param[in] frequency 用于判断填补的数据频率
     * @param[in] n frequency的数据长度
     * @param[in] count 当前总的帧数目
     * @param[out] predictBlock 预测打存储框
     * @param[out] armourBlock 用于数据传输的装甲板
     * @param[out] findArmourBlock 检测与跟踪是否有装甲板
     * @return null
     */
    void fillArmourBlock(Mat image, bool *frequency, int n,
                         int &count, Rect2d &predictBlock, Rect2d &armourBlock, bool &findArmourBlock);

    /**
     * @brief 对越界的矩形框矩形矫正
     * @param[in] srcImage 原图像
     * @param[in] initRect 输入的矩形框
     * @return 矫正后的矩形框
     */
    Rect2d correctBorders(const Mat srcImage, Rect2d initRect);

private:
    //! 卡尔曼滤波器1
    KalmanFilter KF1;

    //! 卡尔曼滤波器2
    KalmanFilter KF2;

    //!概率预测卡尔曼滤波器
    KalmanFilter KF;
    
    //! 初始测量值x'(0)，用于后面更新
    Mat measurement1, measurement2, measurement;
};
}

#endif // PREDICTION_H
