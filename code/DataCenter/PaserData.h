#ifndef PASER_RXY62PZ297UNKQRI_H_
#define PASER_RXY62PZ297UNKQRI_H_

#include "DataDef.hpp"
#include <QDomElement>

/* XMLPaser ���ڽ�����WebService���صĹ�������XML�ַ������࣬ʾ���ַ������ڿ��ͷ�ļ���˵��ע����
*
*/

class XMLPaser{
public:
    //����Sensorȱ����Ϣ
    static bool PaserSensorInfo(QDomElement& sensor, SensorInfo& data,QString* err);

    //����Mold��Ϣ
    static bool PaserMoldInfo(QDomElement& mold, MoldInfo& data,QString* err);

    //�����豸�ڵ�
    static bool PaserMachineInfo(QDomElement& root, XmlData& data,QString* err);
    //����XML����
    static bool PaserInfo(const QString& xmlstr, XmlData& data,QString* err = nullptr);

public:
    //XmlData tempdata;   //���һ�ν�������ʱ����
};

#endif//PASER_RXY62PZ297UNKQRI_H_
