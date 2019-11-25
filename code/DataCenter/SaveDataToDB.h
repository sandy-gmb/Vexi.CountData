#ifndef SAVEDATATODB_JQQ8207II27VSQ97_H_
#define SAVEDATATODB_JQQ8207II27VSQ97_H_

#include <QObject>
#include "DataDef.hpp"

#include <QList>
#include <QDateTime>

/************************************************************************/
/* 保存数据到数据库，此类的隐藏逻辑是会根据策略文件的策略存放指定范围的数据     */
/************************************************************************/
class SaveDataToDB{
public:
    /**
    * @brief  :  Init 数据库初始化，当数据库文件不存在时，会创建文件，然后创建表
    *                   解析策略文件
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool Init( QString* err = nullptr);

    /**
    * @brief  :  ParserStrategyFile 解析策略文件，每次执行前检查一遍如果文件有改动则再解析一遍
    *
    * @param  :  bool first
    * @return :  void
    * @retval :
    */
    void ParserStrategyFile(bool first = false);

    bool SaveStrategyData(QString* err = nullptr);

    bool checkSensorIDIsShouldSave(int id);

    /**
    * @brief  :  SaveData 保存数据到数据库，每次都重新打开数据库，因为操作频率很小
    *
    * @param  :  const XmlData & data
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool SaveData(const XmlData& data, QString* err = nullptr);

private:
    //保存策略数据
    bool strategy_mode; //策略模式 true表示 白名单 false表示黑名单
    QList<int> sensorids; //策略影响的缺陷ID列表
    QDateTime  lastmodify; //策略文件的最后修改时间

public:
    bool m_islogevery_data;
};

#endif//SAVEDATATODB_JQQ8207II27VSQ97_H_

