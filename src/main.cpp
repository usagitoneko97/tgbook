#include <rapidxml/rapidxml.hpp>
#include <nlohmann/json.hpp>
#include <cpr/response.h>
#include <cpr/cpr.h>
#include <string>
#include <list>
#include <cstdio>
#include <exception>
#include <tgbot/tgbot.h>

#include "Book.h"
#include "calibre_api.h"
#include "spdlog/spdlog.h"
#include "parser.h"

using namespace std;
using namespace TgBot;


Book& query_goodreads(const string& title, const string& author="") {
    cpr::Response r = cpr::Get(cpr::Url{"https://www.goodreads.com/search/index.xml"},
                               cpr::Parameters{{"key", "rlrW2MX64G1FMRxPJKVL7w"},
                                               {"q", title}}
    );
    rapidxml::xml_document<> doc;
    doc.parse<0>(&r.text[0]);
    rapidxml::xml_node<>* book = doc.first_node("GoodreadsResponse")->first_node("search")->first_node("results")->first_node("work")->first_node("best_book");
    Book* b = new Book(book->first_node("title")->value(), book->first_node("author")->first_node("name")->value());
    return *b;
}


string get_book_path(int book_id, const string& format) {
    std::map<string ,string> format_path;
    string url = "192.168.1.105:8080/ajax/book/" + std::to_string(book_id);
    cpr::Response book_response = cpr::Get(cpr::Url{url});
    nlohmann::json book_json = nlohmann::json::parse(book_response.text);
    string p = book_json.at("format_metadata").at(format).at("path").get<string>();
    return p;
}


int main() {
    string token("964403607:AAFaCL75bqTie0r8FBJAsHcC0c3nkNtS0j4");
    string calibre_ip = "192.168.1.105:8080";
    spdlog::info("using token: {}", token);

    //initialize all interface
    TgBot::Bot bot(token);
    CalibreApi calibre_api(calibre_ip);

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
    bot.getEvents().onCommand("book-get", [&bot](TgBot::Message::Ptr message) {
        std::shared_ptr<BookGetParser> parser = Parser<BookGetParser>().parse(message->text);
        string path = get_book_path(parser->getBookId(), parser->getBookFormat());
        spdlog::info("trying to get book from path: {}", path);
        bot.getApi().sendDocument(message->chat->id, TgBot::InputFile::fromFile(path, "text/plain"));
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
    return 0;
}
