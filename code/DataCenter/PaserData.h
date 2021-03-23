#ifndef PASER_RXY62PZ297UNKQRI_H_
#define PASER_RXY62PZ297UNKQRI_H_

#include "DataDef.hpp"
#include <QDomElement>

/* XMLPaser ���ڽ�����WebService���صĹ�������XML�ַ������࣬ʾ���ַ������ڿ��ͷ�ļ���˵��ע����
* 
//����Ϊʾ��XML�ַ������޻��е�ԭ�ַ���
//<Root xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns="">
//    <Machine Id="MX">
//    <!-- �������� -->
//    <Inspected>1</Inspected>
//    <!-- ���޷��ź��� -->
//    <Rejects>1</Rejects>
//    <!-- ʵ���޷��� -->
//    <Defects>1</Defects>
//    <!-- �źŲ�ֵ�� -->
//    <Autoreject>0</Autoreject>
//    <!-- ģ�� 0-99 �����ж�� -->
//    <Mold id="0">
//    <!-- �������� | ���޷��ź��� | ʵ���޷��� | �źŲ�ֵ�� -->
//    <Inspected>1</Inspected>
//    <Rejects>1</Rejects>
//    <Defects>1</Defects>
//    <Autoreject>0</Autoreject>
//
//    <!-- ȱ�ݴ��� | ������ | ʵ���޷ϸ��� | ��������-->
//    <Sensor id="40">
//    <Rejects>0</Rejects>
//    <Defects>0</Defects>
//    </Sensor>
//    <Sensor id="41">
//    <Rejects>1</Rejects>
//    <Defects>1</Defects>
//    <Counter id="1" Nb="1" />
//    </Sensor>
//    <Sensor id="42">
//    <Rejects>0</Rejects>
//    <Defects>0</Defects>
//    </Sensor>
//    <Sensor id="46">
//    <Rejects>1</Rejects>
//    <Defects>1</Defects>
//    <Counter id="1" Nb="1" />
//    <Counter id="2" Nb="1" />
//    </Sensor>
//    </Mold>
//    </Machine>
//    </Root>
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

/**XMLPaserV2 ���ڽ���protocolV2������
 * 
 * <?xml version="1.0" encoding="UTF-8"?>
<string xmlns="http://www.tiama-inspection.com/">
    <Mold id="0">
        <Machine id="MCAL">
            <Inspected>320</Inspected>
            <Rejects>16</Rejects>
            <Defects>16</Defects>
            <Sensor id="Sidewall">
                <Rejects>6</Rejects>
                <Defects>6</Defects>
                <Counter id="Sidewall-other" Nb="0"/>
                <Counter id="Stone" Nb="0"/>
                <Counter id="Blister" Nb="0"/>
                <Counter id="Drawn" Nb="5"/>
                <Counter id="Side object" Nb="0"/>
                <Counter id="Density" Nb="0"/>
                <Counter id="Thin" Nb="0"/>
                <Counter id="Bird" Nb="0"/>
                <Counter id="Wing" Nb="1"/>
                <Counter id="Error" Nb="0"/>
                <Counter id="Black Image" Nb="0"/>
            </Sensor>
            <Sensor id="Low contrast">
                <Rejects>9</Rejects>
                <Defects>9</Defects>
                <Counter id="Transition" Nb="0"/>
                <Counter id="Deformation" Nb="4"/>
                <Counter id="No learning parameters" Nb="0"/>
                <Counter id="Error" Nb="5"/>
                <Counter id="Black image" Nb="0"/>
            </Sensor>
            <Sensor id="Stress">
                <Rejects>0</Rejects>
                <Defects>0</Defects>
                <Counter id="Stress" Nb="0"/>
                <Counter id="Error" Nb="0"/>
                <Counter id="Black image" Nb="0"/>
            </Sensor>
            <Sensor id="Dim">
                <Rejects>2</Rejects>
                <Defects>2</Defects>
                <Counter id="Verticality" Nb="2"/>
                <Counter id="Error" Nb="0"/>
                <Counter id="Diameter" Nb="0"/>
                <Counter id="Height" Nb="0"/>
            </Sensor>
        </Machine>
    </Mold>
</string>
 */
class XMLPaserV2:public XMLPaser {
public:
    //�����豸�ڵ�
    static bool PaserMachineInfo(QDomElement& root, XmlData& data, QString* err);
    //����XML����
    static bool PaserInfo(const QString& xmlstr, XmlData& data, QString* err = nullptr);

public:
    //XmlData tempdata;   //���һ�ν�������ʱ����
};

#endif//PASER_RXY62PZ297UNKQRI_H_
