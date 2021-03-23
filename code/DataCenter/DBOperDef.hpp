#ifndef DB_OPER_DEFINE_HH_
#define DB_OPER_DEFINE_HH_

#pragma once

#include <QString>

static const QString& strategyfile = QObject::tr("./Record/strategy.xml");

static const QString& db_path = QObject::tr("./Record/");
static const QString& db_filename = QObject::tr("Record.db3");
static const QString& rtdb_filename = QObject::tr("RTRecord.db3");
static const QString& shiftdb_filename = QObject::tr("ShiftRecord.db3");
//实时数据库|自己的数据库 表名 主表|模表|缺陷表|附加信息表
static const QString& tb_Main = QObject::tr("T_Main");
static const QString& tb_Mold = QObject::tr("T_Mold");
static const QString& tb_Sensor = QObject::tr("T_Sensor");
static const QString& tb_SensorAdd = QObject::tr("T_SensorAdd");


static const QString& create_tb_Main_SQL = QString("CREATE TABLE IF NOT EXISTS T_Main (\
                                            ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                            MachineID    VARCHAR,\
                                            Inspected    INTEGER,\
                                            Rejects      INTEGER,\
                                            Defects      INTEGER,\
                                            Autoreject   INTEGER,\
                                            [TimeStart] DATETIME,\
                                            [TimeEnd]   DATETIME );");
static const QString& create_tb_Mold_SQL = QString("CREATE TABLE IF NOT EXISTS T_Mold (\
                                            ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                            TMainRowID   INTEGER,\
                                            MoldID       VARCHAR,\
                                            Inspected    INTEGER,\
                                            Rejects      INTEGER,\
                                            Defects      INTEGER,\
                                            Autoreject   INTEGER,\
                                            FOREIGN KEY (\
                                            TMainRowID\
                                            )\
                                            REFERENCES T_Main (ID) ON DELETE CASCADE);");
static const QString& create_tb_Sensor_SQL = QString("CREATE TABLE IF NOT EXISTS T_Sensor (\
                                              ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                              TMoldRowID   INTEGER,\
                                              SensorID     VARCHAR,\
                                              Rejects      INTEGER,\
                                              Defects      INTEGER ,\
                                              FOREIGN KEY (\
                                              TMoldRowID\
                                              )\
                                              REFERENCES T_Mold (ID) ON DELETE CASCADE);");
static const QString& create_tb_SensorAdd_SQL = QString("CREATE TABLE IF NOT EXISTS T_SensorAdd (\
                                                 ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                                 TSensorRowID INTEGER,\
                                                 CounterID    VARCHAR,\
                                                 Nb           INTEGER ,\
                                                 FOREIGN KEY (\
                                                 TSensorRowID\
                                                 )\
                                                 REFERENCES T_Sensor (ID) ON DELETE CASCADE);");

static const QString& create_tb_ShiftMain_SQL = QString("CREATE TABLE IF NOT EXISTS T_Main (\
                                                   ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                                   MachineID    VARCHAR,\
                                                   Inspected    INTEGER,\
                                                   Rejects      INTEGER,\
                                                   Defects      INTEGER,\
                                                   Autoreject   INTEGER,\
                                                   [TimeStart] DATETIME,\
                                                   [TimeEnd]   DATETIME,\
                                                   Date VARCHAR,\
                                                   Shift   INTEGER );");

enum EOperDatabase
{
    EOperDB_RuntimeDB,
    EOperDB_TimeIntervalDB,
    EOperDB_ShiftDB,
};


#endif //DB_OPER_DEFINE_HH_