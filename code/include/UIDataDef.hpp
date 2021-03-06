#ifndef UIDataDef_HH_
#define UIDataDef_HH_

#pragma once

inline bool compareAsc(const QPair<QString, int>& p1, const QPair<QString, int>& p2)
{
    return p1.second <= p2.second;
}

class SimpleRecord
{
public:
    int inspected;      //系统过检总数
    int rejects;        //系统发出的剔废信号数
    int defects;        //系统收到的实际剔废数
    QDateTime dt_start;    //开始时间
    QDateTime dt_end;      //结束时间

    QMap<QString, QMap<QString, int> > mold_sensor_rejects;   //模板ID->SensorID->Count
    QList<QPair<QString, int> > mold_rejects;               //模板剔废数  界面要求升序排列
    QList<QPair<QString, int> > sensor_rejects;             //缺陷剔废数  界面要求升序排列

	QDate shiftDate;	//班次的日期
	int shift;			//班次的整数

    void SetRecord(const Record& r)
    {
        inspected = r.inspected;
        rejects = r.rejects;
        defects = r.defects;
        dt_start = r.dt_start;
        dt_end = r.dt_end;
		shiftDate = r.date;
		shift = r.shift;

        QMap<QString, int> mold_rej;                      //模板剔废数 
        QMap<QString, int> sensor_rej;                    //缺陷剔废数 
        foreach(MoldInfo m, r.moldinfo)
        {
            mold_rej.insert(m.id, m.rejects);
            QMap<QString, int> s_rj;
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
        foreach(auto id, mold_rej.keys())
        {
            mold_rejects.push_back(qMakePair(id, mold_rej[id]));
        }
        foreach(auto id, sensor_rej.keys())
        {
            sensor_rejects.push_back(qMakePair(id, sensor_rej[id]));
        }
        qSort(mold_rejects.begin(), mold_rejects.end(), compareAsc);
        qSort(sensor_rejects.begin(), sensor_rejects.end(), compareAsc);
    }
    void SetRecordV2(const Record& r)
    {
        inspected = r.inspected;
        rejects = r.rejects;
        defects = r.defects;
        dt_start = r.dt_start;
        dt_end = r.dt_end;
        shiftDate = r.date;
        shift = r.shift;

        QMap<QString, int> mold_rej;                      //模板剔废数 
        QMap<QString, int> sensor_rej;                    //缺陷剔废数 
        foreach(MoldInfo m, r.moldinfo)
        {
            mold_rej.insert(m.id, m.rejects);
            QMap<QString, int> s_rj;
            foreach(SensorInfo s, m.sensorinfo)
            {
                //s_rj.insert(s.id, s.rejects);
                bool bSensAdd = false;
                foreach(auto cid, s.addinginfo.keys())
                {
                    auto k = s.id + "-" + cid;
                    if (sensor_rej.contains(k))
                    {
                        sensor_rej[k] += s.addinginfo[cid];
                    }
                    else
                    {
                        sensor_rej.insert(k, s.addinginfo[cid]);
                    }
                    bSensAdd = true;
                }
                if (!bSensAdd)
                {
                    if (sensor_rej.contains(s.id))
                    {
                        sensor_rej[s.id] += s.rejects;
                    }
                    else
                    {
                        sensor_rej.insert(s.id, s.rejects);
                    }
                }
            }
            mold_sensor_rejects.insert(m.id, sensor_rej);
        }
        foreach(auto id, mold_rej.keys())
        {
            mold_rejects.push_back(qMakePair(id, mold_rej[id]));
        }
        foreach(auto id, sensor_rej.keys())
        {
            sensor_rejects.push_back(qMakePair(id, sensor_rej[id]));
        }
        qSort(mold_rejects.begin(), mold_rejects.end(), compareAsc);
        qSort(sensor_rejects.begin(), sensor_rejects.end(), compareAsc);
    }
};

#endif//UIDataDef_HH_