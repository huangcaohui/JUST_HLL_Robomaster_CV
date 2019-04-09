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

#ifndef CAMERA_H
#define CAMERA_H

/*公共头文件*/
#include "common.h"

/*Qt库*/
#include <QDomDocument>
#include <QtCore/QFile>

namespace HCVC
{
//! @addtogroup deivce
//! @{

/**
* @brief 摄像头模块
* @details 摄像头的初始化和读取图像
*/
class Camera
{
public:
    Camera();

    /**
    * @brief 设置读取摄像头源
    * @details 从指定摄像头编号读取视频流
    * @param[in] cameraId 读取摄像头编号
    * @param[in] xmlPath 需要打开的xml文件路径
    * @return 摄像头是否打开成功。
    *         返回true，表示摄像头打开成功；
    *         返回false，表示摄像头打开失败
    */
    bool init(int cameraId, string xmlPath);

    /**
     * @brief 读取摄像头参数并写入xml文件
     * @brief[in] 需要打开的xml文件路径
     * @return null
     */
    bool writeCamParams(string xmlPath);

    /**
    * @brief 读取视频流下一帧图像
    * @param[out] frame 下一帧图像
    * @return 视频流数据结构
    */
    VideoCapture& operator >> (Mat& frame);

    /**
     * @brief 访问私有成员srcFile
     * @return 私有成员srcFile
     */
    VideoCapture &getCamera();

    //! 摄像头参数
    struct Params
    {
        double brightness;             /*!< 亮度 */
        double contrast;               /*!< 对比度 */
        double hue;                    /*!< 色调 */
        double saturation;             /*!< 饱和度 */
        double pan;                    /*!< 清晰度 */
        double gamma;                  /*!< 伽马 */
        double white_balance_red_v;    /*!< 白平衡 */
        double backlight;              /*!< 逆光对比 */
        double gain;                   /*!< 增益 */
        double exposure;               /*!< 曝光 */
    }params;

private:
    //! 存储摄像头视频流
    VideoCapture srcFile;

    //! 摄像头编号
    string cameraId;
};
//! @}
}

#endif // CAMERA_H
