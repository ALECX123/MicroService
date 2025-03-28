#include "PLOG.h"
std::shared_ptr<PLOG> PLOG::logs;
PLOG::PLOG(bool init)
{
	//同一个文件只能实例化一个
	log_task = init;
	if (init)
	{
		my_logger_ = spdlog::rotating_logger_mt("MicroService", "C://logs/MicroService.txt", 1048576 * 5, 3);
		my_logger_->flush_on(spdlog::level::warn);
		my_logger_->flush_on(spdlog::level::err);
		my_logger_->flush_on(spdlog::level::critical);
		//每10秒刷新一次
		spdlog::flush_every(std::chrono::seconds(10));
		my_logger_->info("MicroService Start");
	}
}
PLOG::~PLOG()
{
	if (log_task)
		my_logger_.reset();
}

void PLOG::initLogging(bool init)
{
	if (!logs)
	{
		logs = std::shared_ptr<PLOG>(new PLOG(init));
	}
}