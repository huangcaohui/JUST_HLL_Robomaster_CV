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

#ifndef ARMOUR_TRACKER_H
#define ARMOUR_TRACKER_H

#include "common.h"

/*OpenCV追踪算法库*/
#include <opencv2/video/tracking.hpp>
#include <opencv2/tracking.hpp>

namespace HCVC {
//! @addtogroup armour_recognition
//! @{

/**
 * @brief 装甲板追踪模块
 * @details 在装甲板检测得出最佳区域后，对该区域进行追踪
 */
class ArmourTracker
{
public:
    /**
    * @brief 初始化
    *
    */
    ArmourTracker();

    /**
    * @brief 创建图像追踪器并设定参数
    * @param[in] srcImage 待检测原图像
    * @param[in] armourBlock 需要追踪的矩形区域
    * @return null
    */
    void init(const Mat& srcImage, Rect2d armourBlock);

    /**
    * @brief 追踪目标区域
    * @param[in] srcImage 待检测原图像
    * @return 是否追踪成功，追踪成功返回true，追踪失败返回false
    */
    bool track(Mat srcImage);

    /**
     * @brief 对更新后的矩形框进行矫正
     * @details 将矩形框进行放大，在放大后矩形框内根据轮廓数量信息分类筛选
     * @param[in] updateRoi 更新后的矩形框
     * @param[in] srcImage 原图像
     * @return 矫正后的矩形框
     */
    Rect2d refineRect(Mat& updateRoi, Mat& srcImage);

    /**
     * @brief 对越界矩形框的进行矫正
     * @param[out] minArmourRect 矫正后的矩形框
     * @param[in] rotatedRect 更新后的矩形内连通域外接矩形
     * @return null
     */
    void refineOverBorder(RotatedRect& minArmourRect, RotatedRect* rotatedRect);

    /**
     * @brief 对未越界但只含一个连通域的矩形框进行矫正
     * @param[out] minArmourRect 矫正后的矩形框
     * @param[in] rotatedRect 更新后的矩形内连通域外接矩形
     * @return null
     */
    void refineNonOverBorder(RotatedRect& minArmourRect, RotatedRect* rotatedRect);

    /**
     * @brief 对更新后矩形框内连通域数量大于2的矩形框进行矫正
     * @details 分为单独更新后矩形内连通域匹配与更新后连通域与放大后其余连通域匹配
     * @details 将扩大后矩形内连通域分为两类，一类与更新后矩形框形同，其余为另一类，分别进行筛选
     * @param[out] minAreaRect 矫正后的矩形框
     * @param[in] rotatedRect 更新后的矩形内连通域外接矩形
     * @param[in] rotatedSize rotatedRect数组的长度
     * @param[in] boundNum flag,用于判断矩形匹配是否为空
     * @return null
     */
    void searchMatchDomains(RotatedRect& minAreaRect,
                            RotatedRect* rotatedRect,
                            unsigned int rotatedSize,
                            int& boundNum);

    /**
     * @brief 对扩大后矩形区域与原矩形区域团块的单个筛选
     * @param[in] blocks 原区域的团块
     * @param[in] blockSize blocks数组的长度
     * @param[out] clone 筛选后团块数组
     * @return num clone数组的实际储存数量
     */
    int cloneScreen(RotatedRect* blocks, unsigned int blockSize, RotatedRect* clone);

    /**
     * @brief 对分类后单独的更新后矩形内完整矩形矩形筛选匹配
     * @param[in] updateClone 通过单个矩形性质筛选出来的更新后矩形框
     * @param[in] updateNum updateClone数组内实际储存的矩形数量
     * @return 最后筛选出来的包围两连通域的旋转矩形
     */
    vector<RotatedRect> updateScreen(RotatedRect* updateClone, int updateNum);

    /**
     * @brief 对第一类分类后未成功匹配与第二类分类的矩形进行匹配筛选
     * @param[in] updateClone 通过单个矩形性质筛选出来的更新后矩形框
     * @param[in] updateNum updateClone数组内实际储存的矩形数量
     * @param[in] adjustClone 通过单个矩形筛选出来的扩大后矩形框
     * @param[in] adjustNum adjustClone数组的长度
     * @return 最后筛选出来的包围两连通域的旋转矩形
     */
    vector<RotatedRect> adjustScreen(RotatedRect* updateClone,
                                     int updateNum,
                                     RotatedRect* adjustClone,
                                     int adjustNum);

    /**
     * @brief 对经过分类后筛选出数量大于2的连通域进行评分，选出最优矩
     * @param[in] armours 筛选后数量大于2的装甲板
     * @return 最优矩
     */
    RotatedRect armourConfidence(vector<RotatedRect>& armours);

    /**
     * @brief 选出最优矩阵
     * @param[in] appraiseArmour 经过连通域检测的数组
     * @param[in] appraiseGrade 对应每个旋转矩形的分数
     * @param[in] num 数组的实际数量
     * @return 最优矩
     */
    RotatedRect sortArmour(RotatedRect* appraiseArmour,
                           double *appraiseGrade,
                           unsigned int num);

    /**
     * @brief 对越界的矩形框矩形矫正
     * @param[in] srcImage 原图像
     * @param[in] initRect 输入的矩形框
     */
    void correctBorders(const Mat srcImage, Rect2d& initRect);

    /**
     * @brief 获取分类后筛选出来的两矩形框的外接矩形
     * @param[in] matchDomains 筛选出来的两矩形框
     * @param[in] matchSize matchDomains数组的长度
     * @return 量矩形框的外接矩形
     */
    RotatedRect getArmourRotated(RotatedRect* matchDomains, int matchSize);

    Rect2d getArmourBlock() const;  

private:
    //! kcf匹配算法图像追踪器
    Ptr<TrackerKCF> tracker;

    //! 检测到的装甲板区域
    Rect2d armourBlock;
    //! 初始矩形框
    Rect2d initRect;

    //! 检测的装甲板区域
    Mat roi;
    //! 更新后的检测框
    Mat updateRoi;
    //! 调整后检测的装甲板区域
    Mat adjustRoi;
    //! 更新后的检测框V通道区域
    Mat updateValue;
    //! 调整后检装甲板区域V通道图
    Mat adjustValue;

    //! 初始图像装甲板长度
    float initArmourLength;
    //! 初始灯条最小外接矩形的旋转角
    float gamma;
};
//! @}
}
#endif // ARMOUR_TRACKER_H
