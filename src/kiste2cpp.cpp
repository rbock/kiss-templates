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
#include "parse_context.h"
#include "line.h"
#include <kiste/cpp.h>

#warning : Need to add license all over the place

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

  auto parse_class(const parse_context& ctx, const std::string& line) -> class_t
  {
    if (ctx.class_curly_level)
      throw parse_error(ctx, "Cannot open new class here, did you forget to call $endclass?");
    const auto nameBegin = line.find_first_not_of(" \t", std::strlen("class"));
    if (nameBegin == line.npos)
      throw parse_error(ctx, "Could not find class name");
    const auto nameEnd = line.find_first_of(" \t", nameBegin);

    auto cd = class_t{};
    cd.name = (nameEnd == line.npos) ? line.substr(nameBegin)
                                     : line.substr(nameBegin, nameEnd - nameBegin);
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
        return line_t{};
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
      ctx.update(line);
      line.update(ctx);

      if (previous_line)
      {
        line.previous_type = get_line_type(previous_line);
        previous_line->next_type = line.type;
      }
      previous_line = &line;
    }
    if (previous_line->curly_level)
    {
      throw parse_error(ctx, "not enough closing curly braces");
    }

    return lines;
  }

  auto write(const parse_context& ctx, const std::vector<line_t>& lines) -> void
  {
    auto serializer = ::kiste::cpp(ctx.os);
    auto kissTemplate = ::kiste::KisteTemplate(ctx, serializer);
    auto classTemplate = ::kiste::ClassTemplate(ctx, serializer);
    auto lineTemplate = ::kiste::LineTemplate(ctx, serializer);

    auto class_data = class_t{};

    kissTemplate.render_header();
    for (const auto& line : lines)
    {
      switch (line.type)
      {
      case line_type::none:
        lineTemplate.render_none();
        break;
      case line_type::cpp:
        lineTemplate.render_cpp(line);
        break;
      case line_type::text:
        lineTemplate.render_text(line);
        break;
      case line_type::class_begin:
        class_data = line.class_data;
        classTemplate.render_header(class_data);
        break;
      case line_type::member:
        classTemplate.render_member(class_data, line.member);
        break;
      case line_type::class_end:
        classTemplate.render_footer(class_data);
        break;
      }
    }
    kissTemplate.render_footer();
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
