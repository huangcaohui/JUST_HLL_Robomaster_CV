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

#ifndef SERIAL_H
#define SERIAL_H

/*公共头文件*/
#include "common.h"

/*Qt库*/
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTimer>

/*自定义库*/
#include "tool.h"

namespace HCVC {
//! @addtogroup device
//! @{

/**
 * @brief 串口通信模块组
 * @details 提供和电控部分串口通信功能，发送相对坐标，接收反馈信息
 */
class Serial: public QSerialPort
{    
    Q_OBJECT

public:
    /**
    * @brief 初始化
    *
    */
    Serial();

    /**
    * @brief 初始化串口，设定串口通信参数
    * @param[in] portName 串口名称
    * @return 串口初始化是否成功
    *         返回true，表示初始化成功；
    *         返回false，表示串口初始化出现错误
    */
    bool init(QString portName);

    /**
    * @brief 向串口写入相对坐标
    * @details 串口写入四字节数据。前两个字节为一个短整型数，表示x轴相对坐标；后两个字节为
    *          也为一个短整型数，表示y轴相对坐标。都为小端模式，即低字节节在前，高字节在
    *          后
    * @param[in] xDiff x轴相对坐标
    * @param[in] yDiff y轴相对坐标
    * @return null
    * @note 计算的坐标原点为图像几何中心，建立笛卡尔坐标系
    */
    void writeBytes(const cv::Rect2d& armourBlock, const cv::Mat& resizeFrame,
                          const bool& findArmourBlock);

    int receiveFlag; //! 接收标志，如果收到连续0则为0（红色），如果收到连续1则为1（蓝色）

private:
    short HEAD; //! equal to 00111111 00111111
    short TAIL; //! equal to 00011111 00011111

private slots:
    /**
    * @brief 回调函数。一旦串口接收到数据，则触发该函数，读取数据，并执行一定操作。
    * @note  具体功能待实现。
    */
    void readBytes();
};
//! @}
}
#endif // SERIAL_H
