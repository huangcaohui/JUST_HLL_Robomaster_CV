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

#ifndef RANGING_H
#define RANGING_H

#include "common.h"

/*OpenCV测距算法库*/
#include <opencv2/calib3d.hpp>

namespace HCVC
{
//! @addtogroup ranging
//! @{

/**
 * @brief 对识别到的目标进行测距
 * @details 在识别到目标后，通过PNP算法对目标进行测距
 */
class Ranging
{
public:
    /**
     * @brief 初始化
     *
     */
    Ranging();

    /**
     * @brief 读取摄像机矩阵参数
     * @param[in] xmlPath 需要打开的xml文件路径
     * @return 是否初始化成功
     */
    bool init(string xmlPath);

    /**
     * @brief 计算相机与装甲板的距离
     * @details 选取四组点，通过PNP进行求解
     * @param[in] rect 识别到的装甲板的最小外接矩形
     * @return 计算出的距离
     */
    double calDistance(RotatedRect rect);

    //! 摄像机的标定参数
    struct CameraParams
    {
        Size imageSize;                     /*!< 图像分辨率 */
        Mat	cameraMatrix;			        /*!< 摄像机内参矩阵 */
        Mat	distCoeff;	        /*!< 摄像机畸变参数 */
        double skew;                        /*!< 摄像机x,y轴的垂直偏差 */
    }params;

private:
    /**
     * @brief 将每个节点中的参数写入相应的矩阵
     * @param[in] 标定文件的节点
     * @return 标定参数的矩阵
     */
    Mat writeMatrix(FileNode node);

    /**
     * @brief 将矩形的四个点进行分组
     * @details 以矩形左下角第一个点为p0，逆时针方向将点存入容器
     * @param[in] rect 输入的最小外接矩形
     * @return 按顺序存储的容器
     */
    vector<Point2f> dividePoints(RotatedRect rect);
};
//! @}
}
#endif // RANGING_H
