#include "PaserData.h"

#include <QMap>
#include <QObject>
#include <QDomElement>
#include <logger.h>

//定义特定节点和属性名
//General
const QString& Node_Mach = QObject::tr("Machine");
const QString& Node_Mold = QObject::tr("Mold");
const QString& Node_Sensor = QObject::tr("Sensor");
const QString& Node_Counter = QObject::tr("Counter");

const QString& Node_Inspected = QObject::tr("Inspected");
const QString& Node_Rejects = QObject::tr("Rejects");
const QString& Node_Defects = QObject::tr("Defects");

const QString& Attr_Other_ID = QObject::tr("id");
const QString& Attr_Nb = QObject::tr("Nb");

//VEXI
const QString& Node_Root = QObject::tr("Root");
const QString& Attr_Mach_ID = QObject::tr("Id");
const QString& Node_Autoreject = QObject::tr("Autoreject");
//ProtocolV2
const QString& Node_Root_V2 = QObject::tr("string");


bool XMLPaser::PaserSensorInfo(QDomElement& sensor, SensorInfo& data,QString* err)
{
    if(!sensor.hasChildNodes())
    {//没有子节点数据
        SAFE_SET(err, QObject::tr("Paser Sensor Node, No Child Nodes"));
        return false;
    }
    bool res = false;
    data.id = sensor.attribute(Attr_Other_ID);
    if(data.id.isEmpty())
    {
        SAFE_SET(err, QObject::tr("Paser Sensor Node 'id',Value Could't be empty."));
        return false;
    }

    QDomNodeList sensor_children = sensor.childNodes();
    QMap<QString, bool> chkflg; 
    chkflg.insert(Node_Rejects, false);
    chkflg.insert(Node_Defects, false);
    chkflg.insert(Node_Counter, false);
    for(int i = 0; i < sensor_children.length();i++)
    {
        QDomElement t = sensor_children.at(i).toElement();
        res = false;
        if(t.tagName() == Node_Rejects)
        {
            data.rejects = t.text().toInt(&res);
            if(!res)
            {
                SAFE_SET(err, QObject::tr("Paser Sensor Node 'Inspected',Value Could't Convert To Int"));
                return false;
            }
        }
        else if(t.tagName() == Node_Defects)
        {
            data.defects = t.text().toInt(&res);
            if(!res)
            {
                SAFE_SET(err, QObject::tr("Paser Sensor Node 'Defects',Value Could't Convert To Int"));
                return false;
            }
        }
        else if(t.tagName() == Node_Counter)
        {
            QString counter_id = "-1";
            int nb;
            nb = 0;
            counter_id = t.attribute(Attr_Other_ID);
            if(counter_id.isEmpty())
            {
                SAFE_SET(err, QObject::tr("Paser Sensor Node Counter,Value Could't be empty."));
                return false;
            }
            nb = t.attribute(Attr_Nb,"").toInt(&res);
            if(!res)
            {
                SAFE_SET(err, QObject::tr("Paser Sensor Node Nb,Value Could't Convert To Int"));
                return false;
            }
            if (nb > 0)
            {
                data.addinginfo.insert(counter_id, nb);
            }
        }
    }
    return true;
}
//解析Mold信息
bool XMLPaser::PaserMoldInfo(QDomElement& mold, MoldInfo& data,QString* err)
{
    if(!mold.hasChildNodes())
    {//没有子节点数据
        SAFE_SET(err, QObject::tr("Paser Root.Machine.MoldNode, No Child"));
        return false;
    }
    bool res = false;
    data.id = mold.attribute(Attr_Other_ID);
    if(data.id.isEmpty())
    {
        SAFE_SET(err, QObject::tr("Paser Root.Machine.Mold attribute id,Value Could't be empty"));
        return false;
    }

    QDomNodeList mold_children = mold.childNodes();
    QMap<QString, bool> chkflg;
    chkflg.insert(Node_Inspected, false);
    chkflg.insert(Node_Rejects, false);
    chkflg.insert(Node_Defects, false);
    chkflg.insert(Node_Autoreject, false);
    chkflg.insert(Node_Sensor, false);
    for(int i = 0; i < mold_children.length();i++)
    {
        QDomElement t = mold_children.at(i).toElement();
        res = false;
        if (t.tagName() == Node_Inspected)
        {
            data.inspected = t.text().toInt(&res);
            if(!res)
            {
                SAFE_SET(err, QObject::tr("Paser Root.Machine.Mold Node Inspected,Value Could't Convert To Int"));
                return false;
            }
        }
        else if(t.tagName() == Node_Rejects)
        {
            data.rejects = t.text().toInt(&res);
            if(!res)
            {
                SAFE_SET(err, QObject::tr("Paser Root.Machine.Mold Node Inspected,Value Could't Convert To Int"));
                return false;
            }
        }
        else if(t.tagName() == Node_Defects)
        {
            data.defects = t.text().toInt(&res);
            if(!res)
            {
                SAFE_SET(err, QObject::tr("Paser Root.Machine.Mold Node Defects,Value Could't Convert To Int"));
                return false;
            }
        }
        else if(t.tagName() == Node_Autoreject)
        {
            data.autorejects = t.text().toInt(&res);
            if(!res)
            {
                SAFE_SET(err, QObject::tr("Paser Root.Machine.Mold Node Autoreject,Value Could't Convert To Int"));
                return false;
            }
        }
        else if(t.tagName() == Node_Sensor)
        {
            SensorInfo info;
            res = PaserSensorInfo(t, info, err);
            if(!res)
            {
                return false;
            }
            data.sensorinfo.push_back(info);
        }
    }
    return true;
}

