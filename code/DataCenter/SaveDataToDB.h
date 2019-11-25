#ifndef SAVEDATATODB_JQQ8207II27VSQ97_H_
#define SAVEDATATODB_JQQ8207II27VSQ97_H_

#include <QObject>
#include "DataDef.hpp"

#include <QList>
#include <QDateTime>

/************************************************************************/
/* �������ݵ����ݿ⣬����������߼��ǻ���ݲ����ļ��Ĳ��Դ��ָ����Χ������     */
/************************************************************************/
class SaveDataToDB{
public:
    /**
    * @brief  :  Init ���ݿ��ʼ���������ݿ��ļ�������ʱ���ᴴ���ļ���Ȼ�󴴽���
    *                   ���������ļ�
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool Init( QString* err = nullptr);

    /**
    * @brief  :  ParserStrategyFile ���������ļ���ÿ��ִ��ǰ���һ������ļ��иĶ����ٽ���һ��
    *
    * @param  :  bool first
    * @return :  void
    * @retval :
    */
    void ParserStrategyFile(bool first = false);

    bool SaveStrategyData(QString* err = nullptr);

    bool checkSensorIDIsShouldSave(int id);

    /**
    * @brief  :  SaveData �������ݵ����ݿ⣬ÿ�ζ����´����ݿ⣬��Ϊ����Ƶ�ʺ�С
    *
    * @param  :  const XmlData & data
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool SaveData(const XmlData& data, QString* err = nullptr);

private:
    //�����������
    bool strategy_mode; //����ģʽ true��ʾ ������ false��ʾ������
    QList<int> sensorids; //����Ӱ���ȱ��ID�б�
    QDateTime  lastmodify; //�����ļ�������޸�ʱ��

public:
    bool m_islogevery_data;
};

#endif//SAVEDATATODB_JQQ8207II27VSQ97_H_

