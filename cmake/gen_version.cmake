file(STRINGS ${SOURCE}/VERSION vstr)

if(${BTYPE} STREQUAL "Debug")
    string(APPEND vstr "-DEBUG")
endif()

set(CODESTR "#include \"VERSION.h\"\nnamespace LRI::RCI { const char* const RCI_VERSION = \"")
string(APPEND CODESTR ${vstr})
string(APPEND CODESTR "\"\; const char* const RCI_VERSION_END = RCI_VERSION + ")
string(LENGTH ${vstr} vstrlen)
string(APPEND CODESTR ${vstrlen})
string(APPEND CODESTR "\; }")
file(WRITE ${BIN}/VERSION.cpp ${CODESTR})
