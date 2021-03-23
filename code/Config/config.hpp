/********************************************************************
* @filename  : config.hpp
*
* @brief     : 用于本软件的配置管理,可提供给其他模块 读取和写入
*
* @detail    :当前包含以下配置:               
*              1 解析保存元数据的过滤规则
*             2 系统运行的其他配置(特别的时间间隔)
*             3 模板名,缺陷码指定的词条对应文件前缀
*             4 当前语言版本等
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
    //获取相关配置的槽函数

    //获取数据中心相关配置
    /**
    * @brief GetDataCenterConf 获取数据中心配置
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
    * @brief GetCoreConf
    *
    * @param CoreConf & cfg
    * @return:   void
    */
    void GetSystemConf(SystemConfig& cfg);
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

    //配置更新函数
    //获取词条翻译文件路径
    /**
    * @brief GetWordsTranslation
    *
    * @param const QMap<int
    * @param QString> & moldwors
    * @param const QMap<int
    * @param QString> & sensorwors
    * @return:   void
    */
    void GetWordsTranslation(QMap<QString, QString>& moldwors, QMap<QString, QString>& sensorwors);
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
