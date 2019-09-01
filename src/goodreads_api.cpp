//
// Created by usagitoneko on 9/2/19.
//

#include <memory>
#include <rapidxml/rapidxml.hpp>
#include <cpr/response.h>
#include <cpr/cpr.h>
#include "Book.h"
#include "goodreads_api.h"


std::shared_ptr<Book> GoodreadsApi::search(const std::string &title) const {
    cpr::Response r = cpr::Get(cpr::Url{"https://www.goodreads.com/search/index.xml"},
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
