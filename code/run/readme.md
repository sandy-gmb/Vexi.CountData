# 目录
1. [文件架构](#文件架构)
2. [使用说明](#使用说明)
3. [配置示例及说明](#附.配置示例及说明)
    1. [过滤缺陷信息策略](#过滤缺陷信息策略) 
    2. [系统配置](#系统配置) 
    3. [时间间隔配置](#时间间隔配置) 
    4. [班次信息配置](#班次信息配置) 
    5. [调试信息配置](#调试信息配置) 
4. [更新日志](#更新日志)
    1. [Version:1.0.1.0](#Version:1.0.1.0) 
    2. [Version:1.0.0.3](#Version:1.0.0.3) 
    3. [Version:1.0.0.2](#Version:1.0.0.2) 
    4. [Version:1.0.0.1](#Version:1.0.0.1) 


# 文件架构:
* Win32:用于32位系统
* x64:用于64位系统
* Debug:调试版本
* Release:发布版


# 使用说明:
1. 此程序与联合检验机程序独立,不需要放到联合检验机的运行目录;
2. 此程序需要放到联合检验机的工控机上;
3. 使用时打开CountData.exe即可
4. 现场运行可根据系统使用32位或64位版本程序
5. 当前最新的稳定版本为1.0.1.0

# 附.配置示例及说明:
```
Config.ini文件:
[strategy]
mode=true
sensorids\size=2
sensorids\1\id=10
sensorids\2\id=34

[system]
time_interval_generate_record=5
time_interval_get_src_data=5
data_outdate_days=30
language=0
show_record_type=0
code_file_prefix=CodeTs

[timeInterval]
time_interval=5

[shift]
shift\size=1
shift\1\time=00:00:00

[debug]
is_read_source_data=false
is_save_source_data=false
log_level=2
```
 
如上所示为配置示例, 以下为各项配置说明:
## 过滤缺陷信息策略
1. strategy.mode: 缺陷代码命中模式:
    * true 表示白名单模式,意思是此模式下方的sensorids里的缺陷ID会被保存;
    * false 表示黑名单模式,意思是此模式下方的sensorids里的缺陷ID不会被保存
2. sensorids数组为模式命中的ID列表,可参考上示例

## 系统配置
1. system.time_interval_generate_record:刷新记录数据的时间间隔 单位秒,
2. system.time_interval_get_src_data:刷新原始数据的时间间隔(通过webservice获取数据的时间间隔) 单位秒
3. system.data_outdate_days:原始数据过期时间,单位天
4. system.show_record_type:主界面显示的记录类型 0:按时间间隔的统计记录 1:按班次的统计记录
5. system.language:0中文,1英文
6. system.code_file_prefix:使用的代码(包含模板代码和缺陷码)的词条对应文件名前缀
文件放在运行目录下的文件下,如示例配置,完整文件名为CodeTs_en.ini,如下所示,其中MoldWords下的是模板代码对应的词条,SensorWords下对应的是缺陷码对应的词条.
示例文件(英文文件内容如下):
```
[MoldWords]
0=Normal
1=Tailand
2=India

[SensorWords]
4=LNMC
12=BOC
16=LNM/LNMP
35=DIMBAG
40=PLUG
41=LEAK
42=CHECK
43=OVALITY
44=CSB
46=THICKNESS
47=NCI/HCI
49=ATLAS
54=EXTENSION1
55=EXTENSION2
110=COMPOSITE COUNTER
```
## 时间间隔配置
1. timeInterval.time_interval:生成记录数据的时间间隔,
    * 0:30分钟,
    * 1:60分钟,
    * 2:90分钟
    * 3:120分钟

## 班次信息配置
1. shift数组定义班次个数和每个班次切换时间点.

## 调试信息配置
1. debug.isreadsrcdata:实时数据来源是否使用源数据文件(运行目录下的srcdata.txt),true表示使用，如果文件不存在则依然使用webservice通信
2. debug.issavesrcdata:是否保存通信数据到源数据文件（数据来源是数据文件，则不会保存）
3. debug.loglevel:日志信息的输出最低级别 0:Trace, 1:Debug, 2:Info, 3:Error, 4:Alarm, 5:Fatal


# 更新日志
## Version:1.0.1.0
* Date: 2020-5-8
* Note:
    1. 由于都匀运行稳定,将proV:1.0.0.3[DataCenter:1.0.0.5 | DataView:1.0.0.3 | Config:1.0.0.2 ] 发布为1.0.1.0
    2. 现场运行可根据系统使用32位或64位版本程序
        
## Version:1.0.0.3
* Date: 2020-4-14
* Note:
    1. DataCenter->Version:1.0.0.5:
        * fix#1 界面查询bug,界面看不到数据;
        * fix#2 Sqlite使用bug,外键的级联删除未触发,导致子表存在很多垃圾数据的问题.
        * fix#3 计算班次的日期和班次bug
    2. DataView->Version:1.0.0.3:
        * fix#1 查询界面切换记录类型时查询未更新时间列表和table区域的bug
        * improve 按照Kau-hon的要求,界面显示的班次从1开始.
    
## Version:1.0.0.2
* Date: 2020-4-8
* Note:
    1. 增加按班次统计记录;
    2. 拆分副界面:拆分为查询界面和配置界面;
    3. 关键配置例如调试配置和策略配置,不提供界面配置,策略配置可以在界面显示
    4. 数据中心由于使用SQLite,修改了查询功能,使效率能更快;

## Version:1.0.0.1
* Date: 2019-12-09
* Note:
    1. 此版本为默认版本号,文件不附带版本信息;
    2. 使用WebService从联合检验机获取数据;
    3. 界面提供可设置的按时间间隔生成统计记录;
    4. readme提供配置文件设置说明 界面只提供时间间隔配置

[回到目录](#目录)