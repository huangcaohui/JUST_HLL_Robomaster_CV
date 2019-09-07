#include "camera.h"

namespace HCVC {
Camera::Camera()
{

}

bool Camera::init(int cameraId, string xmlPath)
{
    FileStorage fs(xmlPath, FileStorage::READ);
    if(!fs.isOpened())
    {
        cout << "Open " << xmlPath << " file failed" << endl;
    }

    FileNode node = fs["camera_parameter"];

    node["brightness"] >> params.brightness;
    node["contrast"] >> params.contrast;
    node["hue"] >> params.hue;
    node["saturation"] >> params.saturation;
    node["pan"] >> params.pan;
    node["gamma"] >> params.gamma;
    node["white_balance_red_v "] >> params.white_balance_red_v;
    node["backlight"]>> params.backlight;
    node["gain"] >> params.gain;
    node["exposure"] >>params.exposure;

    srcFile.open(cameraId);
    srcFile.set(CAP_PROP_FRAME_WIDTH, 1280);
    srcFile.set(CAP_PROP_FRAME_HEIGHT, 720);
    srcFile.set(CAP_PROP_BRIGHTNESS, params.brightness);
    srcFile.set(CAP_PROP_CONTRAST, params.contrast);
    srcFile.set(CAP_PROP_HUE, params.hue);
    srcFile.set(CAP_PROP_SATURATION, params.saturation);
    srcFile.set(CAP_PROP_PAN, params.pan);
    srcFile.set(CAP_PROP_GAMMA, params.gamma);
    srcFile.set(CAP_PROP_WHITE_BALANCE_RED_V, params.white_balance_red_v);
    srcFile.set(CAP_PROP_BACKLIGHT, params.backlight);
    srcFile.set(CAP_PROP_GAIN, params.gain);
    srcFile.set(CAP_PROP_EXPOSURE, params.exposure);
    srcFile.set(CAP_PROP_SETTINGS, -1);

    return srcFile.isOpened();
}

bool Camera::writeCamParams(string xmlPath)
{
    double brightnessDate = srcFile.get(CAP_PROP_BRIGHTNESS);
    double contrastDate = srcFile.get(CAP_PROP_CONTRAST);
    double hueDate = srcFile.get(CAP_PROP_HUE);
    double saturationDate = srcFile.get(CAP_PROP_SATURATION);
    double panDate = srcFile.get(CAP_PROP_PAN);
    double gammaDate = srcFile.get(CAP_PROP_GAMMA);
    double white_balance_red_vDate = srcFile.get(CAP_PROP_WHITE_BALANCE_RED_V);
    double backlightDate = srcFile.get(CAP_PROP_BACKLIGHT);
    double gainDate = srcFile.get(CAP_PROP_GAIN);
    double exposureDate = srcFile.get(CAP_PROP_EXPOSURE);

    std::cout<<brightnessDate<<'\t';
    std::cout<<contrastDate<<'\t';
    std::cout<<hueDate<<'\t';
    std::cout<<saturationDate<<'\t';
    std::cout<<panDate<<'\t';
    std::cout<<gammaDate<<'\t';
    std::cout<<white_balance_red_vDate<<'\t';
    std::cout<<backlightDate<<'\t';
    std::cout<<gainDate<<'\t';
    std::cout<<exposureDate<<'\t'<<'\n';

    QFile file(QString::fromStdString(xmlPath));
    QDomDocument doc;
    if(!doc.setContent(&file))
    {
        file.close();
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    QDomNodeList list = root.elementsByTagName("camera_parameter");

    //定位到第三个一级子节点的子元素并寻找下一个参数(主定位功能区）
    QDomNode brightness = list.at(list.size() - 1).firstChild();
    QDomNode contrast = brightness.nextSibling();
    QDomNode hue = contrast.nextSibling();
    QDomNode saturation = hue.nextSibling();
    QDomNode pan = saturation.nextSibling();
    QDomNode gamma = pan.nextSibling();
    QDomNode white_balance_red_v = gamma.nextSibling();
    QDomNode backlight = white_balance_red_v.nextSibling();
    QDomNode gain = backlight.nextSibling();
    QDomNode exposure = gain.nextSibling();

    //寻找到对应节点的值并替换
    QString b = QString::number(brightnessDate);
    QString brightnessString = QString("%1 %2 %3").arg("\0").arg(b).arg("\0");
    QDomNode brightnessOld = brightness.firstChild();
    brightness.firstChild().setNodeValue(brightnessString);
    QDomNode brightnessNew = brightness.firstChild();
    brightness.replaceChild(brightnessNew,brightnessOld);

    QString c = QString::number(contrastDate);
    QString contrastString = QString("%1 %2 %3").arg("\0").arg(c).arg("\0");
    QDomNode contrastOld = contrast.firstChild();
    contrast.firstChild().setNodeValue(contrastString);
    QDomNode contrastNew = contrast.firstChild();
    contrast.replaceChild(contrastNew, contrastOld);

    QString h = QString::number(hueDate);
    QString hueString = QString("%1 %2 %3").arg("\0").arg(h).arg("\0");
    QDomNode hueOld = hue.firstChild();
    hue.firstChild().setNodeValue(hueString);
    QDomNode hueNew = hue.firstChild();
    hue.replaceChild(hueNew, hueOld);

    QString s = QString::number(hueDate);
    QString saturationString = QString("%1 %2 %3").arg("\0").arg(h).arg("\0");
    QDomNode saturationOld = saturation.firstChild();
    saturation.firstChild().setNodeValue(saturationString);
    QDomNode saturationNew = saturation.firstChild();
    saturation.replaceChild(saturationNew, saturationOld);

    QString p = QString::number(panDate);
    QString panString = QString("%1 %2 %3").arg("\0").arg(h).arg("\0");
    QDomNode panOld = pan.firstChild();
    pan.firstChild().setNodeValue(panString);
    QDomNode panNew = pan.firstChild();
    pan.replaceChild(panNew, panOld);

    QString g = QString::number(gammaDate);
    QString gammaString = QString("%1 %2 %3").arg("\0").arg(h).arg("\0");
    QDomNode gammaOld = gamma.firstChild();
    gamma.firstChild().setNodeValue(gammaString);
    QDomNode gammaNew = gamma.firstChild();
    gamma.replaceChild(gammaNew, gammaOld);

    QString w = QString::number(white_balance_red_vDate);
    QString white_balance_red_vString = QString("%1 %2 %3").arg("\0").arg(h).arg("\0");
    QDomNode white_balance_red_vOld = white_balance_red_v.firstChild();
    white_balance_red_v.firstChild().setNodeValue(white_balance_red_vString);
    QDomNode white_balance_red_vNew = white_balance_red_v.firstChild();
    white_balance_red_v.replaceChild(white_balance_red_vNew, white_balance_red_vOld);

    QString ba = QString::number(backlightDate);
    QString backlightString = QString("%1 %2 %3").arg("\0").arg(h).arg("\0");
    QDomNode backlightOld = backlight.firstChild();
    backlight.firstChild().setNodeValue(backlightString);
    QDomNode backlightNew = backlight.firstChild();
    backlight.replaceChild(backlightNew, backlightOld);

    QString ga = QString::number(gainDate);
    QString gainString = QString("%1 %2 %3").arg("\0").arg(h).arg("\0");
    QDomNode gainOld = gain.firstChild();
    gain.firstChild().setNodeValue(gainString);
    QDomNode gainNew = gain.firstChild();
    gain.replaceChild(gainNew, gainOld);

    QString e = QString::number(exposureDate);
    QString exposureString = QString("%1 %2 %3").arg("\0").arg(h).arg("\0");
    QDomNode exposureOld = exposure.firstChild();
    exposure.firstChild().setNodeValue(exposureString);
    QDomNode exposureNew = exposure.firstChild();
    exposure.replaceChild(exposureNew, exposureOld);

    if(!file.open(QFile::WriteOnly|QFile::Truncate))
        return false;

    //输出到文件
    QTextStream out_stream(&file);

    //缩进4格
    doc.save(out_stream, 4);
    file.close();

    return true;
}

VideoCapture &Camera::operator >>(Mat &frame)
{
    srcFile >> frame;
    return srcFile;
}

VideoCapture &Camera::getCamera()
{
    return srcFile;
}
}
