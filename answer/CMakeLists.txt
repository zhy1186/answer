add_executable(server server.cc)
target_link_libraries(server pthread)

add_executable(client client.cc)
target_link_libraries(client pthread)