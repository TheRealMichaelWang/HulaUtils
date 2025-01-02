#include "HulaUtils.hpp"
#include <sstream>
#include <cctype>

using namespace HulaUtils;

void write_json(HulaScript::instance::value& current, std::stringstream& ss, HulaScript::instance& instance, int indent = -1, std::optional<std::string> json_property = std::nullopt) {
	for (int i = 0; i < indent; i++) { ss << '\t'; }
	if (json_property.has_value()) {
		ss << '\"' << json_property.value() << "\" : ";
	}

	if (current.check_type(HulaScript::instance::value::vtype::BOOLEAN)) {
		ss << current.boolean(instance) ? "true" : "false";
		return;
	}
	else if (current.check_type(HulaScript::instance::value::vtype::NIL)) {
		ss << "null";
		return;
	}
	else if (current.check_type(HulaScript::instance::value::vtype::DOUBLE)) {
		ss << current.number(instance);
		return;
	}
	else if (current.check_type(HulaScript::instance::value::vtype::RATIONAL)) {
		ss << instance.rational_to_string(current, false) << 'r';
		return;
	}
	else if (current.check_type(HulaScript::instance::value::vtype::STRING)) {
		ss << '\"';
		for (auto c : current.str(instance)) {
			switch (c)
			{
			case '\"':
				ss << '\\\"';
				break;
			case '\'':
				ss << '\\\'';
				break;
			case '\t':
				ss << '\\t';
				break;
			case '\n':
				ss << '\\n';
				break;
			default:
				ss << c;
				break;
			}
		}
		ss << '\"';
		return;
	}
	else if (current.check_type(HulaScript::instance::value::vtype::TABLE)) {
		HulaScript::ffi_table_helper helper(current, instance);

		if (helper.is_array()) {
			ss << '[';
			for (size_t i = 0; i < helper.get_size(); i++) {
				if (i > 0) {
					ss << ',';
				}
				if (indent >= 0) {
					ss << '\n';
					auto elem = helper.get(instance.rational_integer(i));
					write_json(elem, ss, instance, indent + 1);
				}
				else {
					auto elem = helper.get(instance.rational_integer(i));
					write_json(elem, ss, instance, -1);
				}
			}
			if (indent >= 0) {
				ss << '\n';
				for (int i = 0; i < indent; i++) { ss << '\t'; }
			}
			ss << ']';
			return;
		}
		auto key_value = helper.get(std::string("@json_keys"));
		if(!key_value.check_type(HulaScript::instance::value::vtype::NIL)) {
			HulaScript::ffi_table_helper key_table_helper(key_value, instance);
			if (key_table_helper.is_array()) {
				std::vector<std::string> keys;
				std::vector<HulaScript::instance::value> key_strs;
				keys.reserve(key_table_helper.get_size() + 1);
				for (size_t i = 0; i < key_table_helper.get_size(); i++) {
					keys.push_back(key_table_helper.get(instance.rational_integer(i)).str(instance));
					key_strs.push_back(instance.make_string(keys.back()));
				}

				if (!helper.get(std::string("@json_constructor")).check_type(HulaScript::instance::value::vtype::NIL)) {
					keys.push_back("@json_constructor");
				}

				ss << '{';
				for (std::string& key : keys) {
					if (key != keys.front()) {
						ss << ',';
					}
					if (indent >= 0) {
						ss << '\n';
						auto elem = helper.get(key);
						write_json(elem, ss, instance, indent + 1, key);
					}
					else {
						auto elem = helper.get(key);
						write_json(elem, ss, instance, -1, key);
					}
				}
				if (keys.size() > 0) {
					ss << ',';
				}
				int keys_property_indent = -1;
				if (indent >= 0) {
					ss << '\n';
					keys_property_indent = indent + 1;
				}
				auto key_array_value = instance.make_array(key_strs);
				write_json(key_array_value, ss, instance, keys_property_indent, "@json_keys");

				if (indent >= 0) {
					ss << '\n';
					for (int i = 0; i < indent; i++) { ss << '\t'; }
				}
				ss << '}';
				return;
			}
			else {
				instance.panic("@json_keys property must be an array.");
			}
		}
	}
	std::string json_source = instance.invoke_method(current, "toJSON", {}).str(instance);
	ss << json_source;
}

DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL HulaUtils::JSONParser(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	return instance.add_foreign_object(std::make_unique<json_parser>(json_parser()));
}

DYNALO_EXPORT HulaScript::instance::value DYNALO_CALL HulaUtils::toJSON(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	bool allow_newline = false;
	if (args.size() >= 2) {
		allow_newline = args.at(1).boolean(instance);
	}
	else {
		HULASCRIPT_EXPECT_ARGS(1);
	}

	std::stringstream ss;
	write_json(args.at(0), ss, instance, allow_newline ? 0 : -1);

	return instance.make_string(ss.str());
}

HulaScript::instance::value HulaUtils::json_parser::add_constructor(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(3);

	size_t name_hash = HulaScript::Hash::dj2b(args.at(0).str(instance).c_str());

	HulaScript::ffi_table_helper table_helper(args.at(2), instance);
	std::vector<std::string> arguments;
	arguments.reserve(table_helper.get_size());
	for (size_t i = 0; i < table_helper.get_size(); i++) {
		arguments.push_back(table_helper.get(instance.rational_integer(i)).str(instance));
	}

	return HulaScript::instance::value(object_parsers.insert({ name_hash, std::make_pair(args.at(1), arguments) }).second);
}

