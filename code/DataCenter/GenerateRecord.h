#ifndef GenerateRecord_JQQ8207II27VSQ97_H_
#define GenerateRecord_JQQ8207II27VSQ97_H_

#include <QObject>
#include "DataDef.hpp"

#include <QList>
#include <QDateTime>

#include <QSharedPointer>
#include <QMutex>
#include <QSqlDatabase>

class GenerateRecord: public QObject
{
    Q_OBJECT
public:
    explicit GenerateRecord(QSharedPointer<QMutex> rt_mx,QSharedPointer<QMutex> mx, QObject *parent = nullptr) ;
    virtual ~GenerateRecord();
    
public slots:
    void work();

private:

    bool GetLastestRecord(QDateTime& t, QString* err);
    bool QueryRecordByTime(QDateTime st, QDateTime et,QList<Record>& lst, QString* err);
    void CalcGenerateRecordTime(QDateTime start, ETimeInterval eti, QList<QDateTime>& lst);
    bool StatisticsRecord(QList<QDateTime>& tlst, QList<Record>& lst, QList<Record>& newlst, QString* err);
    bool GenerateRecordToDB(QList<Record>& newlst, QString* err);
    void DeleteOutdatedData(int outdatedays);

signals:
    void resultReady(const QString &result);
public:
    bool runflg ;   //运行标志 用于控制线程退出
    ETimeInterval eti;
    int outdatedays;

    QSqlDatabase            db;
    QSharedPointer<QMutex>  rtdb_mutex;
    QSharedPointer<QMutex>  db_mutex;
};
#endif//GenerateRecord_JQQ8207II27VSQ97_H_

