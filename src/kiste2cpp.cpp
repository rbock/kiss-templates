#include <cstring>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>

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
    LineType previous_line_type = LineType::None;
    std::string class_name;
    std::string parent_class_name;

    parse_context(std::istream& is_, std::ostream& os_, const std::string& filename_) : is(is_), os(os_), filename{filename_}
    {
    }

    void open_string()
    {
      if (not stream_opened)
      {
        os << "_os << ";
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
      open_string();
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
        os << ";";
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
      os << "\n";
      if (stream_opened)
      {
        os << "       ";
      }
      previous_line_type = LineType::Text;
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

  auto parse_arg(parse_context& ctx, std::size_t pos) -> std::size_t
  {
    auto arg_curly_level = 0;

    // std::clog << "----------------------------------" << std::endl;
    // std::clog << "line: " << ctx.line.substr(pos) << std::endl;
    if (ctx.line.at(pos) == '$')
    {
      ctx.write_char('$');
      return pos + 1;
    }
    else if (ctx.line.at(pos) == '|')
    {
      ctx.trailing_return = false;
      if (pos != ctx.line.size() - 1)
      {
        throw parse_error(ctx, "Trailing characters after trim-right ($|)");
      }
      return pos + 1;
    }
    else if (ctx.line.at(pos) == '{')
    {
      ctx.close_stream();
      ctx.os << " _serialize(";
      pos += 1;
      arg_curly_level = 1;
    }
    else if (ctx.line.substr(pos, 5) == "call{")
    {
      ctx.close_stream();
      ctx.os << " (";
      pos += 5;
      arg_curly_level = 1;
    }
    else
    {
      throw parse_error(ctx, "Unknown command");
    }
    // std::clog << "----------------------------------" << std::endl;

    for (; pos < ctx.line.size() and arg_curly_level; ++pos)
    {
      switch (ctx.line.at(pos))
      {
      case '{':
        ++arg_curly_level;
        ctx.os << ctx.line.at(pos);
        break;
      case '}':
        --arg_curly_level;
        if (arg_curly_level)
        {
          ctx.os << ctx.line.at(pos);
        }
        else
        {
          ctx.os << "); ";
        }
        break;
      default:
        ctx.os << ctx.line.at(pos);
      }
    }
    return pos - 1;
  }

  auto parse_text_line(parse_context& ctx) -> void
  {
    ctx.open_text();
    if (ctx.curly_level <= ctx.class_curly_level)
      throw parse_error(ctx, "Unexpected text outside of function");
    ctx.os << "  ";
    for (int i = 0; i < ctx.curly_level; ++i)
    {
      ctx.os << "  ";
    }
    for (std::size_t i = 0; i < ctx.line.size(); ++i)
    {
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
      ctx.write_char('\\');
      ctx.write_char('n');
    }
    ctx.close_string();
  }

  void write_header(parse_context& ctx)
  {
    ctx.os << "#pragma once\n";
    ctx.os << "#include <kiste/terminal.h>\n";
    ctx.os << "\n";
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
    ctx.parent_class_name = (nameEnd == line.npos) ? line.substr(nameBegin) : line.substr(nameBegin, nameEnd - nameBegin);

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

    const auto member_name = (nameEnd == line.npos) ? line.substr(nameBegin) : line.substr(nameBegin, nameEnd - nameBegin);

		ctx.os << "  const decltype(" + member_class_name + "(std::declval<" + ctx.class_name + "_t>())) " + member_name + " = " + member_class_name + "(*this);\n";
	}

  void parse_class(parse_context& ctx, const std::string& line)
  {
    if (not ctx.class_name.empty())
      throw parse_error(ctx, "Cannot open new class here, did you forget to call $endclass?");
    const auto nameBegin = line.find_first_not_of(" \t", std::strlen("class"));
    if (nameBegin == line.npos)
      throw parse_error(ctx, "Could not find class name");
    const auto nameEnd = line.find_first_of(" \t", nameBegin);
    ctx.class_name = (nameEnd == line.npos) ? line.substr(nameBegin) : line.substr(nameBegin, nameEnd - nameBegin);
    ctx.class_curly_level = ctx.curly_level;

    if (nameEnd != line.npos)
    {
      parse_parent_class(ctx, line.substr(nameEnd));
    };

    ctx.os << "template<typename _Derived, typename _Data, typename _Serializer>\n";
    ctx.os << "struct " + ctx.class_name << "_t";
    if (not ctx.parent_class_name.empty())
    {
      ctx.os << ": public " + ctx.parent_class_name + "_t<" + ctx.class_name + "_t<_Derived, _Data, _Serializer>, _Data, _Serializer>";
    }
    ctx.os << "\n";
    ctx.os << "{\n";

    // data members
    if (not ctx.parent_class_name.empty())
    {
      ctx.os << "  using _parent_t = " + ctx.parent_class_name + "_t<" + ctx.class_name + "_t, _Data, _Serializer>;\n";
      ctx.os << "  const _parent_t& parent;\n";
    }
    ctx.os << "  const _Derived& child;\n";
    ctx.os << "  using _data_t = _Data;\n";
    ctx.os << "  const _data_t& data;\n";
    ctx.os << "  std::ostream& _os;\n";
    ctx.os << "  using _serializer_t = _Serializer;\n";
    ctx.os << "  const _serializer_t _serialize;\n";
    ctx.os << "\n";

    // constructor
    ctx.os << "  " + ctx.class_name + "_t(const _Derived& derived, const _Data& data_, std::ostream& os, const _Serializer& serialize):\n";
    if (not ctx.parent_class_name.empty())
    {
      ctx.os << "    _parent_t{*this, data_, os, serialize},\n";
      ctx.os << "    parent{*this},\n";
    }
    ctx.os << "    child{derived},\n";
    ctx.os << "    data{data_},\n";
    ctx.os << "    _os(os),\n";
    ctx.os << "    _serialize(serialize)\n";
    ctx.os << "  {}\n";

    ctx.os << "  // ----------------------------------------------------------------------\n";
  }

  void write_class_footer(const parse_context& ctx)
  {
    if (ctx.class_name.empty())
      throw parse_error(ctx, "No class to end here");

    ctx.os << "  // ----------------------------------------------------------------------\n";
    ctx.os << "};\n\n";

    ctx.os << "template<typename _Data, typename _Serializer>\n";
    ctx.os << "auto " + ctx.class_name + "(const _Data& data, std::ostream& os, const _Serializer& serialize)\n";
    ctx.os << "  -> " + ctx.class_name + "_t<kiste::terminal_t, _Data, _Serializer>\n";
    ctx.os << "{\n";
    ctx.os << "  return {kiste::terminal, data, os, serialize};\n";
    ctx.os << "}\n";
    ctx.os << "\n";
    ctx.os << "template<typename _Template>\n";
    ctx.os << "auto " + ctx.class_name + "(const _Template& templ)\n";
    ctx.os << "  -> " + ctx.class_name + "_t<kiste::terminal_t, typename _Template::_data_t, typename _Template::_serializer_t>\n";
    ctx.os << "{\n";
    ctx.os << "  return {kiste::terminal, templ.data, templ._os, templ._serialize};\n";
    ctx.os << "}\n";
    ctx.os << "\n";
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
        if (ctx.curly_level > ctx.class_curly_level)
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

auto main(int argc, char** argv) -> int
{
  if (argc != 2)
  {
    std::cerr << "Usage: kiste2cpp <sourcefilename> {namespace}" << std::endl;
    return 1;
  }

  const auto source_file_name = std::string(argv[1]);

  std::ifstream ifs{source_file_name};
  if (not ifs)
  {
    std::cerr << "Could not open " << source_file_name << std::endl;
    return 1;
  }

  auto ctx = parse_context{ifs, std::cout, source_file_name};

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
  };
}

