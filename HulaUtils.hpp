// HulaUtils.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <cstdio>
#include <ctime>
#include "HulaScript.hpp"
#include "process.h"

#define DYNALO_EXPORT_SYMBOLS
#include "dynalo/symbol_helper.hpp"

// TODO: Reference additional headers your program requires here.

namespace HulaUtils {
	class file_object : public HulaScript::foreign_method_object<file_object> {
	private:
		FILE* infile;

		HulaScript::instance::value read_line(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
		HulaScript::instance::value read_all_lines(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
		HulaScript::instance::value read_to_end(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
		HulaScript::instance::value write(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
		HulaScript::instance::value write_line(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);

		HulaScript::instance::value close(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);

	public:
		file_object(FILE* infile) : infile(infile) {
			declare_method("readLine", &file_object::read_line);
			declare_method("readAllLines", &file_object::read_all_lines);
			declare_method("readToEnd", &file_object::read_to_end);
			declare_method("write", &file_object::write);
			declare_method("writeLine", &file_object::write_line);
			declare_method("close", &file_object::close);
		}

		~file_object() {
			if (infile == NULL) {
				return;
			}
			std::fclose(infile);
		}
	};

	class json_parser : public HulaScript::foreign_method_object<json_parser> {
	private:
		std::unordered_map<size_t, std::pair<HulaScript::instance::value, std::vector<std::string>>> object_parsers;

		HulaScript::instance::value add_constructor(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
		HulaScript::instance::value parse_json(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
	public:
		json_parser() {
			declare_method("addConstructor", &json_parser::add_constructor);
			declare_method("parseJSON", &json_parser::parse_json);
		}
	};

	DYNALO_EXPORT const char** DYNALO_CALL manifest(HulaScript::instance::foreign_object* foreign_obj);

	DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL openFile(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
	DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL dirInfo(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
	DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL dirTraverse(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);

	DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL rem(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
	DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL remAll(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);

	DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL runCmd(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);

	DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL JSONParser(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
	DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL toJSON(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);

	DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL localTime(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
	DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL gmTime(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance);
}