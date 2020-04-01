#ifndef GenerateRecord_JQQ8207II27VSQ97_H_
#define GenerateRecord_JQQ8207II27VSQ97_H_

#include <QObject>
#include "DataDef.hpp"

#include <QList>
#include <QDateTime>

#include <QSharedPointer>
#include <QMutex>
#include <QSqlDatabase>

#include "DBOperDef.hpp"
#include "ConfigDef.hpp"

class DBOperation;

class GenerateRecord: public QObject
{
    Q_OBJECT
public:
    explicit GenerateRecord( QObject *dboper = nullptr) ;
    virtual ~GenerateRecord();
    
public slots:
	void work();

private:
	bool processGenerTimeIntervalRecord();
	bool processGenerateShiftRecord();

    void CalcGenerateRecordTime(QDateTime start, ETimeInterval eti, QList<QDateTime>& lst);
    bool StatisticsRecord(QList<QDateTime>& tlst, QList<Record>& lst, QList<Record>& newlst, QString* err, bool isshiftrecord = false);

	void CalcGenerateShiftRecordTime(QDateTime start, QList<QDateTime>& lst);
signals:
    void resultReady(const QString &result);
public:
	DBOperation* m_pthis;

    bool runflg ;				//运行标志 用于控制线程退出

    ETimeInterval eti;			//时间间隔 分

	SystemConfig cfgSystem;		
	QSharedPointer<Shift> shift;//班次计算对象
};
#endif//GenerateRecord_JQQ8207II27VSQ97_H_

