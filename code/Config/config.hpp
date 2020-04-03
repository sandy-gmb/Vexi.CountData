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
    /**
    * @brief GetDataCenterConf ��ȡ������������
    *
    * @param DataCenterConf & cfg
    * @return:   void
    */
    void GetDataCenterConf(DataCenterConf& cfg);
	/**
	* @brief GetCoreConf
	*
	* @param CoreConf & cfg
	* @return:   void
	*/
	void GetCoreConf(CoreConf& cfg);
	/**
	* @brief GetAllConfig
	*
	* @param AllConfig & cfg
	* @return:   void
	*/
	void GetAllConfig(AllConfig& cfg);
	void SetAllConfig(const AllConfig& cfg);

	/**
	* @brief GetConfigLogLevel
	*
	* @return:   int
	*/
	int GetConfigLogLevel();

	//���ø��º���
    //��ȡ���������ļ�·��
    /**
    * @brief GetWordsTranslation
    *
    * @param const QMap<int
    * @param QString> & moldwors
    * @param const QMap<int
    * @param QString> & sensorwors
    * @return:   void
    */
    void GetWordsTranslation(QMap<int, QString>& moldwors, QMap<int, QString>& sensorwors);
	/**
	* @brief GetTimeInterval
	*
	* @return:   int
	*/
	int GetTimeInterval();

signals:
	void LanguageChanged(int lang);
	void RecordShowChanged(int recordtype);
	void RecordConfigChanged();
	void CoreConfChange(const CoreConf& cfg);
	void DataConfChange(const DataCenterConf& cfg);

private:
    class Impl;
    Impl* pimpl;
};

#endif // CONFIG_HPP
