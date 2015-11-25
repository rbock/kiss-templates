#include <ciso646>  // Make MSCV understand and/or/not
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "command_type.h"
#include "KisteTemplate.h"
#include "ClassTemplate.h"
#include "LineTemplate.h"
#include <kiste/cpp.h>

namespace kiste
{
  bool starts_with(const std::string& text, const std::string& start)
  {
    if (start.size() > text.size())
      return false;
    for (std::size_t i = 0; i < start.size(); ++i)
    {
      if (start[i] != text[i])
      {
        return false;
      }
    }
    return true;
  }

  struct command_t
  {
    std::size_t end_pos;
    command_type type;
    std::string text;
  };

  enum class line_type
  {
    none,
    cpp,
    text,
    command,
    class_begin,
    class_end,
    member,
  };

  struct member_t
  {
    std::string class_name;
    std::string name;
  };

  struct class_data_t
  {
    std::size_t curly_level = 0;
    std::string name;
    std::string parent_name;
  };

  struct line_t
  {
    std::size_t curly_level = 0;
    std::size_t class_curly_level = 0;
    line_type previous_type = line_type::none;
    line_type type = line_type::none;
    line_type next_type = line_type::none;
    bool trailing_return = false;
    // depending on the type, one of the following members is to be used
    std::vector<command_t> commands;
    class_data_t class_data;
    member_t member;

    line_t() = default;

    line_t(line_type t) : type(t)
    {
    }

    line_t(line_type t, const std::vector<command_t>& c) : type(t), commands(c)
    {
    }

    line_t(const class_data_t& data) : type(line_type::class_begin), class_data(data)
    {
    }

    line_t(const member_t& data) : type(line_type::member), member(data)
    {
    }
  };

  static const auto empty_line = line_t{};

  struct parse_context
  {
    std::istream& is;
    std::ostream& os;
    std::string filename;
    bool report_exceptions = false;
    std::string line;
    std::size_t line_no = 0;
    std::size_t curly_level = 0;
    std::size_t class_curly_level = 0;

    parse_context(std::istream& is_,
                  std::ostream& os_,
                  const std::string& filename_,
                  bool report_exceptions_)
        : is(is_), os(os_), filename{filename_}, report_exceptions{report_exceptions_}
    {
    }
  };

  struct parse_error : public std::runtime_error
  {
    std::string filename;
    std::string line;
    std::size_t line_no = 0;

    parse_error(const parse_context& ctx, const std::string& message)
        : std::runtime_error{message}, filename{ctx.filename}, line{ctx.line}, line_no{ctx.line_no}
    {
    }
  };

  auto parse_expression(const parse_context& ctx, command_type type, std::size_t pos) -> command_t
  {
    auto expression = std::string{};
    auto arg_curly_level = 1;

    for (; pos < ctx.line.size() and arg_curly_level; ++pos)
    {
      switch (ctx.line.at(pos))
      {
      case '{':
        ++arg_curly_level;
        expression.push_back(ctx.line.at(pos));
        break;
      case '}':
        --arg_curly_level;
        if (arg_curly_level)
        {
          expression.push_back(ctx.line.at(pos));
        }
        else
        {
          // do nothing, as this probably the closing curly brace of the command
        }
        break;
      default:
        expression.push_back(ctx.line.at(pos));
      }
    }
    if (arg_curly_level > 0)
    {
      throw parse_error(ctx, "missing closing brace");
    }
    --pos;

    return {pos, type, expression};
  }

