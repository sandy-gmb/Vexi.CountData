#include "SaveDataToDB.h"

#include <QString>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>

#include <QDomDocument>

const QString& strategyfile = QObject::tr("./Record/strategy.xml");

const QString& db_path = QObject::tr("./Record/");
const QString& db_filename = QObject::tr("record.db3");
//���� ����|ģ��|ȱ�ݱ�|������Ϣ��
const QString& tb_Main = QObject::tr("T_Main");
const QString& tb_Mold = QObject::tr("T_Mold");
const QString& tb_Sensor = QObject::tr("T_Sensor");
const QString& tb_SensorAdd = QObject::tr("T_SensorAdd");

const QString& create_tb_Main_SQL = QString("CREATE TABLE IF NOT EXISTS T_Main (\
                                            ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                            MachineID    VARCHAR,\
                                            Inspected    INTEGER,\
                                            Rejects      INTEGER,\
                                            Defects      INTEGER,\
                                            Autoreject   INTEGER,\
                                            [TimeStart] DATETIME,\
                                            [TimeEnd]   DATETIME );");
const QString& create_tb_Mold_SQL = QString("CREATE TABLE IF NOT EXISTS T_Mold (\
                                            ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                            TMainRowID   INTEGER,\
                                            MoldID       INTEGER,\
                                            Inspected    INTEGER,\
                                            Rejects      INTEGER,\
                                            Defects      INTEGER,\
                                            Autoreject   INTEGER );");
const QString& create_tb_Sensor_SQL = QString("CREATE TABLE IF NOT EXISTS T_Sensor (\
                                              ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                              TMoldRowID   INTEGER,\
                                              SensorID     INTEGER,\
                                              Rejects      INTEGER,\
                                              Defects      INTEGER );");
const QString& create_tb_SensorAdd_SQL = QString("CREATE TABLE IF NOT EXISTS T_SensorAdd (\
                                               ID           INTEGER  PRIMARY KEY AUTOINCREMENT,\
                                               TSensorRowID INTEGER,\
                                               CounterID    INTEGER,\
                                               Nb           VARCHAR );");

bool SaveDataToDB::Init(QString* err)
{
    m_islogevery_data = false;
    {
        //��ʼ�����ݿ�
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "record_db");
        QFile f(db_path+db_filename);
        if(!f.exists())
        {//û�д�ʱ���Զ������ļ�����˴�ʱ����Ҫִ�д�����䱣֤�������ݱ��Ѵ���
            QDir d ;
            if(!d.exists(db_path))
            {
                bool t = d.mkpath(db_path);
            }
            db.setDatabaseName(db_path+db_filename);
            if(!db.open())
            {
                SAFE_SET(err, QString(QObject::tr("open record database failure,the erro is ")+db.lastError().text())) ;
                return false;
            }
            //�½����ļ� ִ��һ�����ݱ�Ĵ������
            QSqlQuery query(db);
            bool res = true;
            do{
                res = query.exec(create_tb_Main_SQL);
                if(!res)
                {
                    break;
                }
                res = query.exec(create_tb_Mold_SQL);
                if(!res)
                {
                    break;
                }
                res = query.exec(create_tb_Sensor_SQL);
                if(!res)
                {
                    break;
                }
                res = query.exec(create_tb_SensorAdd_SQL);
                if(!res)
                {
                    break;
                }
            } while (false);
            if(!res)
            {
                SAFE_SET(err, QString(QObject::tr("Initial the database failure, the erro is ")+ query.lastError().text())) ;
                db.close();
                return false;
            }
        }
        else{//�����
            db.setDatabaseName(db_path+db_filename);
            if(!db.open())
            {
                SAFE_SET(err, QString(QObject::tr("open record database failure,the erro is ")+db.lastError().text())) ;
                return false;
            }
        }
        db.close();
    }

    QSqlDatabase::removeDatabase("record_db");
    
    return true;
}

