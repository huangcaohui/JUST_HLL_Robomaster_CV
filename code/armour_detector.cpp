#include "armour_detector.h"

namespace HCVC
{
ArmourDetector::ArmourDetector()
{

}

bool ArmourDetector::init(string xmlPath)
{
    FileStorage fs(xmlPath, FileStorage::READ);
    if(!fs.isOpened())
    {
        cout << "Open file failed" << endl;
        return false;
    }

    FileNode node = fs["armour_detector"];

    node["angleRange"] >> params.angleRange;
    node["minArea"] >> params.minArea;
    node["maxHeightWidthRat"] >> params.maxHeightWidthRat;
    node["minHeightWidthRat"] >> params.minHeightWidthRat;

    node["tanAngle"] >> params.tanAngle;
    node["deviationAngle"] >> params.deviationAngle;
    node["armourPixelAvg"] >> params.armourPixelAvg;

    fs.release();

    return true;
}

bool ArmourDetector::detect(const Mat& srcImage)
{
    Mat dstImage = preprocess(srcImage);
    Mat value = detectValue;

    //存储初步找到的团块
    vector<vector<Point> > blocks = searchBlocks(dstImage.clone());

    //检验数，配合数组，检测数组的实际长度
    int lampsNum = 0, armoursNum = 0;

    //存储找到的所有灯柱块
    vector<RotatedRect> lampBlocks = calcBlocksInfo(blocks, lampsNum);

    //将vector存储到数组中
    if(lampsNum < 2)
    {
        return false;
    }

    RotatedRect *lamps = new RotatedRect[lampsNum];
    for(unsigned i = 0; i < lampBlocks.size(); ++i)
        lamps[i] = lampBlocks[i];

    //为中间结果显示准备图像
    Mat drawImage = srcImage;

    //存储筛选过符合条件的所有对灯柱对最小包围矩形即装甲板区域
    double *directAngle = new double[lampsNum*(lampsNum - 1)/2];
    RotatedRect *armourBlocks = new RotatedRect[lampsNum*(lampsNum - 1)/2];
    extracArmourBlocks(armourBlocks, lamps, dstImage, value, lampsNum, directAngle, armoursNum);

    if(!armoursNum)
    {
        return false;
    }

    //对每个装甲板区域评分

    markArmourBlocks(srcImage, dstImage, armourBlocks, directAngle, armoursNum);

#ifdef DEBUG
//    Tool::drawBlocks(drawImage,
//                     vector<RotatedRect>(1, optimalArmourBlocks.front().block),
//                     Scalar(255, 255, 0));
//    imshow("detect",drawImage);
#endif

    delete []lamps; lamps = nullptr;
    delete []directAngle; directAngle = nullptr;
    delete []armourBlocks; armourBlocks = nullptr;

    return true;
}

Rect2d ArmourDetector::getBestArmourBlock() const
{
    return optimalArmourBlocks.front().block.boundingRect();
}

void ArmourDetector::fillLampBlock(Mat& srcImage,
                                   vector<vector<Point> >& blocks,
                                   int row,
                                   int col)
{
    //如果这个像素越界或者为零则不需要进一步访问它的相邻像素
    if(row < 0 || row >= srcImage.rows
               || col < 0 || col >= srcImage.cols
               || srcImage.at<uchar>(row, col) == 0)
    {
        return ;
    }

    //向当前正在填充的团块中加入数据，
    //注意点的坐标是（x，y），指针访问的坐标顺序是（y，x），x为横轴，y为纵轴

    blocks.back().push_back(Point(col, row));

    //避免已访问像素的重复访问，将其置零
    srcImage.at<uchar>(row, col) = 0;

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            //避免重复访问自身
            if(x == 0 && y == 0)
            {
                continue;
            }

            fillLampBlock(srcImage, blocks, row + x, col + y);
        }
    }
}

