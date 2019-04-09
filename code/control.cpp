#include "control.h"

namespace HCVC
{
Control::Control()
{
    status = DETECTING;

    //定义xml文件和video文件的路径
#if defined(Q_OS_WIN)
    string xmlPath = "F:/QT Projects/JUST_HLL_Robomaster_CV/statics/params.xml",
           videoPath = "F:/Robomaster/HCVC/视觉素材/视觉素材/炮台素材红车旋转-ev-0.MOV",
           cameraXmlPath = "F:/QT Projects/JUST_HLL_Robomaster_CV/statics/cameraParams.xml";
    QString serialPort = "COM12";
#elif defined(Q_OS_LINUX)
    string xmlPath = "/home/teliute/JUST_HLL_Robomaster_CV/statics/params.xml",
           videoPath = "/home/teliute/video/炮台素材红车旋转-ev-0.MOV",
           cameraXmlPath = "/home/teliute/JUST_HLL_Robomaster_CV/statics/cameraParams.xml";
    QString serialPort = "ttyUSB0";
#endif

    Image::Color color = Image::RED;

    //初始化视频
    if(video.init(videoPath))
    {
        qDebug() << "Video init sucess!" << endl;
    }

    //初始化摄像头
    if(camera.init(0, xmlPath))
    {
        qDebug() << "Camera init sucess!" << endl;
    }

    //初始化串口
    if(serial.init(serialPort))
    {
        qDebug() << "SerialPort init sucess!" << endl;
    }

    //初始化预处理模块
    if(armourDetector.init(color, xmlPath))
    {
        qDebug() << "Color init sucess!" << endl;
    }

    //初始化检测模块
    if(armourDetector.init(xmlPath))
    {
        qDebug() << "ArmourDetector init sucess!" << endl;
    }

    //初始化测距模块
    if(ranging.init(cameraXmlPath))
    {
        qDebug() << "Ranging init success!" << endl;
    }
}

void Control::run()
{
    //创建原图像显示窗口
    namedWindow("srcFile", WINDOW_AUTOSIZE);

    //vector<Point> points;

    //添加滑动条
    //Tool::addTrackBar("srcFile", video.getVideo());

    //视频图像缓存区域
    Mat frame;

    //帧计数
    int count = 0;

    //预测暂存框
    Rect2d predictBlock;

    //填补帧
    bool frequency[5] = {0};

    //卡尔曼滤波初始化
    prediction.init();

    ////获取当地时间
//    time_t timep;
//    std::time(&timep);
//    char tmp[64];
//    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));

//    Size size = Size(1280, 720);
//    VideoWriter writer(string("/home/teliute/video/") + tmp + ".avi", CV_FOURCC('M', 'P', '4', '2'), 30, size);

    while(true)
    {
        //添加运行时间统计
        Tool::setTimeCount(1, Tool::BEGIN, "total time");

        if(serial.receiveFlag == 0)
        {
            armourDetector.setColor(Image::RED);
        }
        if(serial.receiveFlag == 1)
        {
            armourDetector.setColor(Image::BLUE);
        }

        cout << "Detect color is: " << armourDetector.getColor() << endl;

        //添加键盘控制
        Tool::addKeyboardControl(video.getVideo());
        Tool::addKeyboardControl(camera.getCamera());

        //添加滑动条跟随
        //Tool::setTrackBarFollow("srcFile", video.getVideo());
        //waitKey(1);

        //读取一帧图像
        camera >> frame;
        //writer << frame;

        //视频播放完毕跳出程序
        if(frame.empty())
        {
            break;
        }

        resize(frame, frame, Size(960, 540));

        //检测到的装甲区域
        Rect2d armourBlock;
        bool findArmourBlock = false;
        //double distance = 0;

        //检测图片中的灯柱位置
        if(status == DETECTING && armourDetector.detect(frame))
        {
            armourBlock = armourDetector.getBestArmourBlock().boundingRect2f();
            //distance = ranging.calDistance(armourDetector.getBestArmourBlock());
            armourTracker.init(frame, armourBlock);
            status = TRACKING;
            findArmourBlock = true;
        }

        // 追踪装甲板区域
        if(status == TRACKING)
        {
            if(armourTracker.track(frame))
            {
                armourBlock = armourTracker.getArmourBlock();
                findArmourBlock = true;
            }
            else
            {
                status = DETECTING;
            }
        }

        //进行检测与跟踪的装甲板填补
        prediction.fre_fillArmourBlock(frame, frequency, sizeof(frequency)/sizeof(frequency[0]),
                                       count, predictBlock, armourBlock, findArmourBlock);

        //在输出图像中画出装甲板中心轨迹
        //Tool::drawPoints(frame, points);

        //在输出图像中画出坐标系
        Tool::drawCoord(frame);

        //画出追踪的区域
        rectangle(frame, armourBlock, Scalar(255, 0, 0), 2, 1);

        //向串口写入相对坐标
        serial.writeBytes(armourBlock, frame, findArmourBlock);

        //显示原图像(重调大小后)
        imshow("srcFile", frame);

        //添加运行时间统计
        Tool::setTimeCount(1, Tool::END, "total time");
    }

    destroyAllWindows();
}
}

