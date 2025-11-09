#ifndef LANGUAGES_H
#define LANGUAGES_H

struct LanguagePack
{
    const char* days[7];
    const char* months[12];
};

// English language pack
static const LanguagePack EN =
{
    .days = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},
    .months = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}
};

// Dutch language pack
static const LanguagePack NL = {
    .days = {"Zo", "Ma", "Di", "Wo", "Do", "Vr", "Za"},
    .months = {"jan", "feb", "mrt", "apr", "mei", "jun", "jul", "aug", "sep", "okt", "nov", "dec"}
};

#endif