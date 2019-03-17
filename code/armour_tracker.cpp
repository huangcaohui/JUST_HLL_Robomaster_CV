#include "armour_tracker.h"

namespace HCVC
{
ArmourTracker::ArmourTracker()
{
    
}

void ArmourTracker::init(const Mat &srcImage, Rect2d armourBlock)
{    
    TrackerKCF::Params param;
    param.desc_pca = TrackerKCF::GRAY | TrackerKCF::CN| TrackerKCF::CUSTOM;
    param.desc_npca = 0;
    param.compress_feature = true;
    param.compressed_size = 2;
    param.resize = true;
    param.pca_learning_rate = 2;
    //param.split_coeff = true;

    tracker = TrackerKCF::create(param);

    tracker->init(srcImage, armourBlock);
}

bool ArmourTracker::track(Mat srcImage)
{
    //追踪目标区域
    if(tracker->update(srcImage, armourBlock) == false)
    {
       return false;
    }

    correctBorders(srcImage, armourBlock);

    //获取矫正后矩形框的Mat形式，方便之后处理
//    updateRoi = srcImage(Rect2d(armourBlock.x, armourBlock.y, armourBlock.width, armourBlock.height));

//    Rect2d minBoundRect;

//    if(3*armourBlock.area() > roi.cols*roi.rows)
//    {
//        //对矩形框进行矫正，获取其最小外接矩形
//        minBoundRect = refineRect(updateRoi, srcImage);
//    }

//    armourBlock = Rect2d(minBoundRect.x + armourBlock.x - armourBlock.height/4,
//                         minBoundRect.y + armourBlock.y - armourBlock.height/2,
//                         minBoundRect.width, minBoundRect.height);

//    correctBorders(srcImage, armourBlock);

    return true;
}

Rect2d ArmourTracker::refineRect(Mat& updateRoi, Mat& srcImage)
{
    //提取初始跟踪区域V通道二值化图
    Mat updateHSV;
    cvtColor(updateRoi, updateHSV, CV_BGR2HSV);
    Mat hsvImages[3];
    split(updateHSV, hsvImages);
    inRange(hsvImages[2], 200, 255, updateValue);
    medianBlur(updateValue, updateValue, 3);

    //提取扩大范围跟踪区域V通道二值化图

    Rect2d amplifyRect = Rect2d(armourBlock.x - armourBlock.height/4,
                                armourBlock.y - armourBlock.height/2,
                                armourBlock.width + armourBlock.height/2,
                                9*armourBlock.height/5);

    correctBorders(srcImage, amplifyRect);

    adjustRoi = srcImage(amplifyRect);

    Mat adjustHSV, hsvImage[3];
    cvtColor(adjustRoi, adjustHSV, CV_BGR2HSV);
    split(adjustHSV, hsvImage);
    inRange(hsvImage[2], 200, 255, adjustValue);
    medianBlur(adjustValue, adjustValue, 3);

    //获取更新后矩形框内连通域轮廓
    vector<vector<Point> > contours;
    findContours(updateValue, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    //drawContours(updateRoi, contours, -1, Scalar(0,255,255), 2);

    RotatedRect minArmourRect;
    Rect2d minBoundRect;
    int boundNum = 1;

    //根据更新后框内连通域数量进行不同的矫正
    if(contours.size() == 0)
    {
        minBoundRect = Rect2d(armourBlock.height/4, armourBlock.height/2,
                              armourBlock.width, armourBlock.height);

        return minBoundRect;
    }

    RotatedRect *rotatedRect = new RotatedRect[contours.size()];

    //获取连通域最小外接矩形
    for(unsigned int a = 0; a < contours.size(); ++a)
         rotatedRect[a] = minAreaRect(contours[a]);    

    if(contours.size() == 1)
    {
        searchMatchDomains(minArmourRect, rotatedRect, contours.size(), boundNum);

//        if(boundNum == 0)
//        {
//            if(abs(rotatedRect[0].angle) <= 4 ||abs(rotatedRect[0].angle >= 86))
//                minBoundRect = minArmourRect.boundingRect2f();
//            else
//            {
//                if(armourBlock.x < 0 || armourBlock.x + armourBlock.width > srcImage.cols
//                        ||armourBlock.y < 0 || armourBlock.y + armourBlock.height > srcImage.rows)
//                   refineOverBorder(minArmourRect, rotatedRect);
//                else
//                   refineNonOverBorder(minArmourRect, rotatedRect);
//            }
//        }

        if(3*minArmourRect.size.area() > roi.rows*roi.cols)
            minBoundRect = minArmourRect.boundingRect2f();
    }

    if(contours.size() == 2)
    {
        double area0 = double(rotatedRect[0].size.area()),
               area1 = double(rotatedRect[1].size.area());
        float angle0 = rotatedRect[0].angle, angle1 = rotatedRect[1].angle;

        if(area0 <= 1.5*area1 && area1 <= 1.5*area0
           &&(fabs(angle0 - angle1) < 9 || fabs(fabs(angle0 - angle1) - 90) < 9))
        {
            minBoundRect =Rect2d(armourBlock.height/4, armourBlock.height/2,
                                 armourBlock.width, armourBlock.height);
        }
        else
        {
             searchMatchDomains(minArmourRect, rotatedRect, contours.size(), boundNum);
             if(3*minArmourRect.size.area() > roi.rows*roi.cols)
                 minBoundRect = minArmourRect.boundingRect2f();
        }
    }

    if(contours.size() > 2)
    {
        searchMatchDomains(minArmourRect, rotatedRect, contours.size(), boundNum);
        if(3*minArmourRect.size.area() > roi.rows*roi.cols)
            minBoundRect = minArmourRect.boundingRect2f();
    }

    delete []rotatedRect; rotatedRect = nullptr;

    return minBoundRect;
}

void ArmourTracker::refineOverBorder(RotatedRect& minArmourRect,
                                     RotatedRect* rotatedRect)
{
    Point2f corners[4];
    vector<Point2f> topCorners;
    rotatedRect[0].points(corners);
    Point center0 = rotatedRect[0].center;

    //获取旋转矩形上方两点
    for(unsigned int i = 0; i < 4; ++i)
    {
        if(corners[i].y < center0.y)
            topCorners.push_back(corners[i]);
    }

    minArmourRect.angle = rotatedRect[0].angle;

    //根据旋转矩形的相对更新矩形的为止与上方两点的夹角分四类进行矫正
    if(2*center0.x < updateValue.cols)
    {
        if((topCorners[0].y - topCorners[1].y)/(topCorners[0].x - topCorners[1].x) < 0)
        {
            minArmourRect.center.x = float(center0.x +
                    (updateValue.cols - center0.x)*
                    pow(cos(rotatedRect[0].angle), 2)/2);
            minArmourRect.center.y = center0.y -
                    (updateValue.cols - center0.y)
                    *sin(2*abs(rotatedRect[0].angle))/4;
            minArmourRect.size.width = rotatedRect[0].size.width/2 +
                    (updateValue.cols - center0.x)*
                    cos(rotatedRect[0].angle);
            minArmourRect.size.height = rotatedRect[0].size.height;
        }
        else
        {
            minArmourRect.center.x = float(center0.x +
                    (updateValue.cols - center0.x)*
                    pow(cos(90 + rotatedRect[0].angle), 2)/2);
            minArmourRect.center.y = center0.y +
                    (updateValue.cols - center0.x)*
                    sin(2*(90 - abs(rotatedRect[0].angle)))/4;
            minArmourRect.size.height = rotatedRect[0].size.height/2 +
                    (updateValue.cols - center0.x)*
                    cos(90 + rotatedRect[0].angle);
            minArmourRect.size.width = rotatedRect[0].size.width;
        }
    }
    else
    {
        if((topCorners[0].y - topCorners[1].y)/(topCorners[0].x - topCorners[1].x) < 0)
        {
            minArmourRect.center.x = float(center0.x -
                    center0.x*pow(cos(rotatedRect[0].angle), 2)/2);
            minArmourRect.center.y = center0.y +
                    center0.x*sin(2*abs(rotatedRect[0].angle))/4;
            minArmourRect.size.width = rotatedRect[0].size.width/2 +
                    center0.x*cos(rotatedRect[0].angle);
            minArmourRect.size.height = rotatedRect[0].size.height;
        }
        else
        {
            minArmourRect.center.x = float(center0.x -
                    center0.x*pow(cos(90 + rotatedRect[0].angle), 2)/2);
            minArmourRect.center.y = center0.y -
                    center0.x*sin(2*(90 - abs(rotatedRect[0].angle)))/4;
            minArmourRect.size.height = rotatedRect[0].size.height/2 +
                    center0.x*cos(90 + rotatedRect[0].angle);
            minArmourRect.size.width = rotatedRect[0].size.width;
        }
    }
}

void ArmourTracker::refineNonOverBorder(RotatedRect& minArmourRect,
                                        RotatedRect* rotatedRect)
{
    Point2f corners[4];
    vector<Point2f> topCorners;
    rotatedRect[0].points(corners);

    //获取旋转矩形上方两点
    for(unsigned int i = 0; i < 4; ++i)
    {
        if(corners[i].y < rotatedRect[0].center.y)
            topCorners.push_back(corners[i]);
    }

    minArmourRect.angle = rotatedRect[0].angle;

    //根据旋转矩形的相对更新矩形的为止与上方两点的夹角分四类进行矫正
    if(2*rotatedRect[0].center.x < updateValue.cols)
    {
        if((topCorners[0].y - topCorners[1].y)/(topCorners[0].x - topCorners[1].x) < 0)
        {
            minArmourRect.center.x = rotatedRect[0].center.x +
                    initArmourLength*cos(rotatedRect[0].angle)/2;
            minArmourRect.center.y = rotatedRect[0].center.y +
                    initArmourLength*sin(rotatedRect[0].angle)/2;
            minArmourRect.size.width = rotatedRect[0].size.width/2 +
                    initArmourLength*(1 + sin(rotatedRect[0].angle - gamma));
            minArmourRect.size.height = rotatedRect[0].size.height;
        }
        else
        {
            minArmourRect.center.x = rotatedRect[0].center.x +
                    initArmourLength*cos(90 + rotatedRect[0].angle)/2;
            minArmourRect.center.y = rotatedRect[0].center.y +
                    initArmourLength*sin(90 + rotatedRect[0].angle)/2;
            minArmourRect.size.height = rotatedRect[0].size.height/2 +
                    initArmourLength*(1 + sin(90 + rotatedRect[0].angle - gamma));
            minArmourRect.size.width = rotatedRect[0].size.width;
        }
    }
    else
    {
        if((topCorners[0].y - topCorners[1].y)/(topCorners[0].x - topCorners[1].x) < 0)
        {
            minArmourRect.center.x = rotatedRect[0].center.x -
                    initArmourLength*cos(rotatedRect[0].angle)/2;
            minArmourRect.center.y = rotatedRect[0].center.y -
                    initArmourLength*sin(rotatedRect[0].angle)/2;
            minArmourRect.size.width = rotatedRect[0].size.width/2 +
                    initArmourLength*(1 + sin(rotatedRect[0].angle - gamma));
            minArmourRect.size.height = rotatedRect[0].size.height;
        }
        else
        {
            minArmourRect.center.x = rotatedRect[0].center.x -
                    initArmourLength*cos(90 + rotatedRect[0].angle)/2;
            minArmourRect.center.y = rotatedRect[0].center.y -
                    initArmourLength*sin(90 + rotatedRect[0].angle)/2;
            minArmourRect.size.height = rotatedRect[0].size.height/2 +
                    initArmourLength*(1 + sin(90 + rotatedRect[0].angle - gamma));
            minArmourRect.size.width = rotatedRect[0].size.width;
        }
    }
}

void ArmourTracker::searchMatchDomains(RotatedRect& minArmourRect,
                                       RotatedRect* rotatedRect,
                                       unsigned int rotatedSize,
                                       int& boundNum)
{
    //获取放大后矩形框内连通域数量
    Point2d *rotatedCenter = new Point2d[rotatedSize];
    vector<vector<Point> > contours;
    findContours(adjustValue, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    RotatedRect *updateBlocks = new RotatedRect[rotatedSize];
    RotatedRect *adjustBlocks = new RotatedRect[contours.size() - 1];

    //对矩形框进行计数
    unsigned int updateBlockNum = 0;
    unsigned int preNum = 0;
    unsigned int exceptNum = 0;

    //将放大后矩形框分更新后矩形与非更新后矩形两类
    for(unsigned int i = 0; i < contours.size(); ++i)
    {
        preNum = updateBlockNum;
        for(unsigned int j = 0; j < rotatedSize; ++j)
        {
            rotatedCenter[j].x = double(rotatedRect[j].center.x) + armourBlock.height/4;
            rotatedCenter[j].y = double(rotatedRect[j].center.y) + armourBlock.height/2;

            //判断点与矩形的关系进行分类
            if(pointPolygonTest(contours[i], rotatedCenter[j], false) >= 0)
            {
                updateBlocks[updateBlockNum] = minAreaRect(contours[i]);
                ++updateBlockNum;
            }
        }
        if(preNum == updateBlockNum)
        {
            adjustBlocks[exceptNum] = minAreaRect(contours[i]);
            ++exceptNum;
        }
    }

    if(updateBlockNum && exceptNum)
    {
        vector<RotatedRect> armours;
        RotatedRect *updateClone = new RotatedRect[updateBlockNum];
        RotatedRect *adjustClone = new RotatedRect[exceptNum];
        int updateNum;
        int adjustNum;

        updateNum = cloneScreen(updateBlocks, rotatedSize, updateClone);
        adjustNum = cloneScreen(adjustBlocks, contours.size() - 1, adjustClone);

        //对更新后矩形框内连通域数量大于2的进行匹配，寻找最优匹配
        if(updateNum >= 2)
        {
            armours = updateScreen(updateClone, updateNum);

            if(armours.size() == 0)
                armours = adjustScreen(updateClone, updateNum,
                                         adjustClone, adjustNum);
            if(armours.size() == 1)
                minArmourRect = armours[0];
            if(armours.size() > 1)
                minArmourRect = armourConfidence(armours);
        }

        //对改性后矩形框内连通域与放大后矩形框内非更新矩形框内连通域矩形匹配
        if(updateNum == 1)
        {
            armours = adjustScreen(updateClone, updateNum,
                                     adjustClone, adjustNum);

            if(armours.size() == 0)
                --boundNum;
            if(armours.size() == 1)
                minArmourRect = armours[0];
            if(armours.size() > 1)
                minArmourRect = armourConfidence(armours);
        }

        delete []updateClone;updateClone = nullptr;
        delete []adjustClone;adjustClone = nullptr;
    }

    delete []rotatedCenter; rotatedCenter = nullptr;
    delete []updateBlocks;updateBlocks = nullptr;
    delete []adjustBlocks;adjustBlocks = nullptr;   
}

int ArmourTracker::cloneScreen(RotatedRect* blocks,
                               unsigned int blockSize,
                               RotatedRect* clone)
{
    int num = 0;
    float ration = float(0.8);

    //对单个矩形矩形筛选，初步选出优先矩
    for(unsigned int i = 0; i < blockSize; ++i)
    {
        if(blocks[i].angle > -20)
        {
            if(blocks[i].size.height > ration*blocks[i].size.width)
            {
                clone[num] = blocks[i];
                ++num;
            }
        }

        if(blocks[i].angle < -70)
        {
            if(blocks[i].size.width > ration*blocks[i].size.height)
            {
                clone[num] = blocks[i];
                ++num;
            }
        }
    }

    return num;
}

vector<RotatedRect> ArmourTracker::updateScreen(RotatedRect* updateClone,
                                                int updateNum)
{
    vector<RotatedRect> armours;
    RotatedRect armourRotated;

    //对筛选后矩形进行匹配筛选
    if(updateNum >= 2)
    {
        for(int i = 0; i < updateNum - 1; ++i)
        {
            for(int j = i + 1; j < updateNum; ++j)
            {
                double areaI = double(updateClone[i].size.area()),
                       areaJ = double(updateClone[j].size.area());

                if(areaI > 0.15*areaJ && areaJ > 0.15*areaI)
                {
                    RotatedRect matchDomains[2];

                    double includedAngle = double(atan(fabs((updateClone[i].center.y - updateClone[j].center.y)/
                            (updateClone[i].center.x - updateClone[j].center.x))))*180/CV_PI;
                    if(includedAngle < 30)
                    {
                        float angleI = min(fabs(updateClone[i].angle), 90 - fabs(updateClone[i].angle));
                        float angleJ = min(fabs(updateClone[j].angle), 90 - fabs(updateClone[j].angle));
                        if(fabs(angleI - angleJ) < 15)
                        {
                            //获取两矩形的外接矩形
                            matchDomains[0] = updateClone[i];
                            matchDomains[1] = updateClone[j];
                            armourRotated = getArmourRotated(matchDomains, 2);
                            armours.push_back(armourRotated);
                        }
                    }
                }
            }
        }
    }

    return armours;
}

vector<RotatedRect> ArmourTracker::adjustScreen(RotatedRect* updateClone,
                                                int updateNum,
                                                RotatedRect* adjustClone,
                                                int adjustNum)
{
    vector<RotatedRect> armours;
    RotatedRect armourRotated;

    //对筛选后矩形进行匹配筛选
    if(adjustNum > 1)
    {
        for(int i = 0; i < updateNum; ++i)
        {
            for(int j = 0; j < adjustNum; ++j)
            {
                double areaI = double(updateClone[i].size.area()),
                        areaJ = double(adjustClone[j].size.area());

                if(areaI > 0.15*areaJ && areaJ > 0.15*areaI)
                {
                    RotatedRect matchDomains[2];

                    double includedAngle = double(atan(fabs((updateClone[i].center.y - adjustClone[j].center.y)/
                            (updateClone[i].center.x - adjustClone[j].center.x))))*180/CV_PI;
                    if(includedAngle < 30)
                    {
                        float angleI = min(fabs(updateClone[i].angle), 90 - fabs(updateClone[i].angle));
                        float angleJ = min(fabs(adjustClone[j].angle), 90 - fabs(adjustClone[j].angle));
                        if(fabs(angleI - angleJ) < 15)
                        {
                            //获取两矩形的外接矩形
                            matchDomains[0] = updateClone[i];
                            matchDomains[1] = adjustClone[j];
                            armourRotated = getArmourRotated(matchDomains, 2);
                            armours.push_back(armourRotated);
                        }
                    }
                }
            }
        }
    }

    return armours;
}

RotatedRect ArmourTracker::armourConfidence(vector<RotatedRect>& armours)
{
    RotatedRect *appraiseArmour = new RotatedRect[armours.size()];
    double *appraiseGrade = new double[armours.size()];
    unsigned int num = 0;

    for(unsigned i = 0; i < armours.size(); ++i)
    {
        double grade = 0;
        Point2f fpoints[4];
        armours[i].points(fpoints);

        //浮点数转换整数
        Point points[4];
        for(unsigned int j = 0; j < 4; ++j)
        {
            points[j] = Point(static_cast<int>(fpoints[j].x), static_cast<int>(fpoints[j].y));
        }

        const Point* pts = points;
        const int npts = 4;

        //创建掩码区域为包含装甲板的旋转矩形
        Mat mask(adjustValue.size(), CV_8UC1, Scalar(0));
        //多边形填充
        fillConvexPoly(mask, pts, npts, Scalar(255));

        //使用位“与”运算获取旋转矩形内连通域
        bitwise_and(mask, adjustValue, mask);

        //获取旋转矩形内连通域数量
        vector<vector<Point> >contours;
        findContours(mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

        //对连通域数量为2的旋转矩形进行评分
        if(contours.size() == 2)
        {
            RotatedRect light[2];
            light[0] = minAreaRect(contours[0]);
            light[1] = minAreaRect(contours[1]);

            double angle_0 = double(min(fabs(light[0].angle), 90 - fabs(light[0].angle)));
            double angle_1 = double(min(fabs(light[1].angle), 90 - fabs(light[1].angle)));

            double slantAngle = double(min(fabs(armours[i].angle), 90 - fabs(armours[i].angle)));

            double similarity = matchShapes(contours[0], contours[1], CV_CONTOURS_MATCH_I1, 0);

            //计算两轮廓的分数，由旋转矩形倾斜角，两连通域旋转角差, 轮廓相似度组成
            grade = (slantAngle*CV_PI/180 + 0.01)/5 * (fabs(angle_0 - angle_1) + 1)*similarity/3;

            appraiseArmour[num] = armours[i];
            appraiseGrade[num] = grade;
            ++num;
        }
    }

    RotatedRect optimalArmour = sortArmour(appraiseArmour, appraiseGrade, num);

    delete []appraiseArmour; appraiseArmour = nullptr;
    delete []appraiseGrade; appraiseGrade = nullptr;

    return optimalArmour;
}

RotatedRect ArmourTracker::sortArmour(RotatedRect* appraiseArmour,
                                      double* appraiseGrade,
                                      unsigned int num)
{
    RotatedRect optimalArmour = appraiseArmour[0];
    double lowestGrade = appraiseGrade[0];

    //选出分数最低的旋转矩形为最优矩
    for(unsigned i = 1; i < num; ++i)
    {
        if(appraiseGrade[i] < lowestGrade)
            optimalArmour = appraiseArmour[i];
    }

    return optimalArmour;
}

void ArmourTracker::correctBorders(const Mat srcImage, Rect2d& initRect)
{
    Rect2d region = initRect;

    //对越界矩形边界进行纠正
    if (initRect.x < 0)
    {
        initRect.x = 0;
        initRect.width += region.x;
    }
    if (initRect.x + initRect.width > srcImage.cols)
    {
        initRect.width = srcImage.cols - region.x;
    }
    if (initRect.y < 0)
    {
        initRect.y = 0;
        initRect.height += region.y;
    }
    if ( initRect.y + initRect.height > srcImage.rows)
    {
        initRect.height = srcImage.rows - region.y;
    }
}

RotatedRect ArmourTracker::getArmourRotated(RotatedRect* matchDomains, int matchSize)
{
    RotatedRect armourRotated;
    vector<Point> armourPoints;

    for(int i = 0; i < matchSize; ++i)
    {
        Point2f lightPoints[4];
        matchDomains[i].points(lightPoints);

        for(int j = 0; j < 4; ++j)
            armourPoints.push_back(lightPoints[j]);
    }
    armourRotated = minAreaRect(armourPoints);

    return armourRotated;
}

Rect2d ArmourTracker::getArmourBlock() const
{
    return armourBlock;
}
}
