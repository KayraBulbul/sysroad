foreach(tool echo cat wc)
  add_executable(${tool} ${tool}.cpp)
  target_link_libraries(${tool} PRIVATE coreutils project_warnings)
endforeach()

install(TARGETS echo cat wc RUNTIME DESTINATION bin)
