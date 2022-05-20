#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>

#include "httplib.h"

class Client {
 public:
  Client() = default;
  Client(const std::string& host, const std::string& port, std::chrono::milliseconds interval, double sigma)
      : sigma_(sigma), interval_(interval), cli_(host + ":" + port) {}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
  void send_numbers_regularly(double a, double b) {
    while (true) {
      auto send_pair = generate_random_pair();
      send_numbers_once(send_pair.first, send_pair.second);
      std::this_thread::sleep_for(interval_);
    }
  }
#pragma clang diagnostic pop

  static std::pair<double, double> generate_random_pair() {
    double a = (rand() % 10 + 1) / 10.0;
    double b = (rand() % 10 + 1) / 10.0;
    return std::make_pair(a, b);
  }

 private:
  void send_numbers_once(double a, double b) {
    std::string body = "[" + std::to_string(a) + "A" + std::to_string(b) + "]";
    cli_.Post("/data", body, "text/plain");
  }

 private:
  double sigma_{};
  std::chrono::milliseconds interval_{};
  httplib::Client cli_{std::string{}};
};

int main() {
  Client client("localhost", "20000", std::chrono::milliseconds{1000}, 0.1);
  client.send_numbers_regularly(1.234, 5.678);
  //  httplib::Client cli1("http://127.0.0.1:20000");
  //  auto rest = cli1.Post("/data", "123123123", "text/plain");
  //  std::cout << rest->body << std::endl;
}