vector<vector<Point> > ArmourDetector::searchBlocks(Mat srcImage)
{
    vector<vector<Point> > blocks;

    int rowNum = srcImage.rows;
    int colNum = srcImage.cols;

    //如果图像在内存空间中存储是连续的，则把图像当作一行连续的矩阵来访问，提高访问效率
    if (srcImage.isContinuous())
    {
        rowNum = 1;
        colNum = colNum * srcImage.rows * srcImage.channels();
    }

    for (int row = 0; row < rowNum; ++row)
    {
        uchar* srcImagePtr = srcImage.ptr<uchar>(row);

        for (int col = 0; col < colNum; ++col)
        {
            //按行优先的顺序访问图像矩阵后，找到的每一个非零数据一定是一个新的独立团块的起点
            if(*srcImagePtr++)
            {
                //根据找到的起点，递归遍历所有它的相邻像素
                blocks.push_back(vector<Point>());

                //由于存在两种不同的访问方式，需要进行坐标转换
                if(row == 0)
                {
                    fillLampBlock(srcImage, blocks, col/srcImage.cols, col%srcImage.cols);
                }
                else
                {
                    fillLampBlock(srcImage, blocks, row, col);
                }
            }
        }
    }

    return blocks;
}

vector<RotatedRect> ArmourDetector::calcBlocksInfo(const vector<vector<Point> >& blocks,
                                                   int& lampsNum)
{
    vector<RotatedRect> lampBlocks;

    if(blocks.size() != 0)
    {
        for(unsigned int i = 0; i < blocks.size(); ++i)
        {
            RotatedRect minRotatedRect = minAreaRect(blocks[i]);

            if(minRotatedRect.size.area() > params.minArea)
            {
                if(minRotatedRect.angle > -45)
                {
                    if(minRotatedRect.size.height >
                            minRotatedRect.size.width*params.minHeightWidthRat)
                    {
                        lampBlocks.push_back(minRotatedRect);
                        ++lampsNum;
                    }
                }
                else
                {
                    if(minRotatedRect.size.width >
                            minRotatedRect.size.height*params.minHeightWidthRat)
                    {
                        lampBlocks.push_back(minRotatedRect);
                        ++lampsNum;
                    }
                }
            }
        }
    }

    return lampBlocks;
}

