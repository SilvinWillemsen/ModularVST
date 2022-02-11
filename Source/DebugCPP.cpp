/*
  ==============================================================================

    DebugCPP.cpp
    Created: 10 Feb 2022 5:20:26pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include "DebugCPP.h"

#include <stdio.h>
#include <string>
#include <stdio.h>
#include <sstream>
// Usage in C++

//    Debug::Log ("Hellow Red", Color::Red);
//    Debug::Log ("Hellow Green", Color::Green);
//    Debug::Log ("Hellow Blue", Color::Blue);
//    Debug::Log ("Hellow Black", Color::Black);
//    Debug::Log ("Hellow White", Color::White);
//    Debug::Log ("Hellow Yellow", Color::Yellow);
//    Debug::Log ("Hellow Orange", Color::Orange);
//
//    Debug::Log (true, Color::Black);
//    Debug::Log (false, Color::Red);

//
//-------------------------------------------------------------------
void Debug::Log(const char* message, Color color) {
    if (callbackInstance != nullptr)
        callbackInstance(message, (int)color, (int)strlen(message));
}

void Debug::Log(const std::string message, Color color) {
    const char* tmsg = message.c_str();
    if (callbackInstance != nullptr)
        callbackInstance(tmsg, (int)color, (int)strlen(tmsg));
}

void Debug::Log(const int message, Color color) {
    std::stringstream ss;
    ss << message;
    send_log(ss, color);
}

void Debug::Log(const char message, Color color) {
    std::stringstream ss;
    ss << message;
    send_log(ss, color);
}

void Debug::Log(const float message, Color color) {
    std::stringstream ss;
    ss << message;
    send_log(ss, color);
}

void Debug::Log(const double message, Color color) {
    std::stringstream ss;
    ss << message;
    send_log(ss, color);
}

void Debug::Log(const bool message, Color color) {
    std::stringstream ss;
    if (message)
        ss << "true";
    else
        ss << "false";

    send_log(ss, color);
}

void Debug::send_log(const std::stringstream &ss, const Color &color) {
    const std::string tmp = ss.str();
    const char* tmsg = tmp.c_str();
    if (callbackInstance != nullptr)
        callbackInstance(tmsg, (int)color, (int)strlen(tmsg));
}
//-------------------------------------------------------------------

//Create a callback delegate
void RegisterDebugCallback(FuncCallBack cb) {
    callbackInstance = cb;
}


