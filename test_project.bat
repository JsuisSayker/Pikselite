install OpenCppCoverage if not found
Build in debug mode
go in the build folder and run the tests using ctest

to get the report, run the following command:
"C:\Program Files\OpenCppCoverage\OpenCppCoverage.exe" --sources src --sources interface --excluded_sources build --excluded_sources tests --excluded_sources "C:\Program Files" --excluded_sources "C:\Program Files (x86)" --excluded_sources "D:\a\_work" --export_type html:coverage -- build/tests/Debug/pikselite_tests.exe
OpenCppCoverage --sources src --sources interface --excluded_sources build --excluded_sources tests --excluded_sources "C:\Program Files" --excluded_sources "C:\Program Files (x86)" --excluded_sources "D:\a\_work" --export_type html:coverage -- build/tests/Debug/pikselite_tests.exe




find src interface tests -name "*.cpp" \
| xargs -P4 -I{} clang-tidy {} -p build \
--config-file=.clang-tidy \
-fix \
--quiet


find src include -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) \
  -exec clang-format -i {} +



cppcheck --enable=all --inconclusive --std=c++20 --force --quiet --error-exitcode=1 --template=gcc --suppress=missingIncludeSystem --check-level=exhaustive -I interface/include src interface tests 2> cppcheck_report.txt