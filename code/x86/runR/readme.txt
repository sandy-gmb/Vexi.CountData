直接在联合检验机的工控机上打开CountData.exe即可


附;配置示例及说明:
Config.ini文件:
[strategy]																
mode=true																
sensorids\size=0															
;sensorids\1\0=14															
;sensorids\2\1=34															
islogall=false																
																		
[system]																	
time_interval_generate_record=0											
time_interval_get_src_data=60												
data_outdate_days=30														
language=1																
code_file_prefix=CodeTs													

如上所示:
strategy.mode: 缺陷代码命中模式:
*	true表示白名单模式,意思是此模式下方的sensorids里的缺陷ID会被保存;
*	false表示黑名单模式,意思是此模式下方的sensorids里的缺陷ID不会被保存
strategy.islogall:true表示数据中心的日志会记录所有解析的xml字符串.

system. time_interval_generate_record:生成记录数据的时间间隔,
*	0:30分钟,
*	1:60分钟,
*	2:90分钟
*	3:120分钟
system. time_interval_get_src_data:通过webservice获取数据的时间间隔 单位秒
system.data_outdate_days:原始数据过期时间,单位天
system.language:0中文,1英文
system.code_file_prefix:使用的代码(包含模板代码和缺陷码)的词条对应文件名前缀
文件放在运行目录下的文件下,如示例配置,完整文件名为CodeTs_en.ini,如下所示,其中MoldWords下的是模板代码对应的词条,SensorWords下对应的是缺陷码对应的词条.
示例文件:

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