bool SaveDataToDB::SaveData(const XmlData& data, QString* err)
{
    {
        bool res = true;

        //�����ݿ�
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "record_db");
        db.setDatabaseName(db_path+db_filename);
        if(!db.open())
        {
            SAFE_SET(err, QString(QObject::tr("open record database failure,the erro is ")+db.lastError().text())) ;
            return false;
        }
        if(!db.transaction())
        {//��������ʧ��
            SAFE_SET(err, QString(QObject::tr("Start Transaction Error ")+db.lastError().text())) ;
            db.close();
            return false;
        }

        do
        {//�����������
            //�½����ļ� ִ��һ�����ݱ�Ĵ������
            QSqlQuery query(db);

            //���ݲ��Ծ������������
            QString sql = QString("INSERT INTO %1(MachineID, Inspected, Rejects, Defects, Autoreject, TimeStart, TimeEnd) \
                                  Values('%2', %3, %4, %5, %6, '%7', '%8');").arg(tb_Main).arg(data.id).arg(data.inspected).arg(data.rejects).arg(data.defects)
                                  .arg(data.autorejects).arg(data.dt_start.toString("yyyy-MM-dd hh:mm:ss")).arg(data.dt_end.toString("yyyy-MM-dd hh:mm:ss"));

            //�����������ݣ�����ѯ�����Ӧ��IDֵ
            if(!query.exec(sql))
            {
                SAFE_SET(err, QString(QObject::tr("Save Machine Data Error, the Error is ")+query.lastError().text())) ;
                res = false;
                break;
            }
            //��ѯ�����Ӧ��IDֵ��������IDֵ���
            if(!query.exec(QString("SELECT max(ID) as res from %1").arg(tb_Main)))
            {
                SAFE_SET(err, QString(QObject::tr("Query Main Data ID Error: ")+query.lastError().text())) ;
                res = false;
                break;
            }
            //�����ܳ��ֵ����� 
            if(!query.next() || query.isNull(0))
            {
                SAFE_SET(err, QString(QObject::tr("query in a exception, no main data")+query.lastError().text())) ;
                res = false;
                break;
            }
            int mainrowid = query.value(0).toInt();

            foreach(MoldInfo mold, data.moldinfo)
            {
                //ѭ������ÿ��ģ����Ϣ���ݣ�����ѯÿ��ģ���Ӧ��IDֵ
                sql = QString("INSERT INTO %1(TMainRowID, MoldID, Inspected, Rejects, Defects, Autoreject) \
                              VALUES(%2, %3, %4, %5, %6, %7);").arg(tb_Mold).arg(mainrowid).arg(mold.id)
                              .arg(mold.inspected).arg(mold.rejects).arg(mold.defects).arg(mold.autorejects);
                if(!query.exec(sql))
                {
                    SAFE_SET(err, QString(QObject::tr("Save Mold Data Error, the Error is ")+query.lastError().text())) ;
                    res = false;
                    break;
                }
                //��ѯģ����Ӧ��IDֵ��������IDֵ���
                if(!query.exec(QString("SELECT max(ID) as res from %1").arg(tb_Mold)))
                {
                    SAFE_SET(err, QString(QObject::tr("Query Mold Data ID Error: ")+query.lastError().text())) ;
                    res = false;
                    break;
                }
                //�����ܳ��ֵ����� 
                if(!query.next() || query.isNull(0))
                {
                    SAFE_SET(err, QString(QObject::tr("query in a exception, no mold data")+query.lastError().text())) ;
                    res = false;
                    break;
                }
                int moldrowid = query.value(0).toInt();

                foreach(SensorInfo sensor, mold.sensorinfo)
                {
                    if(!checkSensorIDIsShouldSave(sensor.id))
                    {//���ݲ��ԣ�ȱ�����ݲ��ñ���
                        continue;
                    }
                    //ѭ�����浥��ģ�������ȱ����Ϣ���ݣ�����¼ÿ��ȱ�����ݵ�ID
                    sql = QString("INSERT INTO %1(TMoldRowID, SensorID, Rejects, Defects) \
                                  VALUES(%2, %3, %4, %5);").arg(tb_Sensor).arg(moldrowid).arg(sensor.id).arg(sensor.rejects)
                                  .arg(sensor.defects);
                    if (!query.exec(sql))
                    {
                        SAFE_SET(err, QString(QObject::tr("Save Sensor Data Error, the Error is ")+query.lastError().text())) ;
                        res = false;
                        break;
                    }
                    //��ѯģ����Ӧ��IDֵ��������IDֵ���
                    if(!query.exec(QString("SELECT max(ID) as res from %1").arg(tb_Sensor)))
                    {
                        SAFE_SET(err, QString(QObject::tr("Query Sensor Data ID Error: ")+query.lastError().text())) ;
                        res = false;
                        break;
                    }
                    //�����ܳ��ֵ����� 
                    if(!query.next() || query.isNull(0))
                    {
                        SAFE_SET(err, QString(QObject::tr("query in a exception, no sensor data")+query.lastError().text())) ;
                        res = false;
                        break;
                    }
                    int sensorrowid = query.value(0).toInt();

                    foreach(SensorAddingInfo adding, sensor.addinginfo)
                    {
                        //ѭ�����浥��ȱ�ݵĸ�����Ϣ����
                        sql = QString("INSERT INTO %1(TSensorRowID, CounterID, Nb) VALUES(%2,%3,%4);")
                            .arg(tb_SensorAdd).arg(sensorrowid).arg(adding.counter_id).arg(adding.nb);
                        if(!query.exec(sql))
                        {
                            SAFE_SET(err, QString(QObject::tr("Save SensorAdd Data Error, the Error is ")+query.lastError().text())) ;
                            res = false;
                            break;
                        }
                    }
                    if(!res)
                    {
                        break;
                    }
                }
                if(!res)
                {
                    break;
                }
            }
            if(!res)
            {
                break;
            }

            db.commit();
            db.close();
        }while(false); 

        if(!res)
        {
            db.rollback();
            db.close();
            return false;
        }
    }
    
    QSqlDatabase::removeDatabase("record_db");
    ParserStrategyFile(true);
    return true;
}

