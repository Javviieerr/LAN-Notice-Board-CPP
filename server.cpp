
#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>

#include "notice_functions.h"

using asio::ip::tcp;

void sendLine(tcp::socket& socket, const std::string& msg) {
    asio::write(socket, asio::buffer(msg + "\n"));
}

void handle_client(tcp::socket socket) {
    try {
        asio::streambuf buf;
        asio::read_until(socket, buf, "\n");
        std::istream input(&buf);
        std::string line;

        while (std::getline(input, line)) {
            if (line.empty()) continue;

            std::istringstream ss(line);
            std::string command;
            getline(ss, command, '|');
            command.erase(command.find_last_not_of(" \n\r\t") + 1);

            if (command == "GETALL") {
                auto notices = getAllNotices();
                for (auto& n : notices) {
                    sendLine(socket, std::to_string(n.id) + "|" + n.title + "|" + n.content + "|" + n.author);
                }
                sendLine(socket, "END");
            }
            else if (command == "POST") {
              Notice n;
              std::string temp_id;
              bool valid = true;

               // 1. Get ID
              if (!std::getline(ss, temp_id, '|') || temp_id.empty()) valid = false;
               else {
                  try { n.id = std::stoi(temp_id); }
                  catch (...) { valid = false; }
                    }

               // 2. Get Title, Content, and Author
              if (valid && !std::getline(ss, n.title, '|'))   valid = false;
              if (valid && !std::getline(ss, n.content, '|')) valid = false;
              if (valid && !std::getline(ss, n.author, '|'))  valid = false;

              // 3. Final Check: Only add if all fields were present
              if (valid) {
               if (addNotice(n)) {
                sendLine(socket, "OK");
              } else {
                    sendLine(socket, "ERROR|Could not add notice to database");
                    }
            } else {
                  sendLine(socket, "ERROR|Invalid POST format. Expected: POST|ID|Title|Content|Author");
                }
            }
            else if (command == "DELETE") {
                std::string temp;
                if (getline(ss, temp, '|')) {
                    int id = std::stoi(temp);
                    if (deleteNotice(id)) sendLine(socket, "OK");
                    else sendLine(socket, "ERROR|Notice not found");
                }
            }
            else if (command == "SEARCH") {
                std::string keyword;
                if (getline(ss, keyword)) {
                    auto result = searchNotices(keyword);
                    for (auto& n : result) {
                        sendLine(socket, std::to_string(n.id) + "|" + n.title + "|" + n.content + "|" + n.author);
                    }
                    sendLine(socket, "END");
                }
            }
            else {
                sendLine(socket, "ERROR|Unknown command");
            }

            buf.consume(buf.size());
            asio::read_until(socket, buf, "\n");
            input.clear();
            input.seekg(0);
            input.rdbuf(&buf);
        }
    }
    catch (std::exception& e) {
        std::cerr << "Client disconnected: " << e.what() << "\n";
    }
}

int main()
{
    try
    {
        asio::io_context io_context;

        tcp::acceptor acceptor(io_context,
            tcp::endpoint(tcp::v4(), 9000));

        std::cout << "Server running on port 9000...\n";

        while (true)
        {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            std::cout << "Client connected.\n";

            std::thread(handle_client, std::move(socket)).detach();
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Server error: " << e.what() << std::endl;
    }

    return 0;
}
