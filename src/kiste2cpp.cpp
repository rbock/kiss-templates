#include <ciso646>  // Make MSCV understand and/or/not
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "ClassTemplate.h"
#include <kiste/cpp.h>

namespace
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

  enum class LineType
  {
    None,
    Cpp,
    Text
  };

  struct parse_context
  {
    std::istream& is;
    std::ostream& os;
    std::string filename;
    std::string line;
    std::size_t line_no = 0;
    std::size_t class_curly_level = 0;
    std::size_t curly_level = 0;
    bool trailing_return = true;
    bool stream_opened = false;
    bool string_opened = false;
    bool report_exceptions = false;
    LineType previous_line_type = LineType::None;
    std::string class_name;
    std::string parent_class_name;

    parse_context(std::istream& is_,
                  std::ostream& os_,
                  const std::string& filename_,
                  bool report_exceptions_)
        : is(is_), os(os_), filename{filename_}, report_exceptions{report_exceptions_}
    {
    }

    void open_string()
    {
      if (not stream_opened)
      {
        os << "_serialize.text(";
        stream_opened = true;
      }

      if (not string_opened)
      {
        os << "\"";
        string_opened = true;
      }
    }

    void write_char(char c)
    {
      switch (c)
      {
      case '\\':
      case '\"':
        os << "\\";
      default:
        break;
      }
      os << c;
    }

    void close_string()
    {
      if (string_opened)
      {
        os << "\"";
        string_opened = false;
      }
    }

    void close_stream()
    {
      close_string();
      if (stream_opened)
      {
        os << ");";
        stream_opened = false;
      }
    }

    void close_text()
    {
      if (previous_line_type == LineType::Text)
      {
        close_stream();
        os << "\n";
      }
      previous_line_type = LineType::None;
    }

    void open_text()
    {
      if (previous_line_type == LineType::Text)
        os << "\n";
      if (stream_opened)
      {
        os << "       ";
      }
      previous_line_type = LineType::Text;
    }

    void open_exception_handling()
    {
      if (report_exceptions)
      {
        os << " try {";
      }
    }
    void close_exception_handling(const std::string& expression)
    {
      if (report_exceptions)
      {
        os << "} catch(...) {_serialize.report_exception(__LINE__, \"";
        for (const auto& c : expression)
        {
          write_char(c);
        }
        os << "\", std::current_exception()); } ";
      }
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

  auto parse_expression(const parse_context& ctx, std::size_t& pos) -> std::string
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
          // do nothing;
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

    return expression;
  }

  auto parse_arg(parse_context& ctx, std::size_t pos) -> std::size_t
  {
    ctx.open_string();

    // std::clog << "----------------------------------" << std::endl;
    // std::clog << "line: " << ctx.line.substr(pos) << std::endl;
    if (ctx.line.at(pos) == '$')
    {
      ctx.write_char('$');
      return pos;
    }
    else if (ctx.line.at(pos) == '%')
    {
      ctx.write_char('%');
      return pos;
    }
    else if (ctx.line.at(pos) == '|')
    {
      ctx.trailing_return = false;
      if (pos != ctx.line.size() - 1)
      {
        throw parse_error(ctx, "Trailing characters after trim-right ($|)");
      }
      return pos;
    }
    else if (ctx.line.at(pos) == '{')
    {
      ctx.close_stream();
      pos += 1;
      ctx.open_exception_handling();
      const auto expression = parse_expression(ctx, pos);
      ctx.os << " _serialize.escape(" << expression << "); ";
      ctx.close_exception_handling(expression);
    }
    else if (ctx.line.substr(pos, 4) == "raw{")
    {
      ctx.close_stream();
      pos += 4;
      ctx.open_exception_handling();
      const auto expression = parse_expression(ctx, pos);
      ctx.os << " _serialize.raw(" << expression << "); ";
      ctx.close_exception_handling(expression);
    }
    else if (ctx.line.substr(pos, 5) == "call{")
    {
      ctx.close_stream();
      pos += 5;
      ctx.open_exception_handling();
      const auto expression = parse_expression(ctx, pos);
      ctx.os << " static_assert(std::is_same<decltype(" << expression
             << "), void>::value, \"$call{} requires void expression\"); (" << expression << "); ";
      ctx.close_exception_handling(expression);
    }
    else
    {
      throw parse_error(ctx, "Unknown command");
    }
    // std::clog << "----------------------------------" << std::endl;

    return pos;
  }

  auto parse_text_line(parse_context& ctx) -> void
  {
    ctx.open_text();
    if (ctx.curly_level <= ctx.class_curly_level)
      throw parse_error(ctx, "Unexpected text outside of function");
    ctx.os << "  ";
    for (std::size_t i = 0; i < ctx.curly_level; ++i)
    {
      ctx.os << "  ";
    }
    for (std::size_t i = 0; i < ctx.line.size(); ++i)
    {
      ctx.open_string();
      switch (ctx.line.at(i))
      {
      case '$':
        i = parse_arg(ctx, ++i);
        break;
      default:
        ctx.write_char(ctx.line.at(i));
      }
    }
    if (ctx.trailing_return)
    {
      ctx.open_string();
      ctx.os << "\\n";
    }
    ctx.close_string();
  }

  void write_header(parse_context& ctx)
  {
    ctx.os << "#pragma once\n";
    if (ctx.report_exceptions)
    {
      ctx.os << "#include <exception>\n";
    }
    ctx.os << "#include <kiste/terminal.h>\n";
    ctx.os << "\n";
    ctx.os << "#line " << 1 << " \"" << ctx.filename << "\"\n";
  }

  void parse_parent_class(parse_context& ctx, const std::string& line)
  {
    const auto colonPos = line.find_first_not_of(" \t");
    if (colonPos == line.npos)
      return;
    if (line[colonPos] != ':')
      throw parse_error(ctx, "Unexpected character after class name, did you forget a ':'?");
    const auto nameBegin = line.find_first_not_of(" \t", colonPos + 1);
    if (nameBegin == line.npos)
      throw parse_error(ctx, "Could not find parent class name");
    const auto nameEnd = line.find_first_of(" \t", nameBegin);
    ctx.parent_class_name = (nameEnd == line.npos) ? line.substr(nameBegin)
                                                   : line.substr(nameBegin, nameEnd - nameBegin);

    if (nameEnd != line.npos)
    {
      if (line.find_first_not_of(" \t", nameEnd) != line.npos)
        throw parse_error(ctx, "Unexpected trailing characters after parent class name");
    };
  }

  void parse_member(parse_context& ctx, const std::string& line)
  {
    if (ctx.class_name.empty())
      throw parse_error(ctx, "Cannot add a member here, did you forget to call $class?");
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

    // The "using" is required for clang-3.1 and older g++ versions
    const auto member_class_alias = member_class_name + "_t_alias";
    ctx.os << "  using " + member_class_alias + " = " + member_class_name + "_t<" + ctx.class_name +
                  "_t, _data_t, _serializer_t>; " + member_class_alias + " " + member_name + " = " +
                  member_class_alias + "{*this, data, _serialize};\n";
  }

  void parse_class(parse_context& ctx, const std::string& line)
  {
    if (not ctx.class_name.empty())
      throw parse_error(ctx, "Cannot open new class here, did you forget to call $endclass?");
    const auto nameBegin = line.find_first_not_of(" \t", std::strlen("class"));
    if (nameBegin == line.npos)
      throw parse_error(ctx, "Could not find class name");
    const auto nameEnd = line.find_first_of(" \t", nameBegin);
    ctx.class_name = (nameEnd == line.npos) ? line.substr(nameBegin)
                                            : line.substr(nameBegin, nameEnd - nameBegin);
    ctx.class_curly_level = ctx.curly_level;

    if (nameEnd != line.npos)
    {
      parse_parent_class(ctx, line.substr(nameEnd));
    };

#if 00
    auto serializer = kiste::cpp(ctx.os);
    auto classTemplate = kiste::ClassTemplate(ctx, serializer);
    classTemplate.render_header();
#else
    ctx.os << "template<typename DERIVED_T, typename DATA_T, typename SERIALIZER_T>\n";
    ctx.os << "struct " + ctx.class_name << "_t";
    if (not ctx.parent_class_name.empty())
    {
      ctx.os << ": public " + ctx.parent_class_name + "_t<" + ctx.class_name +
                    "_t<DERIVED_T, DATA_T, SERIALIZER_T>, DATA_T, SERIALIZER_T>";
    }
    ctx.os << "\n";
    ctx.os << "{\n";

    // data members
    if (not ctx.parent_class_name.empty())
    {
      ctx.os << "  using _parent_t = " + ctx.parent_class_name + "_t<" + ctx.class_name +
                    "_t, DATA_T, SERIALIZER_T>;\n";
      ctx.os << "  _parent_t& parent;\n";
    }
    ctx.os << "  DERIVED_T& child;\n";
    ctx.os << "  using _data_t = DATA_T;\n";
    ctx.os << "  const _data_t& data;\n";
    ctx.os << "  using _serializer_t = SERIALIZER_T;\n";
    ctx.os << "  _serializer_t& _serialize;\n";
    ctx.os << "\n";

    // constructor
    ctx.os << "  " + ctx.class_name +
                  "_t(DERIVED_T& derived, const DATA_T& data_, SERIALIZER_T& serialize):\n";
    if (not ctx.parent_class_name.empty())
    {
      ctx.os << "    _parent_t{*this, data_, serialize},\n";
      ctx.os << "    parent(*this),\n";
    }
    ctx.os << "    child(derived),\n";
    ctx.os << "    data(data_),\n";
    ctx.os << "    _serialize(serialize)\n";
    ctx.os << "  {}\n";

    ctx.os << "  // ----------------------------------------------------------------------\n";
    ctx.os << "#line " << ctx.line_no + 1 << "\n";
#endif
  }

  void write_class_footer(const parse_context& ctx)
  {
    if (ctx.class_name.empty())
      throw parse_error(ctx, "No class to end here");

#if !0
    auto serializer = kiste::cpp(ctx.os);
    auto classTemplate = ClassTemplate(ctx, serializer);
    classTemplate.render_footer();
#else
    ctx.os << "  // ----------------------------------------------------------------------\n";
    ctx.os << "#line " << ctx.line_no << "\n";
    ctx.os << "};\n\n";

    ctx.os << "#line " << ctx.line_no << "\n";
    ctx.os << "template<typename DATA_T, typename SERIALIZER_T>\n";
    ctx.os << "auto " + ctx.class_name + "(const DATA_T& data, SERIALIZER_T& serialize)\n";
    ctx.os << "  -> " + ctx.class_name + "_t<kiste::terminal_t, DATA_T, SERIALIZER_T>\n";
    ctx.os << "{\n";
    ctx.os << "  return {kiste::terminal, data, serialize};\n";
    ctx.os << "}\n";
    ctx.os << "\n";
    ctx.os << "#line " << ctx.line_no + 1 << "\n";
#endif
  }

  void write_footer(const parse_context& ctx)
  {
    if (not ctx.class_name.empty())
      throw parse_error(ctx, "class not ended at the end of the file, did you forget $endclass?");

    ctx.os << "\n";
  }

  void parse(parse_context& ctx)
  {
    while (ctx.is.good())
    {
      ++ctx.line_no;
      ctx.trailing_return = true;
      getline(ctx.is, ctx.line);

      const auto pos_first_char = ctx.line.find_first_not_of(" \t");
      if (pos_first_char == ctx.line.npos)
      {
        if (not ctx.class_name.empty() and ctx.curly_level > ctx.class_curly_level)
        {
          parse_text_line(ctx);
        }
        else
        {
          ctx.os << "\n";
        }
      }
      else
      {
        switch (ctx.line.at(pos_first_char))
        {
        case '%':  // cpp line
          switch (ctx.previous_line_type)
          {
          case LineType::None:
            break;
          case LineType::Cpp:
            break;
          case LineType::Text:
            ctx.close_text();
          }
          for (const auto& c : ctx.line)
          {
            switch (c)
            {
            case '{':
              ++ctx.curly_level;
              break;
            case '}':
              if (ctx.curly_level == 0)
                throw parse_error(ctx, "Too many closing curly braces in C++");
              --ctx.curly_level;
              break;
            default:
              break;
            }
          }
          ctx.os << ctx.line.substr(0, pos_first_char);
          ctx.os << ctx.line.substr(pos_first_char + 1) << '\n';
          ctx.previous_line_type = LineType::Cpp;
          break;
        case '$':  // opening / closing class or text line
        {
          const auto rest = ctx.line.substr(pos_first_char + 1);
          if (starts_with(rest, "class"))
          {
            parse_class(ctx, ctx.line.substr(pos_first_char + 1));
          }
          else if (starts_with(rest, "endclass"))
          {
            write_class_footer(ctx);
            ctx.class_name.clear();
            ctx.parent_class_name.clear();
          }
          else if (starts_with(rest, "member"))
          {
            parse_member(ctx, ctx.line.substr(pos_first_char + 1));
          }
          else if (starts_with(rest, "|"))  // trim left
          {
            ctx.line = ctx.line.substr(pos_first_char + 2);
            if (ctx.line.find_first_not_of(" \t") == ctx.line.npos)
            {
              std::cout << "Warning: No non-space characters after trim left ($|)" << std::endl;
            }
            parse_text_line(ctx);
          }
          else
          {
            parse_text_line(ctx);
          }
        }
        break;
        default:
          parse_text_line(ctx);
        }
      }
    }
    if (ctx.curly_level)
    {
      throw parse_error(ctx, "not enough closing curly braces");
    }
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

  auto ctx = parse_context{ifs, *os, source_file_path, report_exceptions};

  try
  {
    write_header(ctx);
    parse(ctx);
    write_footer(ctx);
  }
  catch (const parse_error& e)
  {
    std::cerr << "Parse error in file: " << ctx.filename << std::endl;
    std::cerr << "Line number: " << ctx.line_no << std::endl;
    std::cerr << "Message: " << e.what() << std::endl;
    std::cerr << "Line: " << e.line << std::endl;
    return 1;
  }
}
