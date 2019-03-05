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
  "UndoCommands.cpp")

target_link_libraries(laph
  julia
  Qt5::Qml
  Qt5::Quick
  Qt5::Widgets)
