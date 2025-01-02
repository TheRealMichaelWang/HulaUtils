// HulaUtils.cpp : Defines the entry point for the application.
//

#include <sstream>
#include <string>
#include <filesystem>
#include "HulaUtils.hpp"

using namespace HulaUtils;

DYNALO_EXPORT const char** DYNALO_CALL HulaUtils::manifest(HulaScript::instance::foreign_object* foreign_obj) {
	static const char* my_functions[] = {
		"openFile",
		"dirInfo",
		"dirTraverse",
		"rem",
		"remAll",
		"runCommand",
		"JSONParser",
		"toJSON",
		"currentTime",
		NULL
	};

	HulaScript::library_owner = foreign_obj;
	return my_functions;
}

DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL HulaUtils::openFile(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(2);

	FILE* infile = std::fopen(args[0].str(instance).c_str(), args[1].str(instance).c_str());
	if (infile == NULL) {
		return HulaScript::instance::value();
	}

	return instance.add_foreign_object(std::make_unique<file_object>(infile));
}

DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL HulaUtils::dirInfo(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(1);

	std::vector<HulaScript::instance::value> file_list;
	std::vector<HulaScript::instance::value> dir_list;
	for (auto const& entry : std::filesystem::directory_iterator(std::filesystem::path(args[0].str(instance)))) {
		auto name = instance.make_string(entry.path().string());
		if (entry.is_directory()) {
			dir_list.push_back(name);
		} else {
			file_list.push_back(name);
		}
	}

	return instance.make_table_obj({
		std::make_pair("subDirs", instance.make_array(dir_list)),
		std::make_pair("files", instance.make_array(file_list))
	});
}

DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL HulaUtils::dirTraverse(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(3);

	for (auto const& entry : std::filesystem::recursive_directory_iterator(std::filesystem::path(args[0].str(instance)))) {
		auto name = instance.make_string(entry.path().string());
		
		if (entry.is_directory()) {
			instance.invoke_value(args[1], { name });
		}
		else {
			instance.invoke_value(args[2], { name });
		}
	}

	return HulaScript::instance::value();
}

DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL HulaUtils::rem(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(1);
	return HulaScript::instance::value(std::filesystem::remove(args[0].str(instance)));
}

DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL HulaUtils::remAll(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(1);
	return instance.rational_integer(std::filesystem::remove_all(args[0].str(instance)));
}

DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL HulaUtils::runCommand(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	std::string cmd;
	for (auto& arg : args) {
		cmd.append(instance.get_value_print_string(arg));
	}

	std::system(cmd.c_str());
	return HulaScript::instance::value();
}

HulaScript::instance::value HulaUtils::file_object::read_line(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(0);
	if (infile == NULL) {
		instance.panic("File handle object is closed.");
		return HulaScript::instance::value(); //unreachable
	}

	std::string line;
	for(;;)
	{
		int c = std::fgetc(infile);
		if (c == EOF || c == '\n') {
			break;
		}
		line.push_back(c);
	}
	return instance.make_string(line);
}

HulaScript::instance::value HulaUtils::file_object::read_all_lines(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(0);
	if (infile == NULL) {
		instance.panic("File handle object is closed.");
		return HulaScript::instance::value(); //unreachable
	}

	std::vector<HulaScript::instance::value> lines;

	std::string line;
	for(;;)
	{
		int c = std::fgetc(infile);
		if (c == EOF) {
			break;
		}
		else if (c == '\n') {
			lines.push_back(instance.make_string(line));
			line.clear();
		}
		else {
			line.push_back(c);
		}
	}
	lines.push_back(instance.make_string(line));

	return instance.make_array(lines);
}

HulaScript::instance::value HulaUtils::file_object::read_to_end(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(0);
	if (infile == NULL) {
		instance.panic("File handle object is closed.");
		return HulaScript::instance::value(); //unreachable
	}

	std::string line;
	for(;;)
	{
		int c = std::fgetc(infile);
		if (c == EOF) {
			break;
		}
		line.push_back(c);
	}
	return instance.make_string(line);
}

HulaScript::instance::value HulaUtils::file_object::write(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(1);
	if (infile == NULL) {
		instance.panic("File handle object is closed.");
		return HulaScript::instance::value(); //unreachable
	}
	
	std::string str = instance.get_value_print_string(args[0]);
	
	bool success = std::fwrite(str.c_str(), sizeof(char), str.size(), infile) == str.size();
	
	return HulaScript::instance::value(success);
}

HulaScript::instance::value HulaUtils::file_object::write_line(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(1);
	if (infile == NULL) {
		instance.panic("File handle object is closed.");
		return HulaScript::instance::value(); //unreachable
	}

	std::string str = instance.get_value_print_string(args[0]);

	bool success = std::fwrite(str.c_str(), sizeof(char), str.size(), infile) == str.size();
	if (success) {
		fputc('\n', infile);
	}

	return HulaScript::instance::value(success);
}

HulaScript::instance::value HulaUtils::file_object::close(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(0);
	if (infile == NULL) {
		instance.panic("File handle object is closed.");
		return HulaScript::instance::value(); //unreachable
	}
	std::fclose(infile);
	infile = NULL;

	return HulaScript::instance::value();
}