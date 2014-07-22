# CMake generated Testfile for 
# Source directory: C:/Users/uppfinnarn/Projects/KanColleTool/tool
# Build directory: C:/Users/uppfinnarn/Projects/KanColleTool/tool
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(KanColleToolTest "C:/Users/uppfinnarn/Projects/KanColleTool/tool/bin/Debug/KanColleToolTest.exe")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(KanColleToolTest "C:/Users/uppfinnarn/Projects/KanColleTool/tool/bin/Release/KanColleToolTest.exe")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(KanColleToolTest "C:/Users/uppfinnarn/Projects/KanColleTool/tool/bin/MinSizeRel/KanColleToolTest.exe")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(KanColleToolTest "C:/Users/uppfinnarn/Projects/KanColleTool/tool/bin/RelWithDebInfo/KanColleToolTest.exe")
else()
  add_test(KanColleToolTest NOT_AVAILABLE)
endif()
subdirs(src)
subdirs(test)
