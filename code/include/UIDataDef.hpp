#ifndef UIDataDef_HH_
#define UIDataDef_HH_

#pragma once

inline bool compareAsc(const QPair<int, int>& p1, const QPair<int, int>& p2)
{
    return p1.second <= p2.second;
}

class SimpleRecord
{
public:
    int inspected;      //ϵͳ��������
    int rejects;        //ϵͳ�������޷��ź���
    int defects;        //ϵͳ�յ���ʵ���޷���
    QDateTime dt_start;    //��ʼʱ��
    QDateTime dt_end;      //����ʱ��

    QMap<int, QMap<int, int> > mold_sensor_rejects;   //ģ��ID->SensorID->Count
    QList<QPair<int, int> > mold_rejects;               //ģ���޷���  ����Ҫ����������
    QList<QPair<int, int> > sensor_rejects;             //ȱ���޷���  ����Ҫ����������

	QDate shiftDate;	//��ε�����
	int shift;			//��ε�����

    void SetRecord(const Record& r)
    {
        inspected = r.inspected;
        rejects = r.rejects;
        defects = r.defects;
        dt_start = r.dt_start;
        dt_end = r.dt_end;
		shiftDate = r.date;
		shift = r.shift;

        QMap<int, int> mold_rej;                      //ģ���޷��� 
        QMap<int, int> sensor_rej;                    //ȱ���޷��� 
        foreach(MoldInfo m, r.moldinfo)
        {
            mold_rej.insert(m.id, m.rejects);
            QMap<int, int> s_rj;
            foreach(SensorInfo s, m.sensorinfo)
            {
                s_rj.insert(s.id, s.rejects);
                if(sensor_rej.contains(s.id))
                {
                    sensor_rej[s.id] += s.rejects;
                }
                else
                {
                    sensor_rej.insert(s.id, s.rejects);
                }
            }
            mold_sensor_rejects.insert(m.id, s_rj);
        }
        foreach(int id, mold_rej.keys())
        {
            mold_rejects.push_back(qMakePair(id, mold_rej[id]));
        }
        foreach(int id, sensor_rej.keys())
        {
            sensor_rejects.push_back(qMakePair(id, sensor_rej[id]));
        }
        qSort(mold_rejects.begin(), mold_rejects.end(), compareAsc);
        qSort(sensor_rejects.begin(), sensor_rejects.end(), compareAsc);
    }
    
};

#endif//UIDataDef_HH_