void SaveDataToDB::ParserStrategyFile(bool first)
{
    if(first && !QFile::exists(strategyfile))
    {
        strategy_mode = true;
        sensorids.push_back(10);    //��ʯ
        sensorids.push_back(34);    //С��ʯ

        SaveStrategyData();
        lastmodify = QDateTime::currentDateTime();
        return;
    }

    QDomDocument doc("strategyfile");
    QFile file(strategyfile);
    if (!file.open(QIODevice::ReadOnly))
    {//�ļ���ʧ�ܣ�����ǵ�һ�� ������ΪĬ������ ���򲻹�
        if(first)
        {
            strategy_mode = true;
            sensorids.push_back(10);    //��ʯ
            sensorids.push_back(34);    //С��ʯ

            SaveStrategyData();
            lastmodify = QDateTime::currentDateTime();
            return;
        }
        return;
    }
    
    QFileInfo info(file);
    QDateTime t = info.lastModified();
    if(!first)
    {
        if (t.secsTo(lastmodify)>0)
        {//�ļ��޸Ķ�
            return;
        }
    }
    lastmodify = t;
    //����ΪXML�ļ���׼��ʽ
    ///*<strategy>
    //    <!--����ģʽ true��ʾ������ģʽ ������ʾ������-->
    //    <mode>true</mode>
    //    <sensor>
    //    <id>10</id>
    //    <id>34</id>
    //    </sensor>
    //    <islogall>true</islogall>
    //    </strategy>*/

    //���������ļ�
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    // ����XML�ļ�
    QDomElement root = doc.documentElement();
    if(root.tagName() != "strategy")
    {
        //"XML data the top tag isn't strategy,the xml data has incorrect format"
        return ;
    }
    if(!root.hasChildNodes())
    {//û���ӽڵ�����
        //"Paser XML data,Root Node, No Child";
        return ;
    }
    QDomElement mode = root.firstChildElement("mode");
    if(mode.isNull())
    {//û�д˽ڵ�
        //"Paser XML data,No Called mode child Node";
        return ;
    }
    strategy_mode = (mode.text() == "true")?true:false;
    QDomElement logall = root.firstChildElement("islogall");
    if(logall.isNull())
    {//û�д˽ڵ�
        m_islogevery_data = false;
    }
    else
    {
         m_islogevery_data = (logall.text() == "true")?true:false;
    }
   
    QDomElement sensor = root.firstChildElement("sensor");
    if(!sensor.hasChildNodes())
    {
        return;
    }
   QDomNodeList lst = sensor.childNodes();
   for(int i = 0; i < lst.length();i++)
   {
       sensorids.push_back(lst.at(i).toElement().text().toInt());
   }
}

bool SaveDataToDB::SaveStrategyData( QString* err /*= nullptr*/ )
{
    QFile file(strategyfile);
    if(!file.open(QIODevice::WriteOnly))
    {
        SAFE_SET(err, QObject::tr("write strategy data failure: open file failure"));
        return false; //�½��ļ���ʧ��
    }

    QDomDocument doc("strategydoc");

    QDomComment cm = doc.createComment(QObject::tr("mode: true for whitelist, the other for blacklist, the id in sensor is the id affected by mode"));
    doc.appendChild(cm);

    QDomElement root = doc.createElement("strategy");
    QDomElement mode = doc.createElement("mode");
    mode.appendChild(doc.createTextNode( strategy_mode?QObject::tr("true"):QObject::tr("false")));
    QDomElement sensor = doc.createElement("sensor");
    foreach(int id, sensorids)
    {
        QDomElement n_id = doc.createElement("id");
        n_id.appendChild(doc.createTextNode(QString::number(id)));
        sensor.appendChild(n_id);
    }
    root.appendChild(mode);
    root.appendChild(sensor);

    doc.appendChild(root);

    QTextStream stream(&file);
    stream.setCodec("UTF-8"); //ʹ��utf-8��ʽ

    doc.save(stream, 4, QDomNode::EncodingFromTextStream);
    file.close();
    return true;
}

bool SaveDataToDB::checkSensorIDIsShouldSave( int id )
{
    if(strategy_mode)
    {//������
        foreach(int i, sensorids)
        {
            if(i == id)
            {
                return true;
            }
        }
        return false;
    }
    else
    {//������
        foreach(int i, sensorids)
        {
            if(i == id)
            {
                return false;
            }
        }
        return true;
    }
}


