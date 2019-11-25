#ifndef PASER_RXY62PZ297UNKQRI_H_
#define PASER_RXY62PZ297UNKQRI_H_

#include "DataDef.hpp"
#include <QDomElement>

/* XMLPaser 用于解析的WebService返回的工作数据XML字符串的类，示例字符串放在库的头文件的说明注释中
*
*/

class XMLPaser{
public:
    //解析Sensor缺陷信息
    static bool PaserSensorInfo(QDomElement& sensor, SensorInfo& data,QString* err);

    //解析Mold信息
    static bool PaserMoldInfo(QDomElement& mold, MoldInfo& data,QString* err);

    //解析设备节点
    static bool PaserMachineInfo(QDomElement& root, XmlData& data,QString* err);
    //解析XML数据
    static bool PaserInfo(const QString& xmlstr, XmlData& data,QString* err = nullptr);

public:
    //XmlData tempdata;   //存放一次解析的临时数据
};

#endif//PASER_RXY62PZ297UNKQRI_H_
