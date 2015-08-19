#!/usr/bin/env python
from __future__ import print_function
import os
import re
import sys

SOURCE_TEMPLATE = r'''

  BOOST_AUTO_TEST_CASE(ComparisonBasedTest_%(test_class_name)s)
  {
    std::stringstream os;
    auto serializer = kiste::%(serializer_type)s{os};

    %(data)s

    auto tmpl = ::comparison_based_test::%(test_class_name)s(data, serializer);
    tmpl.render();

    const auto actual = os.str();
    const auto expected = std::string{%(expected_output)s};
    BOOST_CHECK_EQUAL(actual, expected);
  }
'''

def escape_cpp_str(s):
    return r'R"-_-_-_-__-_-_-(%s)-_-_-_-__-_-_-"' % s

if __name__ == '__main__':
    assert len(sys.argv) == 3
    test_dir = sys.argv[1]
    output_header_path = sys.argv[2]

    filenames = [filename for filename in sorted(os.listdir(test_dir), key=lambda x: x.lower()) if filename.endswith('.kiste')]

    with open(output_header_path, 'w') as out_f:
        for filename in filenames:
            print('#include <%s.h>' % filename[:-len('.kiste')], file=out_f)

        for filename in filenames:
            m = re.search(r'\.([a-z]+)\.kiste$', filename)
            if not m:
                raise ValueError('File "%s" does not include the serializer type in its name, e.g. mytest.html.kiste' % filename)
            serializer_type = m.group(1)

            test_name_lowercase = filename[:-len('.%s.kiste' % serializer_type)]

            # Assume that main class name is named after file (in UpperCamelCase)
            test_class_name = re.sub('(?:_|^)([a-z])', lambda m: m.group(1).upper(), test_name_lowercase)

            with open(os.path.join(test_dir, filename[:-len('.kiste')] + '.expected'), 'rb') as expected_f:
                expected_output = escape_cpp_str(expected_f.read())

            data = 'struct TestHasNoData {} data;'
            data_src_path = os.path.join(test_dir, filename[:-len('.kiste')] + '.data')
            if os.path.exists(data_src_path):
                with open(data_src_path, 'rb') as data_src_f:
                    data = data_src_f.read()

            print(SOURCE_TEMPLATE % locals(), file=out_f)
