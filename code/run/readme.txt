�ļ��ܹ�:
Win32:����32λϵͳ
x64:����64λϵͳ
Debug:���԰汾
Release:������

ע:��ǰ���� ��ʹ�ö�Ӧϵͳ��Debug�汾�ĳ�������.

# ʹ��˵��:
1. �˳��������ϼ�����������,����Ҫ�ŵ����ϼ����������Ŀ¼;
2. �˳�����Ҫ�ŵ����ϼ�����Ĺ��ػ���;
3. ʹ��ʱ��CountData.exe����

# ��.����ʾ����˵��:
```
Config.ini�ļ�:
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
 
������ʾΪ����ʾ��, ����Ϊ��������˵��:
## ����ȱ����Ϣ����
1. strategy.mode: ȱ�ݴ�������ģʽ:
* true��ʾ������ģʽ,��˼�Ǵ�ģʽ�·���sensorids���ȱ��ID�ᱻ����;
* false��ʾ������ģʽ,��˼�Ǵ�ģʽ�·���sensorids���ȱ��ID���ᱻ����
2. sensorids����Ϊģʽ���е�ID�б�,�ɲο���ʾ��

## ϵͳ����
1. system.time_interval_generate_record:ˢ�¼�¼���ݵ�ʱ���� ��λ��,
2. system.time_interval_get_src_data:ˢ��ԭʼ���ݵ�ʱ����(ͨ��webservice��ȡ���ݵ�ʱ����) ��λ��
3. system.data_outdate_days:ԭʼ���ݹ���ʱ��,��λ��
4. system.show_record_type:��������ʾ�ļ�¼���� 0:��ʱ������ͳ�Ƽ�¼ 1:����ε�ͳ�Ƽ�¼
5. system.language:0����,1Ӣ��
6. system.code_file_prefix:ʹ�õĴ���(����ģ������ȱ����)�Ĵ�����Ӧ�ļ���ǰ׺
�ļ���������Ŀ¼�µ��ļ���,��ʾ������,�����ļ���ΪCodeTs_en.ini,������ʾ,����MoldWords�µ���ģ������Ӧ�Ĵ���,SensorWords�¶�Ӧ����ȱ�����Ӧ�Ĵ���.
ʾ���ļ�(Ӣ���ļ���������):
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
## ʱ��������
1. timeInterval.time_interval:���ɼ�¼���ݵ�ʱ����,
    * 0:30����,
    * 1:60����,
    * 2:90����
    * 3:120����

## �����Ϣ����
1. shift���鶨���θ�����ÿ������л�ʱ���.

## ������Ϣ����
1. debug.isreadsrcdata:ʵʱ������Դ�Ƿ�ʹ��Դ�����ļ�(����Ŀ¼�µ�srcdata.txt),true��ʾʹ�ã�����ļ�����������Ȼʹ��webserviceͨ��
2. debug.issavesrcdata:�Ƿ񱣴�ͨ�����ݵ�Դ�����ļ���������Դ�������ļ����򲻻ᱣ�棩
3. debug.loglevel:��־��Ϣ�������ͼ��� 0:Trace, 1:Debug, 2:Info, 3:Error, 4:Alarm, 5:Fatal
