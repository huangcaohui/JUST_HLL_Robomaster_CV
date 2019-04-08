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

#ifndef ARMOUR_DETECTOR_H
#define ARMOUR_DETECTOR_H

#include "image.h"

namespace HCVC {
//! @addtogroup armour_recognition
//! @{

/**
 * @brief 装甲板检测模块
 * @details 在图像预处理之后，检测图像确定装甲板区域，根据一定评价原则选出最佳目标
 */
class ArmourDetector: public Image
{
public:
    //函数重载
    using Image::init;

    /**
    * @brief 加载装甲板区域判定参数
    *
    */
    ArmourDetector();

    /**
     * @brief 装甲板识别参数的初始化
     * @param[in] xmlPath xml文件的路径
     * @return 是否初始化成功
     */
    bool init(string xmlPath);

    /**
    * @brief 检测图像中是否存在装甲板区域
    * @details 对装甲板检测流程总体控制，并将检测出来的装甲板区域旋转矩形存储起来
    * @param[in] srcImage 待检测原图像，图像由摄像头直接传回，并未经过处理
    * @return 图像中是否存在装甲板区域。
    *         返回true，表示检测到装甲板区域；
    *         返回false，表示未检测到装甲板区域
    */
    bool detect(Mat &srcImage);

    /**
    * @brief 获取上一次图像中检测出的最佳装甲板区域
    * @details 对装甲板检测流程总体控制，向外提供运行装甲板检测模块运行接口
    * @return 包围装甲板区域旋转矩形的矩形
    */
    RotatedRect getBestArmourBlock() const;

    //! 装甲板判定参数
    struct Params
    {
        float angleRange;          /*!< 单团块检测，倾斜角度限制 */
        float minArea;             /*!< 单团块检测，最小面积限制 */
        float maxHeightWidthRat;   /*!< 单团块检测，最大高宽比 */
        float minHeightWidthRat;   /*!< 单团块检测，最小高宽比 */

        float tanAngle;            /*!< 灯柱角度夹角设置，最大角度差 */
        float deviationAngle;     /*!< 甲板像素检测，区间范围外像素比例 */
        float armourPixelAvg;      /*!< 甲板像素检测，甲板像素平均值 */
    }params;

private:
    /**
     * @brief 最佳装甲板区域结构体
     *
     */
    struct OptimalArmourBlock
    {
        //! 包围装甲板区域的选装矩形
        RotatedRect block;
        //! 对该装甲板区域的评分
        double grade;

        //! 初始化参数
        OptimalArmourBlock(const RotatedRect& _block, const double& _grade):
        block(_block), grade(_grade)
        {}

        //! 重载小于符号，便于直接使用sort函数，按从小到大排序
        bool operator < (const OptimalArmourBlock& other) const
        {
            return grade < other.grade;
        }
    };

    //！ 按顺序存储最优的装甲板区域
    vector<OptimalArmourBlock> optimalArmourBlocks;

    /**
    * @brief 搜寻图中所有独立的团块
    * @param[in] srcImage 待检测原图像
    * @return 所有连通块的点集合
    */
    vector<vector<Point> > searchBlocks(Mat srcImage);

    /**
    * @brief 用广度优先搜索填充每一个连通块
    * @param[in] srcImage 待检测原图像
    * @param[out] blocks 所有连通块的点集合，初始为空
    * @param[in] row 连通块第一个点的纵坐标
    * @param[in] col 连通块第一个点的横坐标
    * @return null
    */
    void fillLampBlock(Mat& srcImage,
                       vector<vector<Point> >& blocks,
                       int row, int col);

    /**
    * @brief 计算每一个团块的信息，并进行初步的筛选即去除掉一些不符合条件的团块
    * @param[in] blocks 团块点的集合
    * @param[out] lampsNum 初步检测最小外接矩形的实际数量
    * @return 包围团块的最小旋转矩形数组
    */
    vector<RotatedRect> calcBlocksInfo(const vector<vector<Point> >& blocks, int& lampsNum);

    /**
    * @brief 进一步筛选，多个灯柱与甲板进行处理,最后存入armourBlocks
    * @details 分为两灯柱初步信息检测，甲板像素离散度检测与灯柱减连通域数量检测
    *          运用两个灯柱的信息计算甲板像素离散程度，
    *          计算两灯柱框定区域内连通域数量以检测确定甲板
    * @param[out] armourBlocks 包围装甲板的旋转矩形
    * @param[in] lampBlocks 通过单个灯柱信息筛选所确立的灯柱团块
    * @param[in] dstImage 原二值化图像
    * @param[in] value 亮度图像
    * @param[in] lampsNum 初步检测最小外接矩形的实际数量,armourBlocks数组的数量
    * @param[in] directAngle 灯柱的夹角
    * @param[out] armoursNum 装甲板检测后获得的装甲板数量
    * @param[out] drawImage 画完框的图像
    * @return null
    */
    void extracArmourBlocks(RotatedRect* armourBlocks,
                            const RotatedRect* lampBlocks,
                            const Mat dstImage,
                            const Mat value,
                            const int lampsNum,
                            double* directAngle,
                            int &armoursNum,
                            Mat &srcImage);

