#ifndef CONFIG_DEFINE_HH_
#define CONFIG_DEFINE_HH_

#pragma once

#include <QList>

class DataCenterConf
{
public:
    //��������
    bool bStrategyMode;                 //����ģʽ true��ʾ ������ false��ʾ������
    QList<int> lSensorIDs;               //����Ӱ���ȱ��ID�б�
    int iTimeInterval_GeneRecord;       //ʱ����,�����ڶ೤ʱ������һ��ͳ�Ƽ�¼ 1:60���� 2:90����;3:120���� ����:30����
    int iDaysDataOutDate;               //ԭʼ������Ч���� ��λ:��,������ʱ��,���ݿ��ɾ��
    bool bIsLogEveryData;               //�Ƿ���־��¼��������
};

//int iTimeInterval_GetSrcData;       //��ȡԭʼ���ݵ�ʱ���� ��λ:��

//int iLanguage;                       //���� 0:���� 1:Ӣ�� �漰��������ʾ���Ժ�ʹ�õĴ�����Ӧ�ļ���
//QString sCodetsprefix;              //�����ļ�ǰ׺ ������ΪError,��������Ϊ0,��ʵ��ʹ���ļ�ΪError_zh.ini

#endif//CONFIG_DEFINE_HH_