class json_scanner {
private:
	std::string source;
	int position;

	HulaScript::instance& instance;
	std::unordered_map<size_t, std::pair<HulaScript::instance::value, std::vector<std::string>>>& object_parsers;

public:
	json_scanner(std::string source, std::unordered_map<size_t, std::pair<HulaScript::instance::value, std::vector<std::string>>>& object_parsers, HulaScript::instance& instance) : source(source), position(0), object_parsers(object_parsers), instance(instance) {

	}

	char scan_char() noexcept {
		if (position == source.size()) {
			return EOF;
		}
		return source.at(position++);
	}

	char peek_char() const noexcept {
		if (position == source.size()) {
			return EOF;
		}
		return source.at(position);
	}

	void match_char(char expected) const;
	void consume_whitespace() noexcept;

	char parse_char_literal();
	HulaScript::instance::value parse_json();
};

void json_scanner::match_char(char expected) const
{
	if (peek_char() != expected) {
		std::stringstream ss;
		ss << "JSON Parser: Expected char \'" << expected << "\' but got char \'" << (peek_char() == EOF ? '0' : peek_char()) << "\' instead.";
		instance.panic(ss.str());
	}
}

void json_scanner::consume_whitespace() noexcept
{
	while (std::isblank(peek_char()) && peek_char() != EOF) {
		scan_char();
	}
}

char json_scanner::parse_char_literal()
{
	char c = scan_char();
	if (c == '\\') {
		c = scan_char();
		switch (c)
		{
		case '\"':
			return '\"';
		case '\'':
			return '\'';
		case 't':
			return '\t';
		case 'n':
			return '\n';
		default: {
			std::stringstream ss;
			ss << "Unexpected char \'" << c << "\' in \\ control sequence.";
			instance.panic(ss.str());
			break;
		}
		}
	}
	return c;
}

HulaScript::instance::value json_scanner::parse_json()
{
	consume_whitespace();

	char peeked = peek_char();
	if (std::isdigit(peeked)) {
		std::string num_str;
		
		do {
			num_str.push_back(scan_char());
		} while (std::isdigit(peek_char()) || peek_char() == '.');

		if (peek_char() == 'r') {
			scan_char();
			return instance.parse_rational(num_str);
		}
		else {
			return HulaScript::instance::value(std::stod(num_str));
		}
	}
	else if (peeked == '\"') {
		scan_char();

		std::string str;
		while (peek_char() != '\"') {
			str.push_back(parse_char_literal());
		}
		scan_char();
		return instance.make_string(str);
	}
	else if (peeked == '[') {
		scan_char();
		
		std::vector<HulaScript::instance::value> elements;
		bool first = true;
		consume_whitespace();
		while (peek_char() != ']') {
			if (first) {
				first = false;
			}
			else {
				consume_whitespace();
				match_char(',');
				scan_char();
			}
			elements.push_back(parse_json());
			consume_whitespace();
		}
		scan_char();
		return instance.make_array(elements);
	}
	else if (peeked == '{') {
		scan_char();

		std::vector<std::pair<HulaScript::instance::value, HulaScript::instance::value>> elements;
		bool first = true;
		consume_whitespace();
		while (peek_char() != '}') {
			if (first) {
				first = false;
			}
			else {
				consume_whitespace();
				match_char(',');
				scan_char();
			}

			auto key = parse_json();
			instance.temp_gc_protect(key);
			consume_whitespace();
			match_char(':');
			scan_char();
			auto value = parse_json();
			consume_whitespace();
			instance.temp_gc_protect(value);

			elements.push_back(std::make_pair(key, value));
		}
		scan_char();

		HulaScript::ffi_table_helper helper(elements.size(), instance);
		for (auto& key_value_pair : elements) {
			helper.emplace(key_value_pair.first, key_value_pair.second);
			instance.temp_gc_unprotect();
			instance.temp_gc_unprotect();
		}

		if (!helper.get(std::string("@json_constructor")).check_type(HulaScript::instance::value::vtype::NIL)) {
			std::string constructor_name = helper.get(std::string("@json_constructor")).str(instance);
			
			auto it = object_parsers.find(HulaScript::Hash::dj2b(constructor_name.c_str()));
			if (it == object_parsers.end()) {
				std::stringstream ss;
				ss << "Json Parse Error: Invalid @json_constructor \"" << constructor_name << "\".";
				instance.panic(ss.str());
			}

			std::vector<HulaScript::instance::value> arguments;
			arguments.reserve(it->second.second.size());
			for (std::string& key : it->second.second) {
				arguments.push_back(helper.get(key));
			}
			return instance.invoke_value(it->second.first, arguments);
		}
		return helper.get_table();
	}

	std::stringstream ss;
	ss << "Json Parse Error: Unexpected char \'" << peeked << "\'.";
	instance.panic(ss.str());
	return HulaScript::instance::value();
}

HulaScript::instance::value HulaUtils::json_parser::parse_json(std::vector<HulaScript::instance::value>& args, HulaScript::instance& instance)
{
	HULASCRIPT_EXPECT_ARGS(1);

	json_scanner scanner(args.at(0).str(instance), object_parsers, instance);
	return scanner.parse_json();
}