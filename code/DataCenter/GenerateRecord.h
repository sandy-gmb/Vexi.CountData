#ifndef GenerateRecord_JQQ8207II27VSQ97_H_
#define GenerateRecord_JQQ8207II27VSQ97_H_

#include <QObject>
#include "DataDef.hpp"

#include <QList>
#include <QDateTime>

#include <QSharedPointer>
#include <QMutex>
#include <QSqlDatabase>

class SaveDataToDB;

class GenerateRecord: public QObject
{
    Q_OBJECT
public:
    explicit GenerateRecord( QObject *dboper = nullptr) ;
    virtual ~GenerateRecord();
    
public slots:
	void work();
	bool DeleteRecordAfterTime(QDateTime time);

private:
    bool GetLastestRecordEndTime(QDateTime& t, QString* err);
    void CalcGenerateRecordTime(QDateTime start, ETimeInterval eti, QList<QDateTime>& lst);
    bool StatisticsRecord(QList<QDateTime>& tlst, QList<Record>& lst, QList<Record>& newlst, QString* err);
    bool GenerateRecordToDB(QList<Record>& newlst, QString* err);
    void DeleteOutdatedData(int outdatedays);
signals:
    void resultReady(const QString &result);
public:
	SaveDataToDB* m_pthis;

    bool runflg ;   //���б�־ ���ڿ����߳��˳�
    ETimeInterval eti;
    int outdatedays;
	
	QSharedPointer<QSqlDatabase> db;        //�����������ݿ�Ķ���,ʹ��һ������,��ֹ�ظ���
	QSharedPointer<QSqlDatabase> rtdb;      //����ʵʱ���ݿ�Ķ���,ʹ��һ������,��ֹ�ظ���
    QSharedPointer<QMutex>  rtdb_mutex;
    QSharedPointer<QMutex>  db_mutex;
};
#endif//GenerateRecord_JQQ8207II27VSQ97_H_

