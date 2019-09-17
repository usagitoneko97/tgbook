//
// Created by usagitoneko on 8/29/19.
//

#ifndef TGBOOK_CALIBRE_API_H
#define TGBOOK_CALIBRE_API_H

#include <exception>
#include <memory>
#include "book.h"

class CalibreApi {
public:
    explicit CalibreApi(const string& ip) : calibre_ip(ip) {}
    CalibreApi() = default;
    CalibreApi& update(const string& ip){
        calibre_ip = ip;
        return *this;
    }

    std::vector<int>    search(const string &title);
    Book                locate_book(int book_id);
    string              get_book_path(int book_id, const string& format);
private:
    string              calibre_ip;
};

#endif //TGBOOK_CALIBRE_API_H
