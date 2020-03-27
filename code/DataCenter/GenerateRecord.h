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

    bool runflg ;   //运行标志 用于控制线程退出
    ETimeInterval eti;
    int outdatedays;
	
	QSharedPointer<QSqlDatabase> db;        //连接自身数据库的对象,使用一个对象,防止重复打开
	QSharedPointer<QSqlDatabase> rtdb;      //连接实时数据库的对象,使用一个对象,防止重复打开
    QSharedPointer<QMutex>  rtdb_mutex;
    QSharedPointer<QMutex>  db_mutex;
};
#endif//GenerateRecord_JQQ8207II27VSQ97_H_

