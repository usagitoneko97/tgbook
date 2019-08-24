#include <iostream>
#include <string>
#include <list>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>

#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;

class BookHandler {
    string name;
public:
    explicit BookHandler(string& s): name(s) {};
    string get_name() {return name;}
    string get_book_summary();
};

string BookHandler::get_book_summary() {
    string s = "the chosen book is " + name;
    return s;
}

int main() {
    string token("964403607:AAFaCL75bqTie0r8FBJAsHcC0c3nkNtS0j4");
    TgBot::Bot bot(token);
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        BookHandler book_handler(message->text);
        string book_summary = book_handler.get_book_summary();
        cout << book_summary;
        bot.getApi().sendMessage(message->chat->id, book_summary);
    });
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
    return 0;
}
