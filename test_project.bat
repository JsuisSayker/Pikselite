install OpenCppCoverage if not found
Build in debug mode
go in the build folder and run the tests using ctest

to get the report, run the following command:
"C:\Program Files\OpenCppCoverage\OpenCppCoverage.exe" --sources src --sources interface --excluded_sources build --excluded_sources tests --exclude
d_sources "C:\Program Files" --excluded_sources "C:\Program Files (x86)" --excluded_sources "D:\a\_work" --export_type html:coverage -- build/tests/De
bug/pikselite_tests.exe