#include <nlohmann/json.hpp>
#include <string>
#include <list>
#include <cstdio>
#include <exception>
#include <tgbot/tgbot.h>

#include "Book.h"
#include "calibre_api.h"
#include "goodreads_api.h"
#include "spdlog/spdlog.h"
#include "parser.h"

using namespace std;
using namespace TgBot;



int main() {
    string goodreads_key = "rlrW2MX64G1FMRxPJKVL7w";
    string token("964403607:AAFaCL75bqTie0r8FBJAsHcC0c3nkNtS0j4");
    string calibre_ip = "192.168.1.105:8080";
    spdlog::info("calibre using IP: {}", calibre_ip);
    spdlog::info("using telegram token: {}", token);
    spdlog::info("using goodreads developer key: {}", goodreads_key);
    //initialize all interface
    TgBot::Bot bot(token);
    CalibreApi calibre_api(calibre_ip);
    GoodreadsApi goodreads_api(goodreads_key);

    bot.getEvents().onCommand("book", [&bot, &calibre_api](const TgBot::Message::Ptr message) {
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
    });
    bot.getEvents().onCommand("book-get", [&bot, &calibre_api](TgBot::Message::Ptr message) {
        std::shared_ptr<BookGetParser> parser = Parser<BookGetParser>().parse(message->text);
        string path = calibre_api.get_book_path(parser->getBookId(), parser->getBookFormat());
        spdlog::info("trying to get book from path: {}", path);
        bot.getApi().sendDocument(message->chat->id, TgBot::InputFile::fromFile(path, "text/plain"));
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
