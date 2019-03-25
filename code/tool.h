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
#ifndef TOOL_H
#define TOOL_H

/*公共头文件*/
#include "common.h"

namespace HCVC {
//! @addtogroup debug
//! @{
//! 滑动控制条当前位置
extern int g_trackBarLocation;

/**
 * @brief 提供一些调试用工具
 * @details 包括视频播放控制的功能实现
 */
class Tool
{
public:
    /**
    * @brief 初始化
    *
    */
    Tool()
    {

    }

    //! 视频播放控制状态常量
    enum
    {
        ESC = 27,               /*!< 退出 */
        PAUSE = ' ',            /*!< 暂停 */
        MOVE_BACK = 'j',        /*!< 回放一帧 */
        MOVE_FORWARD = 'k',     /*!< 快进一帧 */
    };

    /**
    * @brief 添加滑动控制条
    * @details 通过滑动控制条可以控制视频播放进度
    * @param[in] windowName 添加滑动控制条的窗口名称
    * @param[in] file 用于被滑动控制条控制的视频文件
    * @return null
    */
    static void addTrackBar(const string& windowName, VideoCapture& file);

    /**
    * @brief 添加滑动控制条跟随视频进度功能
    * @details 使滑动控制条能够跟随视频播放进度同步移动
    * @param[in] windowName 添加滑动控制条的窗口名称
    * @param[in] file 用于被滑动控制条跟随的视频文件
    * @return null
    * @warning 滑动控制条跟随视频播放进度极其耗费时间，影响程序运行效率，需慎重使用
    */
    static void setTrackBarFollow(const string& windowName, const VideoCapture& file);

    /**
    * @brief 添加键盘按键控制
    * @details 使能够通过键盘快捷键控制视频的播放，停止，结束
    * @param[in] srcFile 需要控制进度的视频文件
    * @param[in] delay 视频播放的每一帧时间间隔
    * @return null
    */
    static void addKeyboardControl(VideoCapture& srcFile, const int& delay = 1);

    //! 开始或结束标志
    enum
    {
        BEGIN,
        END
    };

    /**
    * @brief 添加运行时间统计
    * @details 对开始到结束标志之间的代码块的运行时间进行统计，分析程序运行效率
    * @param[in] id 统计运行时间的代码块编号, 范围:[0, 100)
    * @param[in] tag 对应代码块开始或结束标志
    * @param[in] timeCountName 运行时间统计名称，将会在输出中显示
    * @return null
    * @note 实际计算时间是通过两次调用该函数的时间差来实现
    */
    static void setTimeCount(const int& id, const int& tag, const string& timeCountName);

    /**
     * @brief 在图中画出装甲板的中心点
     * @param resizeFrame[in] 待画图的原图像
     * @param points 存储中心点的数组
     * @param armourBlock 待获取中心点的装甲板
     */
    static void drawPoints(Mat resizeFrame, vector<Point>& points, Rect2d armourBlock);

    /**
     * @brief 画坐标系
     * @param resizeFrame 输入的原始图像
     * @param null
     */
    static void drawCoord(Mat resizeFrame);

    /**
     * @brief 显示检测的坐标
     * @param resizeFrame 输入的原始图像
     * @param coord 输入的坐标
     * @param org_x 数字显示的横坐标位置
     * @param org_y 数字显示的纵坐标位置
     */
    static void showPoints(Mat resizeFrame, short coord, int org_x, int org_y);

    /**
    * @brief 在原图像上画出类型为vector旋转矩形，便于调试
    * @param[in] srcImage 待检测原图像
    * @param[in] minRotatedRects 需要画出的全部旋转矩形
    * @param[in] color 线条颜色
    * @return null
    */
    static void drawBlocks(Mat &srcImage,
                           const vector<RotatedRect>& minRotatedRects,
                           const Scalar& color);

    /**
     * @brief 在原图上画出类型为array的旋转矩形
     * @param[in] srcImage 待检测原图像
     * @param[in] minRotatedRects 类型为数组的最小外接矩形
     * @param[in] armoursNum minRotatedRects的实际数量
     * @param[in] color 矩形框的颜色
     */
    static void drawBlocks(Mat &srcImage,
                           const RotatedRect* minRotatedRects,
                           int armoursNum,
                           const Scalar& color);

    /**
     * @brief 在原图上画出类型为RotatedRect的旋转矩形
     * @param[in] srcImage 待检测原图像
     * @param[in] minRotatedRects 最小外接矩形
     * @param[in] color 矩形框的颜色
     */
    static void drawBlocks(Mat &srcImage,
                      const RotatedRect minRotatedRects,
                      const Scalar& color);

    /**
     * @brief 将短型字符转字符型
     * @param[in] num 输入的短型字符
     * @return 转换后的字符型字符
     */
    static string toString(short num);

private:
    /**
    * @brief 滑动控制条回调函数
    * @param[in] pos 滑动控制条的当前位置
    * @param[in] data 额外传递的数据
    * @return null
    * @note 回调函数在类中只能设置为静态函数
    */
    static void onTrackBarCallback(int pos, void* data);
};
//! @}
}
#endif // TOOL_H
