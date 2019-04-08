#include "serial.h"

namespace HCVC
{
Serial::Serial(): receiveFlag(2), HEAD(0x3f3f), TAIL(0x1f1f)
{

}

bool Serial::init(QString portName)
{
    //串口信息类，用于读取可用串口所有的信息
    QSerialPortInfo serialPortInfo;
    foreach(serialPortInfo, QSerialPortInfo::availablePorts())
    {
        if(serialPortInfo.portName() == portName )
        {
            setPort(serialPortInfo);
            break;
        }
    }

    //输出所用串口的相关信息
    qDebug() << serialPortInfo.portName() << endl
             << serialPortInfo.description() << endl;
    //<< serialPortInfo.serialNumber() << endl;

    if(open(QIODevice::ReadWrite))
    {
        qDebug() << "open(QIODevice::ReadWrite)" << endl;
        setBaudRate(QSerialPort::Baud115200);
        setParity(QSerialPort::NoParity);
        setDataBits(QSerialPort::Data8);
        setStopBits(QSerialPort::OneStop);
        setFlowControl(QSerialPort::NoFlowControl);

        clearError();
        clear();
        //设定触发事件，如果串口有数据，则触发读取函数
        connect(this, SIGNAL(readyRead()), this, SLOT(readBytes()));

        return true;
    }

    return false;
}

void Serial::writeBytes(const cv::Rect2d& armourBlock, const cv::Mat& resizeFrame,
                        const bool& findArmourBlock)
{
    QByteArray buffer;
    int checksum = 4*0x2F2F;

    //如果没有检测到装甲板，则发送特殊字节串b 01111111 01111111 01111111 01111111
    if(findArmourBlock == false)
    {
        buffer.append(QByteArray(8, 0x2F));
        buffer.append(reinterpret_cast<char*>(&checksum), 2);
        write(buffer);

        return ;
    }
    short int xDiff, yDiff;
    //    short int midX = armourBlock.width/2, midY = armourBlock.height/2;

    //转换坐标，修改坐标原点
    //    xDiff = static_cast<short>(armourBlock.x + armourBlock.width/2 - resizeFrame.cols/2 - 40);
    //    yDiff = static_cast<short>(resizeFrame.rows/2 - (armourBlock.y + armourBlock.height/2));

    xDiff = static_cast<short>(armourBlock.x +armourBlock.width/2);
    yDiff = static_cast<short>(resizeFrame.rows - armourBlock.y - armourBlock.height/2);

    //视频显示适时坐标
    Tool::showPoints(resizeFrame, xDiff, 50, 50);
    Tool::showPoints(resizeFrame, yDiff, 170, 50);

    //xDiff = 0;
    //yDiff = 0;

    //待写入数据缓冲区
    checksum = HEAD+xDiff+yDiff+TAIL;

    //向缓冲区添加表示两个短整型数的四个字节
    buffer.append(reinterpret_cast<char*>(&HEAD), 2);
    buffer.append(reinterpret_cast<char*>(&xDiff), 2);
    buffer.append(reinterpret_cast<char*>(&yDiff), 2);
    buffer.append(reinterpret_cast<char*>(&TAIL), 2);
    buffer.append(reinterpret_cast<char*>(&checksum), 2);

    write(buffer);
    waitForBytesWritten(1);
}

void Serial::readBytes()
{
    QByteArray buffer;
    buffer = readAll().mid(0, 1);

    if(buffer.toHex() == "00")
    {
        receiveFlag = 0;
    }
    if(buffer.toHex() == "0f")
    {
        receiveFlag = 1;
    }

//    qDebug() << "********************************" << endl;
//    qDebug() << "receive data: " << buffer.toHex() << endl;
//    qDebug() << "********************************" << endl;

    //清除缓冲区
    buffer.clear();
}
}
