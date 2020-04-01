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

    bool runflg ;				//���б�־ ���ڿ����߳��˳�

    ETimeInterval eti;			//ʱ���� ��

	SystemConfig cfgSystem;		
	QSharedPointer<Shift> shift;//��μ������
};
#endif//GenerateRecord_JQQ8207II27VSQ97_H_