  auto parse_command(const parse_context& ctx, std::size_t pos) -> command_t
  {
    // std::clog << "----------------------------------" << std::endl;
    // std::clog << "line: " << ctx.line.substr(pos) << std::endl;
    if (ctx.line.at(pos) == '$')
    {
      return {pos, command_type::text, "$"};
    }
    else if (ctx.line.at(pos) == '%')
    {
      return {pos, command_type::text, "%"};
    }
    else if (ctx.line.at(pos) == '|')
    {
      if (pos != ctx.line.size() - 1)
      {
        throw parse_error(ctx, "Trailing characters after trim-right ($|)");
      }
      return {pos, command_type::trim_trailing_return, ""};
    }
    else if (ctx.line.at(pos) == '{')
    {
      return parse_expression(ctx, command_type::escape, pos + 1);
    }
    else if (ctx.line.substr(pos, 4) == "raw{")
    {
      return parse_expression(ctx, command_type::raw, pos + 4);
    }
    else if (ctx.line.substr(pos, 5) == "call{")
    {
      return parse_expression(ctx, command_type::call, pos + 5);
    }
    else
    {
      throw parse_error(ctx, "Unknown command");
    }
  }

  auto parse_text_line(const parse_context& ctx, const std::string& line) -> line_t
  {
    if (ctx.curly_level <= ctx.class_curly_level)
      throw parse_error(ctx, "Unexpected text outside of member function");

    auto commands = std::vector<command_t>{};
    for (std::size_t pos = 0; pos < line.size(); ++pos)
    {
      switch (line.at(pos))
      {
      case '$':
        commands.push_back(parse_command(ctx, ++pos));
        pos = commands.back().end_pos;
        break;
      default:
        if (commands.empty() or commands.back().type != command_type::text)
        {
          commands.push_back({pos, command_type::text, ""});
        }
        commands.back().text.push_back(line.at(pos));
        break;
      }
    }

    return line_t{line_type::text, commands};
  }

  void write_header(const parse_context& ctx)
  {
    auto serializer = kiste::cpp(ctx.os);
    auto kissTemplate = kiste::KisteTemplate(ctx, serializer);
    kissTemplate.render_header();
  }

  void write_lines(const parse_context& ctx, const std::vector<line_t>& lines)
  {
    auto serializer = kiste::cpp(ctx.os);
    auto lineTemplate = kiste::LineTemplate(ctx, serializer);
    lineTemplate.render_lines(lines);
  }

  auto write_class_header(const parse_context& ctx) -> void
  {
    auto serializer = kiste::cpp(ctx.os);
    auto classTemplate = kiste::ClassTemplate(ctx, serializer);
    classTemplate.render_header();
  }

  auto write_class_member(const parse_context& ctx, const member_t& member) -> void
  {
    auto serializer = kiste::cpp(ctx.os);
    auto classTemplate = kiste::ClassTemplate(ctx, serializer);
    classTemplate.render_member(member);
  }

  void write_class_footer(const parse_context& ctx)
  {
    if (ctx.class_curly_level)
      throw parse_error(ctx, "No class to end here");

    auto serializer = kiste::cpp(ctx.os);
    auto classTemplate = ClassTemplate(ctx, serializer);
    classTemplate.render_footer();
  }

  void write_footer(const parse_context& ctx)
  {
    if (ctx.class_curly_level)
      throw parse_error(ctx, "class not ended at the end of the file, did you forget $endclass?");

    auto serializer = kiste::cpp(ctx.os);
    auto kissTemplate = kiste::KisteTemplate(ctx, serializer);
    kissTemplate.render_footer();
  }

  auto parse_parent_class(const parse_context& ctx, const std::string& line) -> std::string
  {
    const auto colonPos = line.find_first_not_of(" \t");
    if (colonPos == line.npos)
    {
      return "";
    }
    if (line[colonPos] != ':')
    {
      throw parse_error(ctx, "Unexpected character after class name, did you forget a ':'?");
    }
    const auto nameBegin = line.find_first_not_of(" \t", colonPos + 1);
    if (nameBegin == line.npos)
    {
      throw parse_error(ctx, "Could not find parent class name");
    }
    const auto nameEnd = line.find_first_of(" \t", nameBegin);
    const auto parent_name = (nameEnd == line.npos) ? line.substr(nameBegin)
                                                    : line.substr(nameBegin, nameEnd - nameBegin);

    if (nameEnd != line.npos and line.find_first_not_of(" \t", nameEnd) != line.npos)
    {
      throw parse_error(ctx, "Unexpected trailing characters after parent class name");
    }

    return parent_name;
  }

