ֱ�������ϼ�����Ĺ��ػ��ϴ�CountData.exe����


��;����ʾ����˵��:
Config.ini�ļ�:
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

������ʾ:
strategy.mode: ȱ�ݴ�������ģʽ:
*	true��ʾ������ģʽ,��˼�Ǵ�ģʽ�·���sensorids���ȱ��ID�ᱻ����;
*	false��ʾ������ģʽ,��˼�Ǵ�ģʽ�·���sensorids���ȱ��ID���ᱻ����
strategy.islogall:true��ʾ�������ĵ���־���¼���н�����xml�ַ���.

system. time_interval_generate_record:���ɼ�¼���ݵ�ʱ����,
*	0:30����,
*	1:60����,
*	2:90����
*	3:120����
system. time_interval_get_src_data:ͨ��webservice��ȡ���ݵ�ʱ���� ��λ��
system.data_outdate_days:ԭʼ���ݹ���ʱ��,��λ��
system.language:0����,1Ӣ��
system.code_file_prefix:ʹ�õĴ���(����ģ������ȱ����)�Ĵ�����Ӧ�ļ���ǰ׺
�ļ���������Ŀ¼�µ��ļ���,��ʾ������,�����ļ���ΪCodeTs_en.ini,������ʾ,����MoldWords�µ���ģ������Ӧ�Ĵ���,SensorWords�¶�Ӧ����ȱ�����Ӧ�Ĵ���.
ʾ���ļ�:

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
