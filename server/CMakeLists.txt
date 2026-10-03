add_executable(chat-server
    main.cpp
    chatserver.cpp
    chatserver.h
)

target_link_libraries(chat-server
    PRIVATE
        chat_common
        Qt5::Core
        Qt5::Network
)