  auto parse_class_member(const parse_context& ctx, const std::string& line) -> member_t
  {
    if (not ctx.class_curly_level)
    {
      throw parse_error(ctx, "Cannot add a member here, did you forget to call $class?");
    }
    const auto classBegin = line.find_first_not_of(" \t", std::strlen("member"));
    if (classBegin == line.npos)
      throw parse_error(ctx, "Could not find member class name");
    const auto classEnd = line.find_first_of(" \t", classBegin);
    if (classEnd == line.npos)
      throw parse_error(ctx, "Could not find member name");

    const auto member_class_name = line.substr(classBegin, classEnd - classBegin);

    const auto nameBegin = line.find_first_not_of(" \t", classEnd);
    if (nameBegin == line.npos)
      throw parse_error(ctx, "Could not find member name");
    const auto nameEnd = line.find_first_of(" \t", nameBegin);

    const auto member_name = (nameEnd == line.npos) ? line.substr(nameBegin)
                                                    : line.substr(nameBegin, nameEnd - nameBegin);

    if (line.find_first_not_of(" \t", nameEnd) != line.npos)
    {
      throw parse_error(ctx, "unexpected characters after member declaration");
    }

    return {member_class_name, member_name};
  }

  auto parse_class(const parse_context& ctx, const std::string& line) -> class_data_t
  {
    if (ctx.class_curly_level)
      throw parse_error(ctx, "Cannot open new class here, did you forget to call $endclass?");
    const auto nameBegin = line.find_first_not_of(" \t", std::strlen("class"));
    if (nameBegin == line.npos)
      throw parse_error(ctx, "Could not find class name");
    const auto nameEnd = line.find_first_of(" \t", nameBegin);

    auto cd = class_data_t{};
    cd.name = (nameEnd == line.npos) ? line.substr(nameBegin)
                                     : line.substr(nameBegin, nameEnd - nameBegin);
    cd.curly_level = ctx.curly_level;

    if (nameEnd != line.npos)
    {
      cd.parent_name = parse_parent_class(ctx, line.substr(nameEnd));
    };

    return cd;
  }

  auto parse_line(const parse_context& ctx) -> line_t
  {
    const auto pos_first_char = ctx.line.find_first_not_of(" \t");
    if (pos_first_char == ctx.line.npos)
    {
      if (ctx.class_curly_level and ctx.curly_level > ctx.class_curly_level)
      {
        return parse_text_line(ctx, ctx.line);
      }
      else
      {
        return empty_line;
      }
    }
    else
    {
      const auto rest = ctx.line.substr(pos_first_char + 1);
      switch (ctx.line.at(pos_first_char))
      {
      case '%':  // cpp line
        return line_t{line_type::cpp,
                      std::vector<command_t>{{0,
                                              command_type::cpp,
                                              ctx.line.substr(0, pos_first_char) +
                                                  ctx.line.substr(pos_first_char + 1)}}};
        break;
      case '$':  // opening / closing class or text line
        if (starts_with(rest, "class"))
        {
          return {parse_class(ctx, rest)};
        }
        else if (starts_with(rest, "endclass"))
        {
          return {line_type::class_end, {}};
        }
        else if (starts_with(rest, "member"))
        {
          return parse_class_member(ctx, rest);
        }
        else if (starts_with(rest, "|"))  // trim left
        {
          return parse_text_line(ctx, ctx.line.substr(pos_first_char + 2));
        }
        else
        {
          return parse_text_line(ctx, ctx.line);
        }
        break;
      default:
        return parse_text_line(ctx, ctx.line);
      }
    }
  }

  auto determine_curly_level(const parse_context& ctx) -> std::size_t
  {
    auto level = ctx.curly_level;
    for (const auto& c : ctx.line)
    {
      switch (c)
      {
      case '{':
        ++level;
        break;
      case '}':
        if (level == 0)
          throw parse_error(ctx, "Too many closing curly braces in C++");
        --level;
        break;
      default:
        break;
      }
    }
    return level;
  }