void ArmourDetector::extracArmourBlocks(RotatedRect* armourBlocks,
                                        const RotatedRect* lampBlocks,
                                        const Mat dstImage,
                                        const Mat value,
                                        const int lampsNum,
                                        double* directAngle,
                                        int& armoursNum)
{
    int screenNum = 0;
    int pairNum = 0;
    double angle = 0;
    int maxPair = lampsNum*(lampsNum - 1)/2;
    Point *angleI = new Point[maxPair], *angleJ = new Point[maxPair];
    RotatedRect *screenLamps = new RotatedRect[lampsNum];
    RotatedRect (*pairLamps)[2] = new RotatedRect[maxPair][2];
    RotatedRect initLightBlocks[2];
    RotatedRect initArmourBlock;

    bool *sequence = new bool[lampsNum];
    for(int i = 0; i < lampsNum; ++i)
    {
        sequence[i] = false;
        directAngle[i] = 0;
    }

    //通过夹角筛选装甲板
    for(int i = 0; i < lampsNum - 1; ++i)
    {
        Point vecI = calVectorY(lampBlocks[i]);

        for(int j = i + 1; j < lampsNum; ++j)
        {
            if(abs(lampBlocks[i].center.y - lampBlocks[j].center.y) <
                    params.tanAngle*abs(lampBlocks[i].center.x - lampBlocks[j].center.x))
            {
                //计算灯柱的与y轴夹角最小的方向向量
                Point vecJ = calVectorY(lampBlocks[j]);

                //两向量的夹角
                angle = acos((vecI.x*vecJ.x + vecI.y*vecJ.y)/
                             (sqrt(pow(vecI.x, 2) + pow(vecI.y, 2)) *
                              sqrt(pow(vecJ.x, 2) + pow(vecJ.y, 2))));

                if(angle*180 < double(params.angleRange)*CV_PI)
                {

                    if((double(lampBlocks[i].size.area()) > 0.2*double(lampBlocks[j].size.area()))
                            &&(double(lampBlocks[j].size.area()) > 0.2*double(lampBlocks[i].size.area())))
                    {                     
                        //获取所有不重复的外接矩形
                        if(!sequence[i])
                        {
                            screenLamps[screenNum] = lampBlocks[i];
                            sequence[i] = true;
                            ++screenNum;
                        }

                        if(!sequence[j])
                        {
                            screenLamps[screenNum] = lampBlocks[j];
                            sequence[j] = true;
                            ++screenNum;
                        }

                        //获取匹配对数，储存夹角
                        angleI[pairNum] = vecI;
                        angleJ[pairNum] = vecJ;
                        directAngle[pairNum] = angle;
                        pairLamps[pairNum][0] = lampBlocks[i];
                        pairLamps[pairNum][1] = lampBlocks[j];
                        ++pairNum;
                    }
                }
            }
        }
    }


    for(int i = 0; i < pairNum; ++i)
    {
        unsigned int labelValue = 0;

        initLightBlocks[0] = pairLamps[i][0];
        initLightBlocks[1] = pairLamps[i][1];

        //外接正矩形连通域数量检测
        domainCountDetect(initLightBlocks, screenLamps, dstImage,
                          value, labelValue, screenNum);

        if(labelValue == 2)
        {
            initArmourBlock = getArmourRotated(initLightBlocks, 2);

            Point vecArmourY = calVectorY(initArmourBlock);
            Point vecArmourX = calVectorX(initArmourBlock);
            double length = sqrt(pow(vecArmourX.x, 2) + pow(vecArmourX.y, 2));

            //计算灯柱相对于装甲板的垂直偏向角
            double deviationAngleI = acos((angleI[i].x*vecArmourY.x
                                        + angleI[i].y*vecArmourY.y)/
                                    (sqrt(pow(angleI[i].x, 2) + pow(angleI[i].y, 2)) *
                                     sqrt(pow(vecArmourY.x, 2) + pow(vecArmourY.y, 2))));

            double deviationAngleJ = acos((angleJ[i].x*vecArmourY.x
                                        + angleJ[i].y*vecArmourY.y)/
                                    (sqrt(pow(angleJ[i].x, 2) + pow(angleJ[i].y, 2)) *
                                     sqrt(pow(vecArmourY.x, 2) + pow(vecArmourY.y, 2))));

            //弧度制转角度制，计算装甲板偏向角
            deviationAngleI *= 180/CV_PI;
            deviationAngleJ *= 180/CV_PI;

            deviationAngleI = min(deviationAngleI, abs(90 - deviationAngleI));
            deviationAngleJ = min(deviationAngleJ, abs(90 - deviationAngleJ));

            int left = initArmourBlock.boundingRect().x,
                top = initArmourBlock.boundingRect().y,
                width = initArmourBlock.boundingRect().width,
                height = initArmourBlock.boundingRect().height;

            //两灯柱的最大短边
            float lightWidth = max(min(initLightBlocks[0].size.height,
                                       initLightBlocks[0].size.width),
                                   min(initLightBlocks[1].size.height,
                                       initLightBlocks[1].size.width));

            if(left > 0 && left + width < dstImage.cols &&
                    top > 0 && top + height < dstImage.rows)
            {
                if(4*double(lightWidth) < length
                   && deviationAngleI < double(params.deviationAngle)
                   && deviationAngleJ < double(params.deviationAngle))
                {
                    directAngle[armoursNum] = directAngle[i] + deviationAngleI + deviationAngleJ;
                    armourBlocks[armoursNum] = initArmourBlock;

                    ++armoursNum;
                }
            }
        }
    }

    delete []angleI; angleI = nullptr;
    delete []angleJ; angleJ = nullptr;
    delete []screenLamps; screenLamps = nullptr;
    delete []pairLamps; pairLamps = nullptr;
    delete []sequence; sequence = nullptr;
}