    /**
     * @brief 计算灯柱的与x轴最小的方向向量
     * @param rotated 输入的待计算的矩形
     * @return 与x轴最小的方向向量
     */
    Point calVectorX(const RotatedRect rotated);

    /**
     * @brief 计算灯柱的与y轴最小的方向向量
     * @param rotated 输入的待计算的矩形
     * @return 与y轴最小的方向向量
     */
    Point calVectorY(const RotatedRect rotated);

    /**
     * @brief 连通域数量检测
     * @details 求两灯柱外接矩形，检测矩形内的连通域，若连通域数量为2，
     *          对此两灯柱块确定最小外接矩形并返回该灯柱团块
     * @param[in] initLightBlocks 通过两灯柱初步信息检测所筛选出来的灯柱块
     * @param[in] screenLamps 筛选出来的不重复的外接矩形
     * @param[in] dstImgage 原二值化图像
     * @param[in] labelValue 连通域检测的连通域个数
     * @param[in] screenNum screenLamps的实际长度
     * @return null
     */
    void domainCountDetect(const RotatedRect* initLightBlocks,
                           const RotatedRect* screenLamps,
                           const Mat& dstImage,
                           const Mat& value,
                           unsigned int &labelValue,
                           const int screenNum);

    /**
     * @brief 获得两团块的最小外接矩形
     * @param[in] initLightBlocks 连通域筛选出来的团块
     * @param[in] lightsNum initLightBlocks数组的长度
     * @return 团块的最小外包围矩形
     */
    RotatedRect getArmourRotated(RotatedRect* initLightBlocks, int lightsNum);

    /**
     * @brief 根据灰度图计算甲板的区间范围的值
     * @details 计算甲板像素平均值左右与大于某区间的像素比例
     * @param[in] armourReserve 通过两灯柱初步信息检测所筛选出来的团块
     * @param[in] srcImage 原彩色图像
     * @param[in] dstImage 原二值化图像
     * @param[out] armourPixelAvg 像素的平均值
     * @param[out] tanAngle 设定区间范围内像素的平均值
     * @param[out] deviationAngle 设定区间范围外像素的平均值
     * @return null
     */
    void calcDeviation(RotatedRect armourReserve,
                       const Mat& srcImage,
                       const Mat& dstImage,
                       double& armourPixelAvg,
                       double& tanAngle,
                       double& deviationAngle,
                       double& armourStandard);

    /**
    * @brief 对最后提取出的灯柱区域评分，选出最优区域
    * @details 通过长宽比区分错误匹配与误判
    *          通过灰度图离散系数确定多辆车时的远近，选择最近攻击目标
    *          通过两变量乘积最小化极小值，获得最优攻击目标
    * @param[in] srcImage 待检测原图像
    * @param[in] dstImage 对原图像进行图像预处理后的图像
    * @param[in] armourBlocks 包围装甲板区域的最小旋转矩形数组
    * @param[in] directAngle 两灯柱的y方向上的夹角
    * @param[in] armoursNum 装甲板检测后获得的装甲板数量
    * @return null
    */
    void markArmourBlocks(const Mat& srcImage,
                          const Mat& dstImage,
                          const RotatedRect* armourBlocks,
                          const double* directAngle,
                          int armoursNum);

    /**
     * @brief 计算装甲板的品均值
     * @param[in] srcImage 输入的原图像
     * @param[in] armour 输入的装甲板区域
     * @param[in] armour 输入的装甲板
     * @param return null
     */
    double calAverage(const Mat srcImage, const Mat dstImage, RotatedRect armour);

    /**
     * @brief 绘制两矩形掩膜
     * @param[in] armourBlock 输入矩形
     * @param[in] dstImage 输入二值化图像
     * @return 掩膜面积
     */
    int extractMask(const RotatedRect* armourBlocks, Mat dstImage);

    /**
     * @brief 对边界进行矫正
     * @param[out] left 左边界
     * @param[out] top 上边界
     * @param[out] width 宽度
     * @param[out] height 高度
     * @param[in] 最大边界范围图像
     * @return null;
     */
    void correctBorder(int& left, int& top, int& width, int& height, Mat image);

    /**
    * @brief 减去旋转矩形的边角，使旋转矩形的左右两条边与竖直方向平行，成为平行四边形
    * @param[in] points 旋转矩形的四个角点
    * @return null
    */
    void cutEdgeOfRect(Point2f* points);
};
//! @}
}
#endif // ARMOUR_DETECTOR_H

