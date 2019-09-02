//
// Created by usagitoneko on 9/2/19.
//

#ifndef TGBOOK_GOODREADS_API_H
#define TGBOOK_GOODREADS_API_H

#include <string>
#include "book.h"

class GoodreadsApi {
public:
    explicit GoodreadsApi(const string& developer_key) : key(developer_key) {};
    std::shared_ptr<Book> search(const std::string& title) const;
private:
    std::string key;
};

#endif //TGBOOK_GOODREADS_API_H
