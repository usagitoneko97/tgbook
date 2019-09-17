//
// Created by usagitoneko on 9/2/19.
//

#include "goodreads_api.h"
#include "book.h"
#include <cpr/cpr.h>
#include <cpr/response.h>
#include <fmt/format.h>
#include <memory>
#include <rapidxml/rapidxml.hpp>
#include "logging_utils.h"
#include <spdlog/spdlog.h>

std::shared_ptr<Book> GoodreadsApi::search(const std::string &title) const {
    spdlog::info("trying to search {} in goodreads", title);
    std::string url = "https://www.goodreads.com/search/index.xml";
    LOG_REQ("GET", url);
    cpr::Response r = cpr::Get(cpr::Url{url},
                               cpr::Parameters{{"key", key},
                                               {"q",   title}}
    );
    rapidxml::xml_document<> doc;
    doc.parse<0>(&r.text[0]);
    rapidxml::xml_node<> *book = doc.first_node("GoodreadsResponse")->first_node("search")->first_node(
            "results")->first_node("work")->first_node("best_book");
    std::shared_ptr<Book> b = std::make_shared<Book>(
            Book(book->first_node("title")->value(), book->first_node("author")->first_node("name")->value()));
    return b;
}

std::shared_ptr<Book> GoodreadsApi::get_book_review(Method meth, const std::string& query){
    // spdlog::info("Trying to get '{}' via {}", query, meth);
    cpr::Response r;
    using StrPair = std::pair<std::string, std::string>;
    // map the method to url and the key.
    std::map<Method, StrPair> meth_map_api {
        {ID, StrPair("show", "id")},
        {ISBN, StrPair("isbn", "isbn")},
        {TITLE, StrPair("title", "title")}
    };
    std::string url = fmt::format("https://www.goodreads.com/book/{}.xml", meth_map_api[meth].first);
    LOG_REQ("GET", url);
    r = cpr::Get(cpr::Url{url},
                 cpr::Parameters{{"key", key},
                                 {meth_map_api[meth].second, query}}
    );
    rapidxml::xml_document<> doc;
    doc.parse<0>(&r.text[0]);
    rapidxml::xml_node<> *book = doc.first_node("GoodreadsResponse")->first_node("book");
    std::shared_ptr<Book> b = std::make_shared<Book>(
        Book(book->first_node("title")->value(), book->first_node("author")->first_node("name")->value()));
    return b;
}
