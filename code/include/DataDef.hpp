#ifndef DATA_DEFINE_H_
#define DATA_DEFINE_H_

#pragma once

#include <QList>
#include <QDateTime>
#include <QMap>

class SensorInfo
{
public:
    int id;             //缺陷代码
    int rejects;        //系统发出的剔废信号数
    int defects;        //系统收到的实际剔废数

	QMap<int, int> addinginfo;	//缺陷附加信息 count_id特征值索引 映射 Nb计数

    SensorInfo()
    {
        id = -1;
        rejects = 0;
        defects = 0;
    }

    void MergeData(const SensorInfo& t)
    {
        rejects += t.rejects;
        defects += t.defects;
		foreach(int k, t.addinginfo.keys())
		{
			if(addinginfo.contains(k))
			{
				addinginfo[k] += t.addinginfo.value(k);
			}
			else
			{
				addinginfo.insert(k, t.addinginfo.value(k));
			}

		}
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

    QList<SensorInfo>   sensorinfo;     //缺陷检测信息
    QMap<int, int>      sensorid_idx;   //缺陷ID对应索引,用于合并

    MoldInfo()
    {
        id = -1;
        inspected = 0;
        rejects = 0;
        defects = 0;
        autorejects = 0;
    }

    void GenerateSensorIdx()
    {
        for(int i = 0; i < sensorinfo.size();i++)
        {
            sensorid_idx.insert( sensorinfo[i].id, i);
        }
    }

    void MergeData( MoldInfo& t)
    {
        if(sensorid_idx.isEmpty())
        {
            GenerateSensorIdx();
        }
        if(t.sensorid_idx.isEmpty())
        {
            t.GenerateSensorIdx();
        }
        inspected += t.inspected;
        rejects += t.rejects;
        defects += t.defects;
        autorejects += t.autorejects;
        //缺陷合并是对应缺陷合并
        foreach(int id, t.sensorid_idx.keys())
        {
            if(sensorid_idx.contains(id))
            {
                sensorinfo[sensorid_idx[id]].MergeData(t.sensorinfo[t.sensorid_idx[id]]);
            }
            else
            {
                sensorinfo.push_back(t.sensorinfo[t.sensorid_idx[id]]);
                sensorid_idx.insert(id, sensorinfo.size()-1);
            }
        }
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

    QList<MoldInfo>     moldinfo;       //模号数据
    QMap<int, int>      moldid_idx;     //模号对应索引,用于合并

    XmlData()
    {
        id = "";
        inspected = 0;
        rejects = 0;
        defects = 0;
        autorejects = 0;
    }

    void GenerateModIdx()
    {
        for(int i = 0; i < moldinfo.size();i++)
        {
            moldid_idx.insert(moldinfo[i].id, i);
        }
    }

    void MergeData(XmlData& t)
    {
        if(moldid_idx.isEmpty())
        {
            GenerateModIdx();
        }
        if(t.moldid_idx.isEmpty())
        {
            t.GenerateModIdx();
        }
        inspected += t.inspected;
        rejects += t.rejects;
        defects += t.defects;
        autorejects += t.autorejects;
        //模板合并是对应模板合并
        foreach(int id, t.moldid_idx.keys())
        {
            if(moldid_idx.contains(id))
            {
                moldinfo[moldid_idx[id]].MergeData(t.moldinfo[t.moldid_idx[id]]);
            }
            else
            {
                moldinfo.push_back(t.moldinfo[t.moldid_idx[id]]);
                moldid_idx.insert(id, moldinfo.size()-1);
            }
        }
    }
};

//用于界面和数据中心传递数据的结构体
typedef XmlData Record;

enum ETimeInterval
{
    ETI_30_Min,
    ETI_60_Min,
    ETI_90_Min,
    ETI_120_Min,
};

inline int ETimeInterval2Min(ETimeInterval eti)
{
    int ti = 30;
    switch(eti)
    {
    case ETI_30_Min:
        {
            ti = 30;
        }
        break;
    case ETI_60_Min:
        {
            ti = 60;
        }
        break;
    case ETI_90_Min:
        {
            ti = 90;
        }
        break;
    case ETI_120_Min:
        {
            ti = 120;
        }
        break;
    }
    return ti;
}


#define  SAFE_SET(s,v) {\
    if(s != nullptr)\
{\
    *s = v;\
}\
}

#endif//DATA_DEFINE_H_
