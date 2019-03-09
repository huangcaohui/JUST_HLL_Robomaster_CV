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

#ifndef VIDEO_H
#define VIDEO_H

#include "common.h"

namespace HCVC
{
//! @addtogroup deivce
//! @{

/**
* @brief 视频模块
* @details 视频文件的初始化和读取图像
*/
class Video
{
public:
    Video();

    /**
    * @brief 设置视频文件读取路径
    * @details 从指定路径读取视频文件
    * @param[in] path 读取视频文件的路径
    * @return 视频文件是否读取成功。
    *         返回true，表示文件读取成功；
    *         返回false，表示文件读取失败
    */
    bool init(string path);

    /**
    * @brief 读取视频流下一帧图像
    * @param[out] frame 下一帧图像
    * @return null
    */
    void getNextFrame(Mat& frame);

    /**
    * @brief 读取视频流下一帧图像
    * @param[out] frame 下一帧图像
    * @return 视频流数据结构
    */
    VideoCapture &operator >> (Mat& frame);

    /**
     * @brief 访问私有成员srcFile
     * @return 私有成员srcFile
     */
    VideoCapture &getVideo();

private:
    //! 存储读取的视频数据
    VideoCapture srcFile;

    //! 读取视频路径
    string srcFilePath;
};
//! @}
}

#endif // VIDEO_H
