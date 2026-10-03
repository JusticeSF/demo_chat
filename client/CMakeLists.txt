add_executable(chat-client
    main.cpp
    chatwindow.cpp
    chatwindow.h
)

target_link_libraries(chat-client
    PRIVATE
        chat_common
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
)