#ifndef CORE_HH_
#define CORE_HH_

#include <QThread>
#include "ConfigDef.hpp"

class Core : public QThread
{
    Q_OBJECT
public:
    Core(QObject* parent = nullptr);
    ~Core();

public slots:
    void run();

    void stop();
	void OnCoreConfChange(const CoreConf& _cfg);

    signals:
        /**
    * @brief  :  PaserDataToDataBase 将传入的XML格式的字符串解析后将数据保存到数据库中
    *
    * @param  :  const QString & xmldata XML格式的字符串，字符串格式见文件头注释部分
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool signal_PaserDataToDataBase(const QString& xmldata, QString* err);

	 //获取 获取使用配置
	void signal_GetCoreConf(CoreConf& cfg);
public:

    class Impl;
    Impl * pimpl;


	int timeinterval;
	CoreConf cfg;

    bool isrun;
};

#endif//CORE_HH_