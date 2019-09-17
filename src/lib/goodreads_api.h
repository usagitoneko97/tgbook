//
// Created by usagitoneko on 9/2/19.
//

#ifndef TGBOOK_GOODREADS_API_H
#define TGBOOK_GOODREADS_API_H

#include <string>
#include "book.h"
#include <memory>

class GoodreadsApi {
public:
    using Method = enum {ID, TITLE, ISBN};
    explicit GoodreadsApi(const string& developer_key) : key(developer_key) {};
    GoodreadsApi() = default;
    GoodreadsApi& update(const string& developer_key) {
        key = developer_key;
        return *this;
    }
    std::shared_ptr<Book> search(const std::string& title) const;
    std::shared_ptr<Book> get_book_review(Method meth, const std::string& query);
private:
    std::string key;
};

#endif //TGBOOK_GOODREADS_API_H