Point ArmourDetector::calVectorX(const RotatedRect rotated)
{
    Point2f corners[4];
    rotated.points(corners);

    //获取矩形的中心点的y坐标
    float centery = 0;
    for(unsigned int i = 0; i < 4; ++i)
    {
        centery += corners[i].y;
    }
    centery /= 4;

    //求出左右两组点的中点
    Point top[2], bottom[2];
    int numTop = 0, numBottom = 0;

    for(unsigned int i = 0; i < 4; ++i)
    {
        if(corners[i].y < centery)
        {
            top[numTop] = corners[i];
            numTop++;
        }
        else
        {
            bottom[numBottom] = corners[i];
            numBottom++;
        }
    }

    //求出底边中点为终点点的向量,方向沿x轴正方向
    Point vec, vecLeft, vecRight;

    if(top[0].x < top[1].x)
    {
        if(bottom[0].x < bottom[1].x)
        {
            vecLeft.x = (top[0].x + bottom[0].x)/2;
            vecLeft.y = (top[0].y + bottom[0].y)/2;
            vecRight.x = (top[1].x + bottom[1].x)/2;
            vecRight.y = (top[1].y + bottom[1].y)/2;
        }
        else
        {
            vecLeft.x = (top[0].x + bottom[1].x)/2;
            vecLeft.y = (top[0].y + bottom[1].y)/2;
            vecRight.x = (top[1].x + bottom[0].x)/2;
            vecRight.y = (top[1].y + bottom[0].y)/2;
        }
    }
    else
    {
        if(bottom[0].x < bottom[1].x)
        {
            vecLeft.x = (top[1].x + bottom[0].x)/2;
            vecLeft.y = (top[1].y + bottom[0].y)/2;
            vecRight.x = (top[0].x + bottom[1].x)/2;
            vecRight.y = (top[0].y + bottom[1].y)/2;
        }
        else
        {
            vecLeft.x = (top[1].x + bottom[1].x)/2;
            vecLeft.y = (top[1].y + bottom[1].y)/2;
            vecRight.x = (top[0].x + bottom[0].x)/2;
            vecRight.y = (top[0].y + bottom[0].y)/2;
        }
    }

    vec = Point2f(vecRight.x - vecLeft.x, vecRight.y - vecLeft.y);

    return vec;
}

Point ArmourDetector::calVectorY(const RotatedRect rotated)
{
    Point2f corners[4];
    rotated.points(corners);

    //获取矩形的中心点的y坐标
    float centery = 0;
    for(unsigned int i = 0; i < 4; ++i)
    {
        centery += corners[i].y;
    }
    centery /= 4;

    //求出上下两组点的中点
    float topx = 0, topy = 0, bottomx = 0, bottomy = 0;
    for(unsigned int i = 0; i < 4; ++i)
    {
        if(corners[i].y < centery)
        {
            topx += corners[i].x;
            topy += corners[i].y;
        }
        else
        {
            bottomx += corners[i].x;
            bottomy += corners[i].y;
        }
    }

    topx /= 2; topy /= 2;
    bottomx /= 2; bottomy /= 2;

    //求出底边中点为终点点的向量,方向沿y轴正方向
    Point2f vec(bottomx - topx, bottomy - topy);

    //cout<<"vec:"<<vec<<endl;

    return Point(vec);
}

