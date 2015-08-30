#!/usr/bin/env python
from __future__ import print_function
import sys

def normalize(content):
    # Ignore newline differences (e.g. on Windows, std::endl is "\r\n") and any extra whitespace
    # at beginning/end of content
    return content.replace('\r\n', '\n').strip()

if __name__ == '__main__':
    assert len(sys.argv) == 4

    test_name = sys.argv[1]
    a_file_path = sys.argv[2]
    b_file_path = sys.argv[3]
    assert a_file_path != b_file_path

    with open(a_file_path, 'rb') as a:
        with open(b_file_path, 'rb') as b:
            a_content = normalize(a.read())
            b_content = normalize(b.read())
            if a_content == b_content:
                exit(0)
            import difflib
            diff = difflib.ndiff(a_content.splitlines(), b_content.splitlines(),
                                 charjunk=lambda c: False)
            diff = list(diff)

            if diff:
                print('Output headers of test %s differ:' % test_name, file=sys.stderr)
                for line in diff:
                    print(line, file=sys.stderr)
                exit(1)
