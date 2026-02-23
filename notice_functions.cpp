#include "notice_functions.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>

std::mutex fileMutex;
const std::string filename = "notices.txt";

bool addNotice(const Notice& notice) {
    std::lock_guard<std::mutex> lock(fileMutex);

    // Prevent duplicate ID
    std::ifstream checkFile(filename);
    std::string line;
    while (getline(checkFile, line)) {
        std::istringstream ss(line);
        std::string temp;
        try {
            if (getline(ss, temp, '|')) {
                int existingId = std::stoi(temp);
                if (existingId == notice.id) {
                    return false; // Duplicate ID found
                }
            }
        }
        catch (...) {
            continue; // Skip corrupted line
        }
    }
    checkFile.close();

    std::ofstream file(filename, std::ios::app);
    if (!file) return false;

    file << notice.id << "|"
         << notice.title << "|"
         << notice.content << "|"
         << notice.author << "\n";

    return true;
}

std::vector<Notice> getAllNotices() {
    std::lock_guard<std::mutex> lock(fileMutex);

    std::vector<Notice> notices;
    std::ifstream file(filename);
    if (!file) return notices;

    std::string line;
    while (getline(file, line)) {
        std::istringstream ss(line);
        Notice n;
        std::string temp;

        try {
            if (getline(ss, temp, '|'))
                n.id = std::stoi(temp);
            else continue;
        }
        catch (...) {
            continue; // Skip corrupted line
        }

        getline(ss, n.title, '|');
        getline(ss, n.content, '|');
        getline(ss, n.author, '|');

        notices.push_back(n);
    }
    return notices;
}

bool deleteNotice(int id) {
    std::lock_guard<std::mutex> lock(fileMutex);

    std::ifstream file(filename);
    if (!file) return false;

    std::vector<Notice> notices;
    std::string line;
    bool found = false;

    while (getline(file, line)) {
        std::istringstream ss(line);
        Notice n;
        std::string temp;

        try {
            if (getline(ss, temp, '|'))
                n.id = std::stoi(temp);
            else continue;
        }
        catch (...) {
            continue; // Skip corrupted line
        }

        getline(ss, n.title, '|');
        getline(ss, n.content, '|');
        getline(ss, n.author, '|');

        if (n.id == id) {
            found = true;
        } else {
            notices.push_back(n);
        }
    }

    file.close();

    if (!found) return false;

    std::ofstream out(filename, std::ios::trunc);
    if (!out) return false;

    for (auto& n : notices) {
        out << n.id << "|"
            << n.title << "|"
            << n.content << "|"
            << n.author << "\n";
    }

    return true;
}

std::vector<Notice> searchNotices(const std::string& keyword) {
    std::lock_guard<std::mutex> lock(fileMutex);

    std::vector<Notice> result;
    std::ifstream file(filename);
    if (!file) return result;

    std::string line;
    while (getline(file, line)) {
        if (line.find(keyword) != std::string::npos) {

            std::istringstream ss(line);
            Notice n;
            std::string temp;

            try {
                if (getline(ss, temp, '|'))
                    n.id = std::stoi(temp);
                else continue;
            }
            catch (...) {
                continue; // Skip corrupted line
            }

            getline(ss, n.title, '|');
            getline(ss, n.content, '|');
            getline(ss, n.author, '|');

            result.push_back(n);
        }
    }

    return result;
}