void ArmourDetector::domainCountDetect(const RotatedRect* initLightBlocks,
                                       const RotatedRect* screenLamps,
                                       const Mat& dstImage,
                                       const Mat& value,
                                       unsigned int& labelValue,
                                       const int screenNum)
{
    Mat labelImg = dstImage.clone();

    float a1 = initLightBlocks[0].boundingRect2f().x,
          a2 = initLightBlocks[0].boundingRect2f().x
             + initLightBlocks[0].boundingRect2f().width,
          a3 = initLightBlocks[1].boundingRect2f().x,
          a4 = initLightBlocks[1].boundingRect2f().x
             + initLightBlocks[1].boundingRect2f().width;
    float b1 = initLightBlocks[0].boundingRect2f().y,
          b2 = initLightBlocks[0].boundingRect2f().y
             + initLightBlocks[0].boundingRect2f().height,
          b3 = initLightBlocks[1].boundingRect2f().y,
          b4 = initLightBlocks[1].boundingRect2f().y
             + initLightBlocks[1].boundingRect2f().height;

    int left = int(min(min(min(a1, a2), a3), a4));     //左边界
    int right = int(max(max(max(a1, a2), a3), a4));    //右边界
    int top = int(min(min(min(b1, b2), b3), b4));      //上边界
    int bottom = int(max(max(max(b1, b2), b3), b4));   //下边界
    int width = right - left, height = bottom - top;

//    Point seed, neighbor;
//    int rows = bottom;
//    int cols = right;
//    stack<Point> pointStack; // 堆栈

    //矫正边界
    correctBorder(left, top, width, height, dstImage);

    int doubleHeight = 2*height;
    if(top + doubleHeight >= dstImage.rows)
        doubleHeight = dstImage.rows - top - 1;

    Rect armourRect = Rect(Point(left, top), Point(left + width, top + doubleHeight));
    //通过压栈计算框定区域内连通域数量
    /*
    for (unsigned int i = top; i < rows; ++i)
    {
        uchar* data = labelImg.ptr<uchar>(i);//获取一行的点
        for (unsigned int j = left; j < cols; ++j)
        {
            if (data[j] == 255)
            {
                labelValue++; //不断将标签数加一
                seed = Point(j, i);// Point坐标
                labelImg.at<uchar>(seed) = labelValue;//标签
                pointStack.push(seed);//将像素seed压入栈，增加数据

                while (!pointStack.empty())//死循环，直到堆栈为空
                {
                    neighbor = Point(seed.x - 1, seed.y);//左像素
                    if ((seed.x != 0)
                            && (labelImg.at<uchar>(neighbor) == 255))
                    {
                        labelImg.at<uchar>(neighbor) = labelValue;
                        pointStack.push(neighbor);
                    }

                    neighbor = Point(seed.x + 1, seed.y);//右像素
                    if ((seed.x != (cols - 1))
                            && (labelImg.at<uchar>(neighbor) == 255))
                    {
                        labelImg.at<uchar>(neighbor) = labelValue;
                        pointStack.push(neighbor);
                    }

                    neighbor = Point(seed.x, seed.y - 1);//上像素
                    if ((seed.y != 0)
                            && (labelImg.at<uchar>(neighbor) == 255))
                    {
                        labelImg.at<uchar>(neighbor) = labelValue;
                        pointStack.push(neighbor);
                    }

                    neighbor = Point(seed.x, seed.y + 1);//下像素
                    if ((seed.y != (rows - 1))
                            && (labelImg.at<uchar>(neighbor) == 255))
                    {
                        labelImg.at<uchar>(neighbor) = labelValue;
                        pointStack.push(neighbor);
                    }
                    seed = pointStack.top();

                    //  获取堆栈上的顶部像素并将其标记为相同的标签
                    pointStack.pop();//弹出栈顶像素
                }
            }
        }
    }
    */

    //通过不重复连通域查找排除地面可能造成的灯柱倒影,并减小下面连通域检测的计算量
    for(int i = 0; i < screenNum; ++i)
    {
        Point p1 = armourRect.tl();
        Point p2 = armourRect.br();
        Point p3 = screenLamps[i].boundingRect().tl();
        Point p4 = screenLamps[i].boundingRect().br();
        if(p2.y > p3.y && p1.y < p4.y && p2.x > p3.x && p1.x < p4.x)
            ++labelValue;
    }

    //再一次进行连通域检测，排除灯柱附近的一些噪点与错误匹配
    if(labelValue == 2)
    {
        labelValue = 0;

        double lightArea = double(initLightBlocks[0].size.area());

        vector<vector<Point> > contours;
        Mat roi = value(Rect(left, top, width, height));
        findContours(roi, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

        for(unsigned int i = 0; i < contours.size(); ++i)
        {
            double contoursArea = fabs(contourArea(contours[i], false));

            //防止噪点产生干扰
            if(2*contoursArea > lightArea && 2*lightArea > contoursArea)
            {
                ++labelValue;
            }
        }
    }
}

RotatedRect ArmourDetector::getArmourRotated(RotatedRect* initLightBlocks, int lightsNum)
{
    RotatedRect initArmourBlock;
    vector<Point> armourPoints;

    //获取两团块的最小外接矩形
    for(int m = 0; m < lightsNum; ++m)
    {
        Point2f lightPoints[4];
        initLightBlocks[m].points(lightPoints);

        for(unsigned int n = 0; n < 4; ++n)
            armourPoints.push_back(lightPoints[n]);
    }

    initArmourBlock = minAreaRect(armourPoints);

    return initArmourBlock;
}

void ArmourDetector::calcDeviation(const RotatedRect armourReserve,
                                   const Mat& srcImage,
                                   const Mat& dstImage,
                                   double& armourPixelAvg,
                                   double& tanAngle,
                                   double& deviationAngle,
                                   double& armourStandard)
{
    Mat gray = Mat (srcImage.rows, srcImage.cols,CV_8UC1);
    cvtColor(srcImage,gray,COLOR_BGR2GRAY);

    Mat framethreshold=dstImage.clone();

    double sum=0;//像素值的总和
    double armourPixelCount = 0;//甲板像素数量
    double notArmourRangPixel = 0;//远离甲板平均值像素
    armourPixelAvg = 0;//像素的平均值
    tanAngle = 0;//区间范围内像素所占比例
    deviationAngle = 0;//区间外像素所占比例

    int left = armourReserve.boundingRect().x,
        top = armourReserve.boundingRect().y,
        width = armourReserve.boundingRect().width,
        height = armourReserve.boundingRect().height;

    double maxPixel = 0, minPixel = 255;
//    int total[256];
//    for(unsigned i = 0; i < 256; ++i)
//        total[i] = 0;

    //矫正边界
    correctBorder(left, top, width, height, dstImage);

    for(int i = top; i < top + height; ++i)
    {
        uchar* grayData = gray.ptr<uchar>(i);//灰度图像素
        uchar* framethresholdData = framethreshold.ptr<uchar>(i);//二值化图像素
        for (int j = left; j < left + width; ++j)
        {
            if (!(*(framethresholdData + j) | 0x00))//非灯条像素
            {
                sum += *(grayData + j);
                if(*(grayData + j) >= maxPixel)
                    maxPixel = *(grayData + j);
                if(*(grayData + j) <= minPixel)
                    minPixel = *(grayData + j);

                armourPixelCount++;
            }
        }
    }

     armourPixelAvg = (sum - armourPixelCount*minPixel)/armourPixelCount;

     //求甲板像素给定区间范围内像素值与标准差
     for (int i = top; i < top + height; ++i)
     {
         uchar* grayData = gray.ptr<uchar>(i);
         uchar* framethresholdData = framethreshold.ptr<uchar>(i);//二值化图像素
         for (int j = left; j < left + width; ++j)
         {
             if (*(framethresholdData + j) == 0)//非灯条像素
             {
                 armourStandard = sqrt(pow(abs((*(grayData + j) - minPixel)) - armourPixelAvg, 2)
                                 /armourPixelCount);
                 if (*(grayData + j) - minPixel > 2.5*armourPixelAvg)
                     ++notArmourRangPixel;
             }
         }
     }

     //armourPixelAvg = armourPixelAvg/(maxPixel - minPixel)*255;
     //tanAngle = armourRangPixel / armourPixelCount;
     deviationAngle = (notArmourRangPixel / armourPixelCount)*100;
}

void ArmourDetector::markArmourBlocks(const Mat& srcImage,
                                      const Mat& dstImage,
                                      const RotatedRect* armourBlocks,
                                      const double *directAngle,
                                      int armoursNum)
{
    //清除之前运算的结果
    optimalArmourBlocks.clear();

    //通过评分选出最优装甲板
    if(armoursNum > 2)
    {
        float armourArea[2];
        RotatedRect initArmour[2];
        double angle[2];

        //初始化
        armourArea[0] = armourBlocks[0].size.area();
        initArmour[0] = armourBlocks[0];
        angle[0] = directAngle[0];        
        armourArea[1] = 0;

        //剪去旋转矩形的多余边角，得到装甲板的平行四边形区域
        //cutEdgeOfRect(fpoints);

        for(int i = 1; i < armoursNum; ++i)
        {            
            if(armourBlocks[i].size.area() > armourArea[0])
            {
                armourArea[0] = armourBlocks[i].size.area();
                initArmour[0] = armourBlocks[i];
                angle[0] = directAngle[i];
            }
        }

        for(int i = 0; i < armoursNum; ++i)
        {
            if(armourBlocks[i].size.area() < armourArea[0]
                    && armourBlocks[i].size.area() > armourArea[1])
            {
                armourArea[1] = armourBlocks[i].size.area();
                initArmour[1] = armourBlocks[i];
                angle[1] = directAngle[i];
            }
        }

        //获取两个矩形的交集
        Rect overlap = initArmour[0].boundingRect() & initArmour[1].boundingRect();

        if(overlap.empty())
        {
            if(armourArea[0] > armourArea[1])
                optimalArmourBlocks.push_back(OptimalArmourBlock(initArmour[0], 1));
            else
                optimalArmourBlocks.push_back(OptimalArmourBlock(initArmour[1], 1));
        }
        else
        {
            for(unsigned int i = 0; i < 2; ++i)
            {
                double average = calAverage(srcImage, dstImage, armourBlocks[i]);

                double grade = -average*angle[i];

                optimalArmourBlocks.push_back(OptimalArmourBlock(initArmour[i], grade));
            }
        }
    }

    if(armoursNum == 2)
    {
        //int contoursArea = extractMask(armourBlocks, dstImage);

        double armourAreaI = double(armourBlocks[0].size.area());
        double armourAreaJ = double(armourBlocks[1].size.area());

        //获取两个矩形的交集
        Rect overlap = armourBlocks[0].boundingRect() & armourBlocks[1].boundingRect();

        //根据矩形交集判断两矩形是否属于一辆车或是否为噪点匹配
        if(overlap.empty())
        {
            if(armourAreaI > armourAreaJ)
                optimalArmourBlocks.push_back(OptimalArmourBlock(armourBlocks[0], armourAreaI));
            else
                optimalArmourBlocks.push_back(OptimalArmourBlock(armourBlocks[1], armourAreaJ));
        }
        else
        {
            for(unsigned int i = 0; i < 2; ++i)
            {
                double average = calAverage(srcImage, dstImage, armourBlocks[i]);

                double grade = -average*directAngle[i];

                optimalArmourBlocks.push_back(OptimalArmourBlock(armourBlocks[i], grade));
            }
        }
    }

    if(armoursNum == 1)
    {
        optimalArmourBlocks.push_back(OptimalArmourBlock(armourBlocks[0], 1));
    }

    //将装甲板区域按分从小到大排序，找出最佳区域
    sort(optimalArmourBlocks.begin(), optimalArmourBlocks.end());
}

double ArmourDetector::calAverage(const Mat srcImage, const Mat dstImage, RotatedRect armour)
{
    Point2f fpoints[4];
    armour.points(fpoints);

    //浮点数转换整数
    Point points[4];
    for(unsigned int i = 0; i < 4; ++i)
    {
        points[i] = Point(static_cast<int>(fpoints[i].x), static_cast<int>(fpoints[i].y));
    }

    const Point* pts = points;
    const int npts = 4;

    //创建掩码区域为包含装甲板的旋转矩形
    Mat mask(dstImage.size(), CV_8UC1, Scalar(0));
    //多边形填充
    fillConvexPoly(mask, pts, npts, Scalar(255));

    Mat dstNot;
    //原二值化图取反操作
    bitwise_not(dstImage, dstNot);
    //两二值化图取或操作
    bitwise_and(mask, dstNot, mask);

//    Mat ycrcb;
//    Mat roi = srcImage(Rect(getBestArmourBlock().tl(), getBestArmourBlock().br()));
//    cvtColor(roi, ycrcb, COLOR_BGR2YCrCb);
//    vector<Mat> channels;
//    split(ycrcb, channels);
//    equalizeHist(channels[0], channels[0]);
//    merge(channels, ycrcb);
//    cvtColor(ycrcb, roi, COLOR_YCrCb2BGR);

    Scalar average, std;

    meanStdDev(srcImage, average, std, mask);

    return average[0];
}

int ArmourDetector::extractMask(const RotatedRect* armourBlocks, Mat dstImage)
{
    Point2f fpoints[4];
    armourBlocks[0].points(fpoints);

    //浮点数转换整数
    Point points[4];
    for(unsigned int i = 0; i < 4; ++i)
    {
        points[i] = Point(static_cast<int>(fpoints[i].x), static_cast<int>(fpoints[i].y));
    }

    const Point* ptsI = points;
    const int npts = 4;

    //创建掩码区域为包含装甲板的旋转矩形
    Mat maskI(dstImage.size(), CV_8UC1, Scalar(0));
    //多边形填充
    fillConvexPoly(maskI, ptsI, npts, Scalar(255));

    armourBlocks[1].points(fpoints);

    for(unsigned int i = 0; i < 4; ++i)
    {
        points[i] = Point(static_cast<int>(fpoints[i].x), static_cast<int>(fpoints[i].y));
    }

    const Point* ptsJ = points;

    //创建掩码区域为包含装甲板的旋转矩形
    Mat maskJ(dstImage.size(), CV_8UC1, Scalar(0));
    //多边形填充
    fillConvexPoly(maskJ, ptsJ, npts, Scalar(255));

    Mat mask;

    bitwise_or(maskI, maskJ, mask);

    vector<vector<Point> > contours;

    findContours(mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    int contoursArea = 0;

    for(unsigned int i = 0; i < contours.size(); ++i)
        contoursArea += contourArea(contours[i], false);

    return contoursArea;
}

void ArmourDetector::correctBorder(int& left, int& top, int& width, int& height, Mat image)
{
    int leftClone = left, topClone = top;

    if(left < 0){left = 0; width += leftClone;}
    if(left + width > image.cols){width = image.cols - leftClone;}
    if(top < 0){top = 0; height += topClone;}
    if(top + height > image.rows){height = image.rows - topClone;}
}

void ArmourDetector::cutEdgeOfRect(Point2f* points)
{
    //求出四个点的横坐标中点
    float centerx = 0;
    for(unsigned int i = 0; i < 4; ++i)
    {
        centerx += points[i].x;
    }
    centerx /= 4;

    //通过横坐标中点将这组点分为左右两对
    vector<Point2f> leftPoints;
    vector<Point2f> rightPoints;
    for(unsigned int i = 0; i < 4; ++i)
    {
        if(points[i].x < centerx)
        {
            leftPoints.push_back(points[i]);
        }
        else
        {
            rightPoints.push_back(points[i]);
        }
    }

    //组内分别按高度排序，方便之后处理
    if(leftPoints[0].y < leftPoints[1].y)
    {
        reverse(leftPoints.begin(), leftPoints.end());
    }

    if(rightPoints[0].y < rightPoints[1].y)
    {
        reverse(rightPoints.begin(), rightPoints.end());
    }

    //如果左边这对比右边高，则矩形为倒向左侧状态，否则为倒向右侧状态
    if(leftPoints[1].y > rightPoints[1].y)
    {
        Point2f newPoint;

        //两条直线相交的交点
        newPoint.x = leftPoints[0].x;
        newPoint.y = (leftPoints[1].y-rightPoints[1].y)/(leftPoints[1].x-rightPoints[1].x)
                * (leftPoints[0].x-rightPoints[1].x) + rightPoints[1].y;
        leftPoints[1] = newPoint;

        newPoint.x = rightPoints[1].x;
        newPoint.y = (leftPoints[0].y-rightPoints[0].y)/(leftPoints[0].x-rightPoints[0].x)
                * (rightPoints[1].x-leftPoints[0].x) + leftPoints[0].y;
        rightPoints[0] = newPoint;
    }
    else
    {
        Point2f newPoint;

        //两条直线相交的交点
        newPoint.x = leftPoints[1].x;
        newPoint.y = (leftPoints[0].y-rightPoints[0].y)/(leftPoints[0].x-rightPoints[0].x)
                * (leftPoints[1].x-rightPoints[0].x) + rightPoints[0].y;
        leftPoints[0] = newPoint;

        newPoint.x = rightPoints[0].x;
        newPoint.y = (leftPoints[1].y-rightPoints[1].y)/(leftPoints[1].x-rightPoints[1].x)
                * (rightPoints[0].x-leftPoints[1].x) + leftPoints[1].y;
        rightPoints[1] = newPoint;
    }

    //拼接两对点
    points[0] = leftPoints[0];
    points[1] = leftPoints[1];
    points[2] = rightPoints[1];
    points[3] = rightPoints[0];
}
}

