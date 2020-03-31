#ifndef DATA_DEFINE_H_
#define DATA_DEFINE_H_

#pragma once

#include <QList>
#include <QDateTime>
#include <QMap>
#include <QTime>

class SensorInfo
{
public:
    int id;             //ȱ�ݴ���
    int rejects;        //ϵͳ�������޷��ź���
    int defects;        //ϵͳ�յ���ʵ���޷���

	QMap<int, int> addinginfo;	//ȱ�ݸ�����Ϣ count_id����ֵ���� ӳ�� Nb����

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

	QList<MoldInfo>     moldinfo;       //ģ������
	QMap<int, int>      moldid_idx;     //ģ�Ŷ�Ӧ����,���ںϲ�

    QDateTime dt_start;    //��ʼʱ��
    QDateTime dt_end;      //����ʱ��

	QDate date;			//���� ��ε�����
	int	shift;			//���	��0��ʼ

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

class Shift
{
public:
	Shift(const QList<QTime>& tlst)
		: shifttime(tlst)
	{
		sorttime = tlst;
		qSort(sorttime);
		if(tlst.size() == 1)
		{
			changtime = tlst.first();
			afterflag = false;
		}
		else
		{
			changtime = tlst.first();
			if(24 - tlst.first().hour() >= 12)
			{
				afterflag = false;
			}
			else	
			{
				afterflag = true;
			}
		}
	}

	QDateTime getNextShiftTime(const QDateTime& _t)
	{
		foreach (QTime t, sorttime)
		{
			if(_t.time() <= t)
			{
				return QDateTime(_t.date(), t);
			}
		}
		return QDateTime(_t.date().addDays(1), sorttime.first());
	};

	QDateTime getLastShiftTime(const QDateTime& t){
		for (int i = sorttime.size() - 1; i >= 0; i--)
		{
			if(t.time() > sorttime[i])
			{
				return QDateTime(t.date(), sorttime[i]);
			}
		}
		return QDateTime(t.date().addDays(-1), sorttime.last());
	};

	void getShiftDateShift(const QDateTime& t, QDate& d, int shift)
	{
		//���ֻ����һ����
		if(shifttime.size() == 1)
		{
			shift = 0;
			if(t.time() < shifttime.first())
			{
				d = t.date().addDays(-1);
			}
			else
			{
				d = t.date();
			}
			return;
		}
		if(t.time() <= changtime && !afterflag)
		{//ʱ�����л�ʱ��֮ǰ �� �л���+1
			d = t.date().addDays(-1);
		}
		else if(t.time() > changtime && afterflag)
		{//ʱ�����л�ʱ��֮�� �� �л���+1
			d = t.date().addDays(1);
		}
		else
		{
			d = t.date();
		}
		QTime st = getLastShiftTime(t).time();
		QTime et = getNextShiftTime(t).time();
		shift = -1;
		for (int i = 0; i < shifttime.size()-1; i++)
		{
			if(st == shifttime[i] && et == shifttime[i+1])
			{
				shift = i;
				break;
			}
		}
		if(shift == -1)
		{
			shift = shifttime.size()-1;
		}
	}
private:
	QList<QTime> shifttime;	//����л�ʱ���
	QList<QTime> sorttime;	//����ʱ��
	
	QTime changtime;		//һ����л�ʱ���
	bool afterflag;			//�л�ʱ���֮���Ƿ�+1 ���������֮ǰ��-1
};

enum ERecordType
{
	ERT_TimeInterval,
	ERT_Shift,
};

#define  SAFE_SET(s,v) {\
    if(s != nullptr)\
{\
    *s = v;\
}\
}

#endif//DATA_DEFINE_H_
