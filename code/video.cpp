#include "video.h"

namespace HCVC {
Video::Video()
{

}

bool Video::init(string path)
{
    srcFilePath = path;
    srcFile.open(srcFilePath);

    return srcFile.isOpened();
}

void Video::getNextFrame(Mat &frame)
{
    srcFile.read(frame);
}

VideoCapture &Video::operator >>(Mat &frame)
{
    srcFile >> frame;
    return srcFile;
}

VideoCapture &Video::getVideo()
{
    return srcFile;
}
}
