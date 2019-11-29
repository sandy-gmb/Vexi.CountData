#ifndef DATA_DEFINE_H_
#define DATA_DEFINE_H_

#pragma once

#include <QList>
#include <QDateTime>
#include <QMap>


class SensorAddingInfo
{
public:
    int counter_id;     //ȱ����Ϣ��counter id 
    QString nb;         //ȱ����ϢNb ����Nb���������ͺ͸����� �����ȴ���ַ�������ʾʱ�ٸ��ݺ���ת��Ϊ���ͻ��߸�����
    SensorAddingInfo()
    {
        counter_id = 0;
        nb = "";
    }
};

class SensorInfo
{
public:
    int id;             //ȱ�ݴ���
    int rejects;        //ϵͳ�������޷��ź���
    int defects;        //ϵͳ�յ���ʵ���޷���

    QList<SensorAddingInfo> addinginfo; //ȱ�ݸ�����Ϣ

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
        //������Ϣ���úϲ�
    }
};

class MoldInfo
{
public:
    int id;             //ģ��
    int inspected;      //ϵͳ��������
    int rejects;        //ϵͳ�������޷��ź���
    int defects;        //ϵͳ�յ���ʵ���޷���
    int autorejects;    //�Զ��޷���(���������Ĳ�ֵ����ֵ���Ǵ��벻���Զ�����)

    QList<SensorInfo>   sensorinfo;     //ȱ�ݼ����Ϣ
    QMap<int, int>      sensorid_idx;   //ȱ��ID��Ӧ����,���ںϲ�

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
        //ȱ�ݺϲ��Ƕ�Ӧȱ�ݺϲ�
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

//���ڱ��洫���XML�ַ�������������
class XmlData{
public:
    QString id;         //MachineID
    int inspected;      //ϵͳ��������
    int rejects;        //ϵͳ�������޷��ź���
    int defects;        //ϵͳ�յ���ʵ���޷���
    int autorejects;    //�Զ��޷���(���������Ĳ�ֵ����ֵ���Ǵ��벻���Զ�����)

    QDateTime dt_start;    //��ʼʱ��
    QDateTime dt_end;      //����ʱ��

    QList<MoldInfo>     moldinfo;       //ģ������
    QMap<int, int>      moldid_idx;     //ģ�Ŷ�Ӧ����,���ںϲ�

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
        //ģ��ϲ��Ƕ�Ӧģ��ϲ�
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

//���ڽ�����������Ĵ������ݵĽṹ��
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
