#include <ciso646>  // Make MSCV understand and/or/not
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "segment_type.h"
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

  auto parse_expression(const std::string& line, segment_type type, std::size_t pos) -> segment_t
  {
    auto expression = std::string{};
    auto arg_curly_level = 1;

    for (; pos < line.size() and arg_curly_level; ++pos)
    {
      switch (line.at(pos))
      {
      case '{':
        ++arg_curly_level;
        expression.push_back(line.at(pos));
        break;
      case '}':
        --arg_curly_level;
        if (arg_curly_level)
        {
          expression.push_back(line.at(pos));
        }
        else
        {
          // do nothing, as this probably the closing curly brace of the command
        }
        break;
      default:
        expression.push_back(line.at(pos));
      }
    }
    if (arg_curly_level > 0)
    {
      throw parse_error("missing closing brace");
    }
    --pos;

    return {pos, type, expression};
  }

  auto parse_command(const std::string& line, std::size_t pos) -> segment_t
  {
    // std::clog << "----------------------------------" << std::endl;
    // std::clog << "line: " << line.substr(pos) << std::endl;
    if (pos == line.size())
    {
      throw parse_error("Missing command after '$'");
    }
    else if (line.at(pos) == '$')
    {
      return {pos, segment_type::text, "$"};
    }
    else if (line.at(pos) == '%')
    {
      return {pos, segment_type::text, "%"};
    }
    else if (line.at(pos) == '|')
    {
      if (pos != line.size() - 1)
      {
        throw parse_error("Trailing characters after trim-right ($|)");
      }
      return {pos, segment_type::trim_trailing_return, ""};
    }
    else if (line.at(pos) == '{')
    {
      return parse_expression(line, segment_type::escape, pos + 1);
    }
    else if (line.substr(pos, 4) == "raw{")
    {
      return parse_expression(line, segment_type::raw, pos + 4);
    }
    else if (line.substr(pos, 5) == "call{")
    {
      return parse_expression(line, segment_type::call, pos + 5);
    }
    else
    {
      throw parse_error("Unknown command: " + line.substr(pos));
    }
  }

  auto parse_text_line(const parse_context& ctx, const std::string& line) -> line_data_t
  {
    if (ctx._curly_level <= ctx._class_curly_level)
      throw parse_error("Unexpected text outside of member function");

    auto text_line = line_data_t{line_type::text, {}};
    for (std::size_t pos = 0; pos < line.size(); ++pos)
    {
      switch (line.at(pos))
      {
      case '$':
      {
        pos = text_line.add_segment(parse_command(line, ++pos));
        break;
      }
      default:
        text_line.add_character(line.at(pos));
        break;
      }
    }

    return text_line;
  }

  auto parse_parent_class(const std::string& line) -> std::string
  {
    const auto colonPos = line.find_first_not_of(" \t");
    if (colonPos == line.npos)
    {
      return "";
    }
    if (line[colonPos] != ':')
    {
      throw parse_error("Unexpected character after class name, did you forget a ':'?");
    }
    const auto nameBegin = line.find_first_not_of(" \t", colonPos + 1);
    if (nameBegin == line.npos)
    {
      throw parse_error("Could not find parent class name");
    }
    const auto nameEnd = line.find_first_of(" \t", nameBegin);
    const auto parent_name = (nameEnd == line.npos) ? line.substr(nameBegin)
                                                    : line.substr(nameBegin, nameEnd - nameBegin);

    if (nameEnd != line.npos and line.find_first_not_of(" \t", nameEnd) != line.npos)
    {
      throw parse_error("Unexpected trailing characters after parent class name");
    }

    return parent_name;
  }

  auto parse_class_member(const parse_context& ctx, const std::string& line) -> member_t
  {
    if (not ctx._class_curly_level)
    {
      throw parse_error("Cannot add a member here, did you forget to call $class?");
    }
    const auto classBegin = line.find_first_not_of(" \t", std::strlen("member"));
    if (classBegin == line.npos)
      throw parse_error("Could not find member class name");
    const auto classEnd = line.find_first_of(" \t", classBegin);
    if (classEnd == line.npos)
      throw parse_error("Could not find member name");

    const auto member_class_name = line.substr(classBegin, classEnd - classBegin);

    const auto nameBegin = line.find_first_not_of(" \t", classEnd);
    if (nameBegin == line.npos)
      throw parse_error("Could not find member name");
    const auto nameEnd = line.find_first_of(" \t", nameBegin);

    const auto member_name = (nameEnd == line.npos) ? line.substr(nameBegin)
                                                    : line.substr(nameBegin, nameEnd - nameBegin);

    if (line.find_first_not_of(" \t", nameEnd) != line.npos)
    {
      throw parse_error("unexpected characters after member declaration");
    }

    return {member_class_name, member_name};
  }

  auto parse_class(const parse_context& ctx, const std::string& line) -> class_t
  {
    if (ctx._class_curly_level)
      throw parse_error("Cannot open new class here, did you forget to call $endclass?");
    const auto nameBegin = line.find_first_not_of(" \t", std::strlen("class"));
    if (nameBegin == line.npos)
      throw parse_error("Could not find class name");
    const auto nameEnd = line.find_first_of(" \t", nameBegin);

    auto cd = class_t{};
    cd._name = (nameEnd == line.npos) ? line.substr(nameBegin)
                                      : line.substr(nameBegin, nameEnd - nameBegin);
    if (nameEnd != line.npos)
    {
      cd._parent_name = parse_parent_class(line.substr(nameEnd));
    };

    return cd;
  }

  auto parse_line(const parse_context& ctx) -> line_data_t
  {
    const auto pos_first_char = ctx._line.find_first_not_of(" \t");
    if (pos_first_char == ctx._line.npos)
    {
      if (ctx._class_curly_level and ctx._curly_level > ctx._class_curly_level)
      {
        return parse_text_line(ctx, ctx._line);
      }
      else
      {
        return line_data_t{};
      }
    }
    else
    {
      const auto rest = ctx._line.substr(pos_first_char + 1);
      switch (ctx._line.at(pos_first_char))
      {
      case '%':  // cpp line
        return line_data_t{line_type::cpp,
                           std::vector<segment_t>{{0,
                                                   segment_type::cpp,
                                                   ctx._line.substr(0, pos_first_char) +
                                                       ctx._line.substr(pos_first_char + 1)}}};
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
          return parse_text_line(ctx, ctx._line.substr(pos_first_char + 2));
        }
        else
        {
          return parse_text_line(ctx, ctx._line);
        }
        break;
      default:
        return parse_text_line(ctx, ctx._line);
      }
    }
  }

  auto parse(parse_context& ctx) -> std::vector<line_t>
  {
    auto lines = std::vector<line_t>{};

    while (ctx._is.good())
    {
      ++ctx._line_no;
      getline(ctx._is, ctx._line);

      const auto line_data = parse_line(ctx);
      ctx.update(line_data);
      lines.push_back({ctx, line_data});

      if (lines.size() > 2)
      {
        auto& line = lines.at(lines.size() - 1);
        auto& previous_line = lines.at(lines.size() - 2);
        line._previous_line_ends_with_text = previous_line.ends_with_text();
        previous_line._next_line_starts_with_text = line.starts_with_text();
      }
    }
    if (not lines.empty() and lines.back()._curly_level)
    {
      throw parse_error("not enough closing curly braces");
    }

    return lines;
  }

  auto write(const parse_context& ctx, const std::vector<line_t>& lines) -> void
  {
    auto serializer = ::kiste::cpp(ctx._os);
    auto kissTemplate = ::kiste::KisteTemplate(ctx, serializer);
    auto classTemplate = ::kiste::ClassTemplate(ctx, serializer);
    auto lineTemplate = ::kiste::LineTemplate(ctx, serializer);

    auto class_data = class_t{};

    kissTemplate.render_header();
    auto line_no = std::size_t{0};
    for (const auto& line : lines)
    {
      ++line_no;
      switch (line._type)
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
        class_data = line._class_data;
        classTemplate.render_header(line_no, class_data);
        break;
      case line_type::member:
        classTemplate.render_member(line_no, class_data, line._member);
        break;
      case line_type::class_end:
        classTemplate.render_footer(line_no, class_data);
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

  std::cerr << "Usage: kiste2cpp [--output OUTPUT_HEADER_FILENAME] [--report-exceptions] "
               "[--no-line-directives] SOURCE_FILENAME" << std::endl;
  return 1;
}

auto main(int argc, char** argv) -> int
{
  auto source_file_path = std::string{};
  auto output_file_path = std::string{};
  auto report_exceptions = false;
  auto line_directives = true;

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
    else if (std::string{argv[i]} == "--no-line-directives")
    {
      line_directives = false;
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

  auto ctx = kiste::parse_context{ifs, *os, source_file_path, report_exceptions, line_directives};

  try
  {
    const auto lines = kiste::parse(ctx);
    kiste::write(ctx, lines);
  }
  catch (const kiste::parse_error& e)
  {
    std::cerr << "Parse error in file: " << ctx._filename << std::endl;
    std::cerr << "Line number: " << ctx._line_no << std::endl;
    std::cerr << "Message: " << e.what() << std::endl;
    std::cerr << "Line: " << ctx._line << std::endl;
    return 1;
  }
}
