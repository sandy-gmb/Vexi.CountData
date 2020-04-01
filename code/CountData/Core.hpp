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
    * @brief  :  PaserDataToDataBase �������XML��ʽ���ַ������������ݱ��浽���ݿ���
    *
    * @param  :  const QString & xmldata XML��ʽ���ַ������ַ�����ʽ���ļ�ͷע�Ͳ���
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool signal_PaserDataToDataBase(const QString& xmldata, QString* err);

	 //��ȡ ��ȡʹ������
	void signal_GetCoreConf(CoreConf& cfg);
public:

    class Impl;
    Impl * pimpl;


	int timeinterval;
	CoreConf cfg;

    bool isrun;
};

#endif//CORE_HH_