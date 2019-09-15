#include <nlohmann/json.hpp>
#include <string>
#include <list>
#include <cstdio>
#include <exception>
#include <tgbot/tgbot.h>

#include "book.h"
#include "calibre_api.h"
#include "goodreads_api.h"
#include "spdlog/spdlog.h"
#include "parser.h"

using namespace std;
using namespace TgBot;


void command_book(TgBot::Bot& bot, CalibreApi& calibre_api, const TgBot::Message::Ptr message) {
    string book_title = message->text.substr(6, message->text.length());
    try {
        std::vector<int> book_ids = calibre_api.search(book_title);
        std::vector<Book> books_found;
        string messages("books available in server:\n");
        for (int id : book_ids) {
            Book located_book = calibre_api.locate_book(id);
            books_found.emplace_back(located_book);
            messages += located_book.dump() += "\n";
        }
        try{
            bot.getApi().sendMessage(message->chat->id, messages);
        }
        catch (std::exception& e) {
            spdlog::warn("Message length is too long.");
            messages = messages.substr(0, 300);
            messages += "\n\n REDACTED since it's too long. Try to scope down the search";
            bot.getApi().sendMessage(message->chat->id, messages);
        }

    }
    catch (CalibreException& e){
        bot.getApi().sendMessage(message->chat->id, e.what());
    }
}

void command_book_dl(TgBot::Bot& bot, CalibreApi& calibre_api, const TgBot::Message::Ptr message) {
    try{
        std::shared_ptr<BookGetParser> parser = Parser<BookGetParser>().parse(message->text);
        string path = calibre_api.get_book_path(parser->getBookId(), parser->getBookFormat());
        spdlog::info("trying to get book from path: {}", path);
        auto result = bot.getApi().sendDocument(message->chat->id, TgBot::InputFile::fromFile(path, "text/plain"));
        spdlog::info("successfully sent document to chat id: {}", message->chat->id);
    }
    catch (CalibreException& e){
        bot.getApi().sendMessage(message->chat->id, e.what());
    }
}

int main() {
    const string goodreads_key = std::string(std::getenv("GOODREADS_KEY"));
    const string token = std::string(std::getenv("TG_TOKEN"));
    const string calibre_ip = std::string(std::getenv("CALIBRE_IP"));
    spdlog::info("calibre using IP: {}", calibre_ip);
    spdlog::info("using telegram token: {}", token);
    spdlog::info("using goodreads developer key: {}", goodreads_key);
    //initialize all interface
    TgBot::Bot bot(token);
    CalibreApi calibre_api(calibre_ip);
    GoodreadsApi goodreads_api(goodreads_key);

    //std::function<void(const TgBot::Message::Ptr)> command_book_f = std::bind(command_book, bot, calibre_api, _1);
    bot.getEvents().onCommand("book", [&](TgBot::Message::Ptr message) {
        command_book(bot, calibre_api, message);
    });
    bot.getEvents().onCommand("book-get", [&](TgBot::Message::Ptr message) {
        command_book_dl(bot, calibre_api, message);
    });

    try {
        spdlog::info("Bot Username: {}", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
    return 0;
}
