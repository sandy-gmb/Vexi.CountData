#ifndef DATA_DEFINE_H_
#define DATA_DEFINE_H_

#pragma once

#include <QList>
#include <QDateTime>


class SensorAddingInfo
{
public:
    int counter_id;     //缺陷信息的counter id 
    QString nb;         //缺陷信息Nb 由于Nb可能是整型和浮点型 所以先存成字符串，显示时再根据含义转换为整型或者浮点型
    SensorAddingInfo()
    {
        counter_id = 0;
        nb = "";
    }
};

class SensorInfo
{
public:
    int id;             //缺陷代码
    int rejects;        //系统发出的剔废信号数
    int defects;        //系统收到的实际剔废数

    QList<SensorAddingInfo> addinginfo; //缺陷附加信息

    SensorInfo()
    {
        id = -1;
        rejects = 0;
        defects = 0;
    }
};

class MoldInfo
{
public:
    int id;             //模号
    int inspected;      //系统过检总数
    int rejects;        //系统发出的剔废信号数
    int defects;        //系统收到的实际剔废数
    int autorejects;    //自动剔废数(以上两个的差值，此值还是传入不是自动计算)

    QList<SensorInfo>   sensorinfo; //缺陷检测信息

    MoldInfo()
    {
        id = -1;
        inspected = 0;
        rejects = 0;
        defects = 0;
        autorejects = 0;
    }
};

//用于保存传入的XML字符串包含的数据
class XmlData{
public:
    QString id;         //MachineID
    int inspected;      //系统过检总数
    int rejects;        //系统发出的剔废信号数
    int defects;        //系统收到的实际剔废数
    int autorejects;    //自动剔废数(以上两个的差值，此值还是传入不是自动计算)

    QDateTime dt_start;    //开始时间
    QDateTime dt_end;      //结束时间

    QList<MoldInfo>     moldinfo;   //模号数据

    XmlData()
    {
        id = "";
        inspected = 0;
        rejects = 0;
        defects = 0;
        autorejects = 0;
    }
};


#define  SAFE_SET(s,v) {\
    if(s != nullptr)\
{\
    *s = v;\
}\
}

#endif//DATA_DEFINE_H_
