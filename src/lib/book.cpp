//
// Created by usagitoneko on 8/27/19.
//

#include "book.h"

string Book::dump() const {
    string authors_repr;
    for (const string& a : author) {
        authors_repr = authors_repr + a + ',';
    }
    string first_line = std::to_string(calibre_id) + ": " + title + " by " + authors_repr;
    string _formats = formats[0];
    for (auto f = (formats.begin() + 1); f != formats.end(); ++f) {
        _formats.append(", " + *f);
    }
    string format_repr = "\n    available formats: " + _formats;
    return first_line + format_repr;
}
ostream& operator<<(ostream& os, const Book& book) {
    os << book.dump();
    return os;
}
