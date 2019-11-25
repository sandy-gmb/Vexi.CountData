/********************************************************************
* @filename  : PaserDataToDataBase.h
*
* @brief     : 本模块用于解析系统的webservice返回的XML字符串，并保存到数据库中
*	
* @author    : Mingbing.Guo
* @date      : 2019-11-12
* 
* @version   : V0.1

//以下为示例XML字符串，无换行的原字符串
//<Root xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns="">
//    <Machine Id="MX">
//    <!-- 过检总数 -->
//    <Inspected>1</Inspected>
//    <!-- 发剔废信号数 -->
//    <Rejects>1</Rejects>
//    <!-- 实际剔废数 -->
//    <Defects>1</Defects>
//    <!-- 信号差值数 -->
//    <Autoreject>0</Autoreject>
//    <!-- 模号 0-99 可能有多个 -->
//    <Mold id="0">
//    <!-- 过检总数 | 发剔废信号数 | 实际剔废数 | 信号差值数 -->
//    <Inspected>1</Inspected>
//    <Rejects>1</Rejects>
//    <Defects>1</Defects>
//    <Autoreject>0</Autoreject>
//
//    <!-- 缺陷代码 | 检测个数 | 实际剔废个数 | 其他变量-->
//    <Sensor id="40">
//    <Rejects>0</Rejects>
//    <Defects>0</Defects>
//    </Sensor>
//    <Sensor id="41">
//    <Rejects>1</Rejects>
//    <Defects>1</Defects>
//    <Counter id="1" Nb="1" />
//    </Sensor>
//    <Sensor id="42">
//    <Rejects>0</Rejects>
//    <Defects>0</Defects>
//    </Sensor>
//    <Sensor id="46">
//    <Rejects>1</Rejects>
//    <Defects>1</Defects>
//    <Counter id="1" Nb="1" />
//    <Counter id="2" Nb="1" />
//    </Sensor>
//    </Mold>
//    </Machine>
//    </Root>
*********************************************************************/
#ifndef PARSERDATATODBTABASE_20GCLXZDB2VN57N8_H_
#define PARSERDATATODBTABASE_20GCLXZDB2VN57N8_H_

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 PASERDATATODATABASE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// PASERDATATODATABASE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef PASERDATATODATABASE_EXPORTS
#define PASERDATATODATABASE_API __declspec(dllexport)
#else
#define PASERDATATODATABASE_API __declspec(dllimport)
#endif


#include <QString>
#include <QDateTime>


// 此类是从 PaserDataToDataBase.dll 导出的
/**
    此类用于将传入的XML数据保存到运行目录的./Data/Satistics.db3中
    表结构定义请参考文档 解析数据库表结构.xlsx
*/
class PASERDATATODATABASE_API CPaserDataToDataBase {
public:
	CPaserDataToDataBase(void);
	~CPaserDataToDataBase();

    /**
    * @brief  :  Init 初始化， 内部会检查数据库是否创建，如果没有创建则会创建数据库
    *
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool Init(QString* err = nullptr);

    /**
    * @brief  :  PaserDataToDataBase 将传入的XML格式的字符串解析后将数据保存到数据库中
    *
    * @param  :  const QString & xmldata XML格式的字符串，字符串格式见文件头注释部分
    * @param  :  QString * err
    * @return :  bool
    * @retval :
    */
    bool PaserDataToDataBase(const QString& xmldata, QString* err = nullptr);

private:
    class Impl;

    Impl* m_pimpl;
};

#endif//PARSERDATATODBTABASE_20GCLXZDB2VN57N8_H_
