#include "image.h"

namespace HCVC {
Image::Image()
{

}

bool Image::init(Color color, string xmlPath)
{
    //初始化阈值列表
    FileStorage fs(xmlPath, FileStorage::READ);
    if(!fs.isOpened())
    {
        cout << "Open " << xmlPath << " file failed" << endl;
        return false;
    }
    FileNode nodeR = fs["red_image_preprocessor_threshod"],
             nodeB = fs["blue_image_preprocessor_threshod"];

    this->color = color;

    for(int i = 0; i < 3; i++)
    {
        thresholdsR[i][0] = int(nodeR[2*i]);
        thresholdsR[i][1] = int(nodeR[2*i+1]);
        thresholdsB[i][0] = int(nodeB[2*i]);
        thresholdsB[i][1] = int(nodeB[2*i+1]);
    }

    if(color == RED)
    {
        thresholds = thresholdsR;
    }
    else
    {
        thresholds = thresholdsB;
    }

    return true;
}

Mat Image::preprocess(const Mat& srcImage)
{
    Mat dstImage;

    //将bgr格式(opencv默认将彩色图片存储为bgr格式)图像转变为hsv格式
    cvtColor(srcImage, dstImage, COLOR_BGR2HSV);
    //分离图像三通道
    Mat hsvImages[3];
    split(dstImage, hsvImages);

    hsvImages[0] = rangeThreshold(hsvImages[0], 0);
    hsvImages[1] = rangeThreshold(hsvImages[1], 1);
    hsvImages[2] = rangeThreshold(hsvImages[2], 2);

    Mat hue, saturation, value;

    //中值滤波，去除S通道噪声点
    medianBlur(hsvImages[0], hue, 3);
    medianBlur(hsvImages[1], saturation,3);
    medianBlur(hsvImages[2], value, 1);

    detectValue = value;

    //闭操作，去除H通道噪声点，以水平方向为主膨胀H,S通道像素
    Mat kernel_1 = getStructuringElement(MORPH_RECT, Size(3, 1));
    Mat kernel_2 = getStructuringElement(MORPH_RECT, Size(5, 2));
    erode(hue, hue, kernel_1);
    dilate(hue, hue, kernel_2);
    dilate(saturation, saturation, kernel_1);
    //dilate(value, value, kernel_1);

    //初始化二值化图
    Mat framethreshold = Mat(value.size(), CV_8UC1, Scalar(0));

    //根据三个通道绘制二值化图
    threshProcess(srcImage, framethreshold, hue, saturation, value, hsvImages[2]);

    //中值滤波去除噪声点，同时使灯柱边缘润滑
    medianBlur(framethreshold, framethreshold, 3);

    //    //水平,竖直方向连接一些连接的团块，防止运动模糊产生重影
    //    Mat kernel_3 = getStructuringElement(MORPH_RECT, Size(4,1));
    //    morphologyEx(framethreshold, framethreshold, MORPH_CLOSE, kernel_3);

#ifdef DEBUG
    //显示单通道处理后图像
    imshow("hImage", hue);
    imshow("SImage", saturation);
    imshow("VImage", value);

    //显示预处理后图像
    imshow("result", framethreshold);
#endif

    return framethreshold;
}

void Image::threshProcess(const Mat& srcImage,
                          Mat& framethreshold,
                          Mat& hue,
                          Mat& saturation,
                          Mat& value,
                          Mat& orgValue)
{
    //查找轮廓，只检索最外面的轮廓，将所有的连码点转换成点
    vector<vector<Point> > contours;//定义一个返回轮廓的容器
    findContours(value, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    if(contours.size() > 1)
    {
        //轮廓的最小外接矩形
        Rect *boundRect = new Rect[contours.size()];
        RotatedRect *rotatedRect = new RotatedRect[contours.size()];

        for(unsigned int a = 0; a < contours.size(); ++a)
        {
            rotatedRect[a] = minAreaRect(contours[a]);
            boundRect[a] = rotatedRect[a].boundingRect2f();

            float longEdge = max(rotatedRect[a].size.height, rotatedRect[a].size.width);
            float shortEdge = min(rotatedRect[a].size.height, rotatedRect[a].size.width);

            if(longEdge < 18*shortEdge)
            {
                Point2f p[4];
                rotatedRect[a].points(p);

                double distance1 = pow((p[0].x - p[1].x), 2) + pow((p[0].y - p[1].y), 2),
                       distance2 = pow((p[0].x - p[3].x), 2) + pow((p[0].y - p[3].y), 2);

                if((fabs(rotatedRect[a].angle) < 30 && distance1 > distance2)
                        || (fabs(rotatedRect[a].angle) > 60 && distance1 < distance2))
                {
                    //计算团块周边红绿特征
                    double contoursArea = fabs(contourArea(contours[a], false));
                    unsigned int redAvg = 0, blueAvg = 0;
                    double hueContourPixels = 0;

                    if(2*contoursArea > double(rotatedRect[a].size.area()))
                    {
                        Point point;
                        for(unsigned int b = 0; b < contours[a].size(); ++b)
                        {
                            point = contours[a][b];
                            redAvg += *(srcImage.ptr<uchar>(point.y) + point.x*srcImage.channels() + 2);
                            blueAvg += *(srcImage.ptr<uchar>(point.y) + point.x*srcImage.channels());

                            if(*(hue.ptr<uchar>(point.y) + point.x) | 0x00)
                                hueContourPixels++;
                        }

                        redAvg /= contours[a].size();
                        blueAvg /= contours[a].size();

                        //cout << "redAvg:" << redAvg << "\t" << "blueAvg:" << blueAvg << endl;
                        //cout<<"hueContourPixels:" << hueContourPixels/contours[a].size() << endl;

                        if(color == RED)
                        {
                            if(redAvg > blueAvg)
                            {
                                goto EXECUTION;
                            }
                            else
                            {
                                continue;
                            }
                        }
                        else
                        {
                            if(redAvg < blueAvg)
                            {
                                goto EXECUTION;
                            }
                            else
                            {
                                continue;
                            }
                        }

EXECUTION:
                        unsigned int huePixel = 0;
                        unsigned int saturationPixel = 0;
                        int left = boundRect[a].x - 4 ,
                            right = boundRect[a].x + boundRect[a].width + 8,
                            top = boundRect[a].y - 4,
                            bottom = boundRect[a].y + boundRect[a].height + 8;

                        //检测亮度通道团块在H与S通道周围像素情况，存在所设定阈值像素则判定为灯柱
                        if(left < 0) left = 0;
                        if(right > srcImage.cols) right = srcImage.cols;
                        if(top < 0) top = 0;
                        if(bottom > srcImage.rows) bottom = srcImage.rows;

                        for(int i = top; i < bottom; ++i)
                        {
                            uchar* hueData = hue.ptr<uchar>(i);
                            uchar* saturationData = saturation.ptr<uchar>(i);
                            for(int j = left; j < right; ++j)
                            {
                                if(*(hueData + j) | 0x00)
                                    ++huePixel;
                                if(*(saturationData + j) | 0x00)
                                    ++saturationPixel;
                                if(huePixel)
                                    goto BREAK;
                            }
                        }

BREAK:
                        //根据亮度图团块进行二值图的绘制
                        if(saturationPixel > 0)
                        {
                            if(contoursArea > 4)
                            {
                                for (int i = top; i < bottom; ++i)
                                {
                                    uchar* valueData = orgValue.ptr<uchar>(i);
                                    uchar* framethresholdData = framethreshold.ptr<uchar>(i);
                                    for (int j = left; j < right; ++j)
                                    {
                                        if (*(valueData + j) > 127)
                                            *(framethresholdData + j) |= 0xFF;
                                    }
                                }
                            }
                        }
                        else if(hueContourPixels > 0.03*contours[a].size()
                                && contoursArea > 4
                                && double(longEdge) > 1.5*double(shortEdge))
                        {
                            for (int i = top; i < bottom; ++i)
                            {
                                uchar* valueData = orgValue.ptr<uchar>(i);
                                uchar* framethresholdData = framethreshold.ptr<uchar>(i);
                                for (int j = left; j < right; ++j)
                                {
                                    if (*(valueData + j) > 127)
                                        *(framethresholdData + j) |= 0xFF;
                                }
                            }
                        }
                    }
                }
            }
        }
        delete []boundRect;
        delete []rotatedRect;
    }
}

void Image::setThreshod(int channel, int minOrMax, int value)
{
    thresholds[channel][minOrMax] = value;
}

int Image::getThreshod(int channel, int minOrMax) const
{
    return thresholds[channel][minOrMax];
}

void Image::setColor(Image::Color color)
{
    this->color = color;

    if(color == RED && **thresholds != **thresholdsR)
    {
        thresholds = thresholdsR;
    }
    if(color == BLUE && **thresholds != **thresholdsB)
    {
        thresholds = thresholdsB;
    }
}

string Image::getColor()
{
    if(color == RED)
    {
        return "red";
    }
    else if(color == BLUE)
    {
        return "blue";
    }
    else
    {
        return "no color gave";
    }

}

Mat Image::rangeThreshold(const Mat& srcImage, const int& channel)
{
    Mat result;

    inRange(srcImage, thresholds[channel][0], thresholds[channel][1], result);

    return result;
}
}
