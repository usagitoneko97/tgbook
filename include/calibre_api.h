//
// Created by usagitoneko on 8/29/19.
//

#ifndef TGBOOK_CALIBRE_API_H
#define TGBOOK_CALIBRE_API_H

#include <exception>
#include <memory>
#include "Book.h"

class CalibreApi {
public:
    explicit CalibreApi(string& ip) : calibre_ip(ip) {}
    std::vector<int>    search(const string &title);
    Book                locate_book(int book_id);
    string              get_book_path(int book_id, const string& format);

private:
    string              calibre_ip;
};

class CalibreException : std::exception {
public:
    const char* what() const noexcept {
        // TODO: implement custom exception message
        return "Calibre Exceptions";
    }
};

#endif //TGBOOK_CALIBRE_API_H
