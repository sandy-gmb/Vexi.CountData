#ifndef DATA_DEFINE_H_
#define DATA_DEFINE_H_

#pragma once

#include <QList>
#include <QDateTime>


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
};

class MoldInfo
{
public:
    int id;             //ģ��
    int inspected;      //ϵͳ��������
    int rejects;        //ϵͳ�������޷��ź���
    int defects;        //ϵͳ�յ���ʵ���޷���
    int autorejects;    //�Զ��޷���(���������Ĳ�ֵ����ֵ���Ǵ��벻���Զ�����)

    QList<SensorInfo>   sensorinfo; //ȱ�ݼ����Ϣ

    MoldInfo()
    {
        id = -1;
        inspected = 0;
        rejects = 0;
        defects = 0;
        autorejects = 0;
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

    QList<MoldInfo>     moldinfo;   //ģ������

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
