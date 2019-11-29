#ifndef CORE_HH_
#define CORE_HH_

#include <QThread>

class Core : public QThread
{
    Q_OBJECT
public:
    Core(QObject* parent = nullptr);
    ~Core();

    public slots:
    void run();

    void stop();

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
public:

    class Impl;
    Impl * pimpl;


    int timeinterval;

    bool isrun;
};

#endif//CORE_HH_