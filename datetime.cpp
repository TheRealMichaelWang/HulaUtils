#include "HulaUtils.hpp"
#include <sstream>

using namespace HulaUtils;

static HulaScript::instance::value toDateTimeObject(struct tm* datetime, std::optional<time_t> timestamp, HulaScript::instance& instance) {
	std::vector<std::pair<std::string, HulaScript::instance::value>> properties;
	properties.push_back(std::make_pair("day", instance.rational_integer(datetime->tm_mday)));
	properties.push_back(std::make_pair("month", instance.rational_integer(datetime->tm_mon + 1)));
	properties.push_back(std::make_pair("year", instance.rational_integer(datetime->tm_year + 1900)));
	properties.push_back(std::make_pair("hour", instance.rational_integer(datetime->tm_hour)));
	properties.push_back(std::make_pair("min", instance.rational_integer(datetime->tm_min)));
	properties.push_back(std::make_pair("sec", instance.rational_integer(datetime->tm_sec)));
	
	if (timestamp.has_value()) {
		properties.push_back(std::make_pair("unix", instance.rational_integer(timestamp.value())));
	}

	return instance.make_table_obj(properties, true);
}

DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL HulaUtils::localTime(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	time_t now;
	if (args.size() == 1) {
		now = args.at(0).size(instance);
	}
	else {
		time(&now);
	}

	auto dt = localtime(&now);
	return toDateTimeObject(dt, now, instance);
}

DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL HulaUtils::gmTime(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	time_t now;
	if (args.size() == 1) {
		now = args.at(0).size(instance);
	}
	else {
		time(&now);
	}

	auto dt = gmtime(&now);
	return toDateTimeObject(dt, now, instance);
}