  auto determine_class_curly_level(const parse_context& ctx, const line_t& line) -> std::size_t
  {
    switch (line.type)
    {
    case line_type::class_begin:
      return ctx.curly_level;
    case line_type::class_end:
      return 0;
    default:
      return ctx.class_curly_level;
    }
  }

  auto has_trailing_return(const line_t& line) -> bool
  {
    if (line.type == line_type::text and not line.commands.empty() and
        line.commands.back().type == command_type::trim_trailing_return)
    {
      return false;
    }
    return true;
  }

  auto get_line_type(const line_t* line) -> line_type
  {
    if (not line)
    {
      return line_type::none;
    }
    return line->type;
  }

  auto parse(parse_context& ctx) -> std::vector<line_t>
  {
    auto lines = std::vector<line_t>{};

    auto previous_line = static_cast<line_t*>(nullptr);
    while (ctx.is.good())
    {
      ++ctx.line_no;
      getline(ctx.is, ctx.line);

      lines.push_back(parse_line(ctx));
      auto& line = lines.back();

      ctx.curly_level = determine_curly_level(ctx);
      ctx.class_curly_level = determine_class_curly_level(ctx, line);

      line.curly_level = ctx.curly_level;
      line.class_curly_level = ctx.class_curly_level;
      line.trailing_return = has_trailing_return(line);  // (not line.commands.empty() and
                                                         // line.commands.back().type ==
                                                         // command_type::trim_trailing_return);
      if (previous_line)
      {
        line.previous_type = get_line_type(previous_line);
        previous_line->next_type = line.type;
      }
      previous_line = &line;
    }
    write_lines(ctx, lines);
    lines = {};
    if (ctx.curly_level)
    {
      throw parse_error(ctx, "not enough closing curly braces");
    }

    return lines;
  }
}

auto usage(std::string reason = "") -> int
{
  if (not reason.empty())
    std::cerr << "ERROR: " << reason << std::endl;

  std::cerr
      << "Usage: kiste2cpp [--output OUTPUT_HEADER_FILENAME] [--report-exceptions] SOURCE_FILENAME"
      << std::endl;
  return 1;
}

auto main(int argc, char** argv) -> int
{
  auto source_file_path = std::string{};
  auto output_file_path = std::string{};
  auto report_exceptions = false;

  for (int i = 1; i < argc; ++i)
  {
    if (std::string{argv[i]} == "--output")
    {
      if (i + 1 < argc and output_file_path.empty())
      {
        output_file_path = argv[i + 1];
        ++i;
      }
      else
      {
        return usage("No output file given, or given twice");
      }
    }
    else if (std::string{argv[i]} == "--report-exceptions")
    {
      report_exceptions = true;
    }
    else if (source_file_path.empty())
    {
      source_file_path = argv[i];
    }
    else
    {
      return usage(std::string{"Extra argument: "} + argv[i]);
    }
  }

  if (source_file_path.empty())
    return usage("No input file given");

  std::ifstream ifs{source_file_path};
  if (not ifs)
  {
    std::cerr << "Could not open " << source_file_path << std::endl;
    return 1;
  }

  std::ostream* os = &std::cout;
  std::ofstream ofs;
  if (not output_file_path.empty())
  {
    ofs.open(output_file_path, std::ios::out);
    if (not ofs)
    {
      std::cerr << "Could not open output file " << output_file_path << std::endl;
      return 1;
    }

    os = &ofs;
  }

  auto ctx = kiste::parse_context{ifs, *os, source_file_path, report_exceptions};

  try
  {
    const auto lines = kiste::parse(ctx);
    kiste::write(ctx, lines);
  }
  catch (const kiste::parse_error& e)
  {
    std::cerr << "Parse error in file: " << ctx.filename << std::endl;
    std::cerr << "Line number: " << ctx.line_no << std::endl;
    std::cerr << "Message: " << e.what() << std::endl;
    std::cerr << "Line: " << e.line << std::endl;
    return 1;
  }
}
