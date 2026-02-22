#ifndef NOTICE_FUNCTIONS_H
#define NOTICE_FUNCTIONS_H

#include <string>
#include <vector>
#include <mutex>

struct Notice {
    int id;
    std::string title;
    std::string content;
    std::string author;
};

// Thread-safe mutex for file operations
extern std::mutex fileMutex;

// CRUD functions
bool addNotice(const Notice& notice);
std::vector<Notice> getAllNotices();
bool deleteNotice(int id);
std::vector<Notice> searchNotices(const std::string& keyword);

#endif
