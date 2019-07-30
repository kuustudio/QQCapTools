//logger.h
/*
//类名：CLogger
//功能介绍：Win平台日志记录功能，多线程安全，支持写日志级别的设置，日志格式包含日志等级，日志时间，文件名，行号信息
//作者：sunflover 2016-1-15 14:31:27

//使用方法：
1：将logger.h，logger.cpp添加到项目中
2：设置logger.cpp的预编译头选项为“不使用预编译头”
3：使用代码示例：
#include "Logger.h"
LOGGER::CLogger logger;

void main()
{
logger.TraceFatal("TraceFatal %d", 1);
logger.TraceError("TraceError %s", "sun");
logger.TraceWarning("TraceWarning");
logger.TraceInfo("TraceInfo");

logger.ChangeLogLevel(LOGGER::LogLevel_Error);

logger.TraceFatal("TraceFatal %d", 2);
logger.TraceError("TraceError %s", "sun2");
logger.TraceWarning("TraceWarning");
logger.TraceInfo("TraceInfo");
}

执行结果：20160115142829.log文件内容如下
Fatal	2016-01-15 14:28:29 logger.cpp:91	TraceFatal 1
Error	2016-01-15 14:28:29 logger.cpp:123	TraceError sun
Warning	2016-01-15 14:28:29 logger.cpp:155	TraceWarning
Info	2016-01-15 14:28:29 logger.cpp:188	TraceInfo
Fatal	2016-01-15 14:28:29 logger.cpp:91	TraceFatal 2
Error	2016-01-15 14:28:29 logger.cpp:123	TraceError sun2
*/

#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <Windows.h>
#include <stdio.h>
#include <string>

namespace LOGGER
{
	//日志级别的提示信息
	static const std::string strFatalPrefix = "Fatal\t";
	static const std::string strErrorPrefix = "Error\t";
	static const std::string strWarningPrefix = "Warning\t";
	static const std::string strInfoPrefix = "Info\t";
	static const std::string strDataPrefix = "Data\t";

	//日志级别枚举
	typedef enum EnumLogLevel
	{
		LogLevel_Stop = 0,	//什么都不记录
		LogLevel_Fatal,		//只记录严重错误
		LogLevel_Error,		//记录严重错误，普通错误
		LogLevel_Warning,	//记录严重错误，普通错误，警告
		LogLevel_Info		//记录严重错误，普通错误，警告，提示信息(也就是全部记录)
	};

	class CLogger
	{
	public:
		//nLogLevel：日志记录的等级，可空
		//strLogPath：日志目录，可空
		//strLogName：日志名称，可空
		CLogger(EnumLogLevel nLogLevel = EnumLogLevel::LogLevel_Info, const std::string strLogPath = "", const std::string strLogName = "");
		//析构函数
		virtual ~CLogger();
	public:
		//写严重错误信息
		void TraceFatal(const char *lpcszFormat, ...);
		//写错误信息
		void TraceError(const char *lpcszFormat, ...);
		//写警告信息
		void TraceWarning(const char *lpcszFormat, ...);
		//写提示信息
		void TraceInfo(const char *lpcszFormat, ...);
		//写数据信息
		void TraceData(const char *lpcszHeader,const char* data,unsigned int len);
		//改变写日志级别
		void ChangeLogLevel(EnumLogLevel nLevel);
	private:
		//写文件操作
		void Trace(const std::string &strLog);
		//获取当前系统时间
		std::string GetTime();
		//获取程序运行路径
		std::string GetAppPathA();
		//格式化字符串
		std::string FormatString(const char *lpcszFormat, ...);
		//文件全路径得到文件名
		const char *path_file(const char *path, char splitter);
	private:
		//写日志文件流
		FILE * m_pFileStream;
		//写日志级别
		EnumLogLevel m_nLogLevel;
		//日志目录
		std::string m_strLogPath;
		//日志的名称
		std::string m_strLogName;
		//日志文件全路径
		std::string m_strLogFilePath;
		//线程同步的临界区变量
		CRITICAL_SECTION m_cs;
	};
}

#endif