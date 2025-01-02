#include "HulaUtils.hpp"
#include <sstream>

using namespace HulaUtils;

HulaScript::instance::value date_time_obj::get_day(HulaScript::instance& instance)
{
	auto dt = localtime(&timestamp);
	return instance.rational_integer(dt->tm_mday);
}

HulaScript::instance::value date_time_obj::get_month(HulaScript::instance& instance)
{
	auto dt = localtime(&timestamp);
	return instance.rational_integer(dt->tm_mon + 1);
}

HulaScript::instance::value date_time_obj::get_year(HulaScript::instance& instance)
{
	auto dt = localtime(&timestamp);
	return instance.rational_integer(dt->tm_year + 1900);
}

HulaScript::instance::value date_time_obj::get_hour(HulaScript::instance& instance)
{
	auto dt = localtime(&timestamp);
	return instance.rational_integer(dt->tm_hour);
}

HulaScript::instance::value date_time_obj::get_minute(HulaScript::instance& instance)
{
	auto dt = localtime(&timestamp);
	return instance.rational_integer(dt->tm_min);
}

HulaScript::instance::value date_time_obj::get_second(HulaScript::instance& instance)
{
	auto dt = localtime(&timestamp);
	return instance.rational_integer(dt->tm_sec);
}

std::string HulaUtils::date_time_obj::to_string() 
{
	std::stringstream ss;
	auto dt = localtime(&timestamp);
	
	ss << (dt->tm_mon + 1) << '/' << dt->tm_mday << '/' << (dt->tm_year + 1900) << ' ' << dt->tm_hour << ':' << dt->tm_min << ':' << dt->tm_sec;

	return ss.str();
}

DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL HulaUtils::currentTime(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	time_t now;
	time(&now);

	return instance.add_foreign_object(std::make_unique<date_time_obj>(now));
}