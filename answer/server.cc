#include <cctype>
#include <chrono>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "httplib.h"

struct received_data {
  double a{};
  double b{};
  std::chrono::milliseconds timestamp{};
};
class Server {
 public:
  Server() = default;
  Server(std::string host, int port, std::chrono::seconds W, std::chrono::seconds S)
      : statistic_region_W_(W), statistic_interval_S_(S), host_(std::move(host)), port_(port) {}

  void start_server_and_receive_nums() {
    svr_.Post("/data", [this](const httplib::Request& req, httplib::Response& res) {
      auto recv = req.body;
      for (auto i = recv.cbegin() + 1; i != recv.cend() - 1; ++i) {
        if (std::isupper(*i)) {
          double a = std::stod(std::string(recv.cbegin() + 1, i));
          double b = std::stod(std::string(i + 1, recv.cend() - 1));
          received_data_.push_back({a, b,
                                    std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::system_clock::now().time_since_epoch())});
          std::cout << received_data_.back().a << "  " << received_data_.back().b << "   "
                    << received_data_.back().timestamp.count() << std::endl;
        }
      }
    });

    svr_.listen(host_.c_str(), port_);
  }

 private:
  std::chrono::seconds statistic_region_W_{};
  std::chrono::seconds statistic_interval_S_{};
  std::string host_{};
  int port_{};
  httplib::Server svr_{};

  std::vector<received_data> received_data_{};
};
int main() {
  //  httplib::Server svr;
  //  svr.Post("/data", [](const httplib::Request& req, httplib::Response& res) {
  //    auto recv = req.body;
  //    std::cout << "3333" + recv + "4444" << std::endl;
  //    res.set_content("33333" + recv + "4444", "text/plain");
  //  });
  //  svr.listen("127.0.0.1", 20000);

  Server server("localhost", 20000, std::chrono::seconds{3}, std::chrono::seconds{3});
  server.start_server_and_receive_nums();
}
