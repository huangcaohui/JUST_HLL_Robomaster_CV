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

/**
* @defgroup armour_recognition 装甲板识别模块组
*
* @defgroup device 设备驱动模块组
*
* @defgroup control 总控制模块组
*
* @defgroup debug 调试模块组
*
* @defgroup ranging 测距模块组
*/

#ifndef MAIN_H
#define MAIN_H

/*C++标准库*/
#include <time.h>

/*自定义库*/
#include "armour_detector.h"
#include "armour_tracker.h"
#include "prediction.h"
#include "serial.h"
#include "camera.h"
#include "video.h"
#include "ranging.h"

/**
* @brief HLL Computer Vision Code namepace.
*
*/
namespace HCVC {
//! @addtogroup control
//! @{
/**
 * @brief 系统总体逻辑控制
 * @details 包括装甲板的识别与追踪，大神符检测，串口通信模块的协调控制
 */
class Control
{
public:
    /**
    * @brief 初始化
    *
    */
    Control();

    /**
    * @brief 运行整体系统并显示运行结果
    * @return null
    */
    void run();

protected:
    //! 图像检测器，处理并分析图像找出装甲的初始位置
    ArmourDetector armourDetector;

    //! 运动追踪器，对经过图像检测后找到的灯柱区域跟踪
    ArmourTracker armourTracker;

    //! 装甲板预测器，对检测到的装甲板进行预测并更新
    Prediction prediction;

private:
    //! 装甲板检测状态常量
    enum
    {
        DETECTING, /*!< 检测状态 */
        TRACKING,  /*!< 跟踪状态 */
    };

    //! 当前装甲板检测程序的状态
    int status;

    //! 串口通信类
    Serial serial;

    //! 摄像头
    Camera camera;

    //! 视频
    Video video;

    //! 测距
    Ranging ranging;
};
//! @}
}
#endif // MAIN_H
