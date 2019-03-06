protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
  proto/core.proto)
set_property(SOURCE ${PROTO_SRCS} PROPERTY SKIP_AUTOMOC ON)

add_executable(laph
  "main.cpp"
  "XCom.hpp"
  "util.cpp"
  "Glaph.cpp"
  "Socket.hpp"
  "NodeItem.cpp"
  "WireItem.cpp"
  "NodeMonitor.cpp"
  "SocketModel.cpp"
  "MessageModel.cpp"
  "UndoCommands.cpp"
  ${PROTO_SRCS})

target_link_libraries(laph
  julia
  Qt5::Qml
  Qt5::Quick
  Qt5::Widgets
  ${Protobuf_LIBRARIES})
