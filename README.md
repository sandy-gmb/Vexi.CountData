# Vexi.CountData
request counts data from vexi app, save and show.
support English,Chinese.

## IDE
* QT addlns + vs2010
* QT 4.8.5
* win7


## Program architecture
at present, these are three module:
* 1 the main app: request data;
* 2 the saved library: parser data and save in sqlite database;
* 3 the ui library: according data, show them in ui.

# 更新日志
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