//解析设备节点
bool XMLPaser::PaserMachineInfo(QDomElement& root, XmlData& data,QString* err)
{//默认只会存在一个节点

    try
    {
        data.dt_end = QDateTime::currentDateTime();

        if(!root.hasChildNodes())
        {//没有子节点数据
            SAFE_SET(err, QObject::tr("Paser XML data,Root Node, No Child"));
            return false;
        }
        QDomElement mach = root.firstChildElement(Node_Mach);
        if(mach.isNull())
        {//没有此节点
            SAFE_SET(err, QObject::tr("Paser XML data,Root.Machine Node, No Machine Node"));
            return false;
        }
        data.id = mach.attribute(Attr_Mach_ID);
        if(!mach.hasChildNodes())
        {
            SAFE_SET(err, QObject::tr("Paser XML data,Root Node No Called Machine child Node"));
            return true;
        }
        QDomNodeList mach_children = mach.childNodes();
        QMap<QString, bool> chkflg;
        chkflg.insert(Node_Inspected, false);
        chkflg.insert(Node_Rejects, false);
        chkflg.insert(Node_Defects, false);
        chkflg.insert(Node_Autoreject, false);
        chkflg.insert(Node_Mold, false);
        for(int i = 0; i < mach_children.length();i++)
        {
            QDomElement t = mach_children.at(i).toElement();
            bool res = false;
            if (t.tagName() == Node_Inspected)
            {
                data.inspected = t.text().toInt(&res);
                if(!res)
                {
                    SAFE_SET(err, QObject::tr("Paser Root.Machine Node Inspected,Value Could't Convert To Int"));
                    return false;
                }
                chkflg.insert(Node_Inspected, true);
            }
            else if(t.tagName() == Node_Rejects)
            {
                data.rejects = t.text().toInt(&res);
                if(!res)
                {
                    SAFE_SET(err, QObject::tr("Paser Root.Machine Node Inspected,Value Could't Convert To Int"));
                    return false;
                }
                chkflg.insert(Node_Rejects, true);
            }
            else if(t.tagName() == Node_Defects)
            {
                data.defects = t.text().toInt(&res);
                if(!res)
                {
                    SAFE_SET(err, QObject::tr("Paser Root.Machine Node Defects,Value Could't Convert To Int"));
                    return false;
                }
                chkflg.insert(Node_Defects, true);
            }
            else if(t.tagName() == Node_Autoreject)
            {
                data.autorejects = t.text().toInt(&res);
                if(!res)
                {
                    SAFE_SET(err, QObject::tr("Paser Root.Machine Node Autoreject,Value Could't Convert To Int"));
                    return false;
                }
                chkflg.insert(Node_Autoreject, true);
            }
            else if(t.tagName() == Node_Mold)
            {
                MoldInfo info;
                res = PaserMoldInfo(t, info, err);
                if(!res)
                {
                    return false;
                }
                data.moldinfo.push_back(info);
                chkflg.insert(Node_Mold, true);
            }
        }

        //检查必要节点或熟悉是否已经被解析

        return true;
    }catch(std::exception& e)
    {
        ELOGE("parse xml data exception:%s", e.what());
    }
    catch(...)
    {
    
    }
    return false;
    
}

bool XMLPaser::PaserInfo(const QString& xmlstr, XmlData& data, QString* err)
{
    QDomDocument dom ;
    if(!dom.setContent(xmlstr, err))
    {
        return false;
    }
    QDomElement root = dom.documentElement();
    if(root.tagName() != Node_Root)
    {
        SAFE_SET(err, QObject::tr("XML data the top tag isn't Root,the xml data has incorrect format"));
        return false;
    }
    if(!XMLPaser::PaserMachineInfo(root, data, err)) 
    {//解析数据出错
        return false;
    }
    return true;
}

