#include "tool.h"

namespace HCVC
{
int g_trackBarLocation = 0;

void Tool::addTrackBar(const string& windowName, VideoCapture& file)
{
    double count = file.get(CAP_PROP_FRAME_COUNT);
    createTrackbar("trackBar", windowName, &g_trackBarLocation, static_cast<int>(count), onTrackBarCallback, &file);
}

void Tool::setTrackBarFollow(const string& windowName, const VideoCapture& file)
{
    double cur = file.get(CAP_PROP_POS_FRAMES);
    setTrackbarPos("trackBar", windowName, static_cast<int>(cur));
}

void Tool::addKeyboardControl(VideoCapture& srcFile, const int& delay)
{
    //记录当前暂停帧
    static double recordFrame = -1;
    //相当于设定播放帧率为每秒1000/33=30帧
    switch(waitKey(delay))
    {
    //如果读取到esc键终止播放
    case ESC:
        srcFile.release();
        break;

    //如果读取到空格键暂停播放，直到再次按下空格键，期间可以按下esc键退出
    case PAUSE:
        if(recordFrame < 0)
        {
            recordFrame = srcFile.get(CAP_PROP_POS_FRAMES);
        }
        else
        {
            recordFrame = -1;
        }
        break;

    //j键为视频回放
    case MOVE_BACK:
        if(recordFrame < 0)
        {
            double cur = srcFile.get(CAP_PROP_POS_FRAMES);
            srcFile.set(CAP_PROP_POS_FRAMES, cur-1);
        }
        else
        {
            recordFrame--;
            srcFile.set(CAP_PROP_POS_FRAMES, recordFrame);
        }
        break;

    //k键为视频快进
    case MOVE_FORWARD:
        if(recordFrame < 0)
        {
            double cur = srcFile.get(CAP_PROP_POS_FRAMES);
            srcFile.set(CAP_PROP_POS_FRAMES, cur+1);
        }
        else
        {
            recordFrame++;
            srcFile.set(CAP_PROP_POS_FRAMES, recordFrame);
        }
        break;

    default:
        if(recordFrame >= 0)
        {
            srcFile.set(CAP_PROP_POS_FRAMES, recordFrame);
        }
    }
}

void Tool::setTimeCount(const int& id, const int& tag, const string& timeCountName)
{
    static long long startTimes[100][2];

    startTimes[id][tag] = getTickCount();

    if(tag == END)
    {
        cout << "id - " << id << " - " << "name: " << timeCountName << " Cost time: "
             << (startTimes[id][END]-startTimes[id][BEGIN]) / getTickFrequency()
             << " s" << endl;
    }
}

void Tool::drawPoints(Mat resizeFrame, vector<Point>& points, Rect2d armourBlock)
{
    points.clear();
    points.push_back(Point2d(armourBlock.x +armourBlock.width/2,
                             armourBlock.y + armourBlock.height/2));

    for(unsigned int i = 0; i < points.size(); i++)
    {
        circle(resizeFrame, points[i], 3, Scalar(0, 0, 255));
    }
}

void Tool::drawCoord(Mat resizeFrame)
{
    line(resizeFrame, Point(0, resizeFrame.rows/2),
         Point(resizeFrame.cols, resizeFrame.rows/2), Scalar(0, 255, 0));
    line(resizeFrame, Point(resizeFrame.cols/2, 0),
         Point(resizeFrame.cols/2, resizeFrame.rows), Scalar(0, 255, 0));
}

void Tool::showPoints(Mat resizeFrame, short coord, int org_x, int org_y)
{
    //短型转整型
    const string text = toString(coord);

    //坐标在屏幕上显示位置
    Point origin(org_x, org_y);

    //坐标字体
    int font_face = FONT_HERSHEY_COMPLEX;

    //显示坐标
    putText(resizeFrame, text, origin, font_face, 1, Scalar(0, 255, 0), 2);
}

void Tool::drawBlocks(Mat &srcImage,
                      const vector<RotatedRect>& minRotatedRects,
                      const Scalar& color)
{
    for(unsigned int i = 0; i < minRotatedRects.size(); i++)
    {
        Point2f points[4];
        minRotatedRects[i].points(points);

        for(unsigned int j = 0; j < 4; j++)
        {
            line(srcImage, points[j], points[(j+1)%4], color, 2);
        }
    }
}

void Tool::drawBlocks(Mat &srcImage,
                      const RotatedRect* minRotatedRects,
                      int armoursNum,
                      const Scalar& color)
{
    for(int i = 0; i < armoursNum; i++)
    {
        Point2f points[4];
        minRotatedRects[i].points(points);

        for(unsigned int j = 0; j < 4; j++)
        {
            line(srcImage, points[j], points[(j+1)%4], color, 2);
        }
    }
}

void Tool::drawBlocks(Mat &srcImage,
                      const RotatedRect minRotatedRects,
                      const Scalar& color)
{
    Point2f points[4];
    minRotatedRects.points(points);

    for(unsigned int j = 0; j < 4; j++)
    {
        line(srcImage, points[j], points[(j+1)%4], color, 2);
    }
}

void Tool::onTrackBarCallback(int pos, void* file)
{
    static_cast<VideoCapture*>(file)->set(CAP_PROP_POS_FRAMES, pos);
}

string Tool::toString(short num)
{
    string result = "";
    char type = '+';
    if(num < 0)
    {
        type = '-';
        num = -num;
    }

    while(num)
    {
        result.insert(result.begin(), num%10 + '0');
        num/=10;
    }

    result.insert(result.begin(), type);

    return result;
}

}
