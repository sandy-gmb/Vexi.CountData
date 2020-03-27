/********************************************************************
* @filename  : config.hpp
*
* @brief     : ���ڱ���������ù���,���ṩ������ģ�� ��ȡ��д��
*
* @detail    :��ǰ������������:               
*	          1 ��������Ԫ���ݵĹ��˹���
*             2 ϵͳ���е���������(�ر��ʱ����)
*             3 ģ����,ȱ����ָ���Ĵ�����Ӧ�ļ�ǰ׺
*             4 ��ǰ���԰汾��
*
* @author    : GuoMingbing
* @date      : 2019/12/02
* 
* @version   : V0.1
*********************************************************************/
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "config_global.h"

#include <QObject>
#include "ConfigDef.hpp"

class CONFIG_EXPORT Config:public QObject
{
    Q_OBJECT
public:
    Config(QObject* parent = nullptr);
    ~Config();

    public slots:
    //��ȡ������õĲۺ���
    //��ȡ���������������
    void GetDataCenterConf(DataCenterConf& cfg);
    void SetGenerateRecordTimeInterval(int ti);

    //��ȡ�������
    int GetSoftwareLanguage();
    //��ȡ���������ļ�·��
    QString GetWordsTranslationFilePath();
    //��ȡ ��ȡԭʼ����ʱ���� ��λ��
    int GetTimeOfObtainSrcData();
	//��ȡcoreʹ�õ�����
	void GetCoreConf(CoreConf& cfg);
	int GetConfigLogLevel();

private:
    class Impl;
    Impl* pimpl;
};

#endif // CONFIG_HPP