bool XMLPaserV2::PaserMachineInfo(QDomElement& root, XmlData& data, QString* err)
{
    try
    {
        data.dt_end = QDateTime::currentDateTime();

        if (!root.hasChildNodes())
        {//没有子节点数据
            SAFE_SET(err, QObject::tr("Paser XML data,string Node, No Child"));
            return false;
        }

        QDomElement Mold = root.firstChildElement(Node_Mold);
        if (Mold.isNull())
        {//没有此节点
            SAFE_SET(err, QObject::tr("Paser XML data,string.Mold Node, No Mold Node"));
            return false;
        }
        QString moldID = Mold.attribute(Attr_Other_ID);
        if (moldID.isEmpty())
        {
            SAFE_SET(err, QObject::tr("Paser XML data,string.Mold id is empty"));
            return false;
        }

        QDomElement mach = Mold.firstChildElement(Node_Mach);
        if (mach.isNull())
        {//没有此节点
            SAFE_SET(err, QObject::tr("Paser XML data,string.Mold.Machine Node, No Machine Node"));
            return false;
        }
        data.id = mach.attribute(Attr_Other_ID);
        if (data.id.isEmpty())
        {
            SAFE_SET(err, QObject::tr("Paser XML data,string.Mold id is empty"));
            return true;
        }
        if (!mach.hasChildNodes())
        {
            SAFE_SET(err, QObject::tr("Paser XML data,string.Mold Node No Called Machine child Node"));
            return true;
        }
        QDomNodeList mach_children = mach.childNodes();
        QMap<QString, bool> chkflg;
        chkflg.insert(Node_Inspected, false);
        chkflg.insert(Node_Rejects, false);
        chkflg.insert(Node_Defects, false);
        chkflg.insert(Node_Autoreject, false);
        chkflg.insert(Node_Mold, true);
        MoldInfo moldinfo;
        moldinfo.id = moldID;
        for (int i = 0; i < mach_children.length(); i++)
        {//only one, if more, the last active.
            QDomElement t = mach_children.at(i).toElement();
            bool res = false;
            if (t.tagName() == Node_Inspected)
            {
                data.inspected = t.text().toInt(&res);
                if (!res)
                {
                    SAFE_SET(err, QObject::tr("Paser string.Mold.Machine Node Inspected,Value Could't Convert To Int"));
                    return false;
                }
                chkflg.insert(Node_Inspected, true);
            }
            else if (t.tagName() == Node_Rejects)
            {
                data.rejects = t.text().toInt(&res);
                if (!res)
                {
                    SAFE_SET(err, QObject::tr("Paser string.Mold.Machine Node Inspected,Value Could't Convert To Int"));
                    return false;
                }
                chkflg.insert(Node_Rejects, true);
            }
            else if (t.tagName() == Node_Defects)
            {
                data.defects = t.text().toInt(&res);
                if (!res)
                {
                    SAFE_SET(err, QObject::tr("Paser string.Mold.Machine Node Defects,Value Could't Convert To Int"));
                    return false;
                }
                chkflg.insert(Node_Defects, true);
            }
            else if (t.tagName() == Node_Sensor)
            {
                SensorInfo info;
                res = XMLPaser::PaserSensorInfo(t, info, err);
                if (!res)
                {
                    return false;
                }
                if (info.rejects > 0)
                {
                    moldinfo.sensorinfo.push_back(info);
                }
            }
        }
        if (data.inspected > 0)
        {
            moldinfo.inspected = data.inspected;
            moldinfo.rejects = data.rejects;
            moldinfo.defects = data.defects;
            moldinfo.autorejects = data.autorejects;
            data.moldinfo.push_back(moldinfo);
            chkflg.insert(Node_Mold, true);
        }

        //检查必要节点或熟悉是否已经被解析

        return true;
    }
    catch (std::exception& e)
    {
        ELOGE("parse xml data exception:%s", e.what());
    }
    catch (...)
    {

    }
    return false;
}

bool XMLPaserV2::PaserInfo(const QString& xmlstr, XmlData& data, QString* err /*= nullptr*/)
{
    QDomDocument dom;
    if (!dom.setContent(xmlstr, err))
    {
        return false;
    }
    QDomElement root = dom.documentElement();
    if (root.tagName() != Node_Root_V2)
    {
        SAFE_SET(err, QObject::tr("XML data the top tag isn't string,the xml data has incorrect format"));
        return false;
    }
    if (!XMLPaserV2::PaserMachineInfo(root, data, err))
    {//解析数据出错
        return false;
    }
    return true;
}
