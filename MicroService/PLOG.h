#ifndef PLOG_H
#define PLOG_H
#include "spdlog/common.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/logger.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include <type_traits>
#include <xstring>
class PLOG
{
public:
	PLOG(bool init);
	~PLOG();
	template <typename ...Args>
	void PLOGWARN(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		if (log_task)
			my_logger_->warn(fmt, std::forward<Args>(args)...);
	}
	template <typename ...Args>
	void PLOGINFOR(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		if (log_task)
			my_logger_->info(fmt, std::forward<Args>(args)...);
	}
	template <typename ...Args>
	void PLOGError(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		if (log_task)
			my_logger_->error(fmt, std::forward<Args>(args)...);
	}
	std::shared_ptr<spdlog::logger> my_logger_;
	static void initLogging(bool init);
	static std::shared_ptr<PLOG> logs;
	bool log_task = false;
};


#endif // !PLOG_H



