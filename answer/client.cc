#include <chrono>
#include <cmath>
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
  void send_numbers_regularly() {
    while (true) {
      auto send_pair = generate_suitable_pair();
      send_numbers_once(send_pair.first, send_pair.second);
      std::this_thread::sleep_for(interval_);
    }
  }
#pragma clang diagnostic pop

  std::pair<double, double> generate_suitable_pair() {
    std::pair<double, double> uniform = generate_uniform_random_pair();
    std::pair<double, double> normal_independent = generate_std_normal_random_pair(uniform);
    std::pair<double, double> suitable_pair = generate_random_pair_sigma(normal_independent);
    return suitable_pair;
  }

 private:
  static std::pair<double, double> generate_uniform_random_pair() {
    double a = (rand() % 10001) / 10000.0;
    double b = (rand() % 10001) / 10000.0;
    return std::make_pair(a, b);
  }

  // Use Box-Mullelr transform
  static std::pair<double, double> generate_std_normal_random_pair(std::pair<double, double> uniform) {
    double a = uniform.first;
    double b = uniform.second;
    const double pi = 3.1415926535;
    double normal_a = std::sqrt((-2) * std::log(a)) * std::cos(2 * pi * b);
    double normal_b = std::sqrt((-2) * std::log(a)) * std::sin(2 * pi * b);
    return std::make_pair(normal_a, normal_b);
  }

  std::pair<double, double> generate_random_pair_sigma(std::pair<double, double> normal) const {
    double normal_a = normal.first;
    double normal_b = normal.second;
    double theta = 0.5 * std::asin(sigma_);
    double normal_sigma_a = std::cos(theta) * normal_a + std::sin(theta) * normal_b;
    double normal_sigma_b = std::cos(theta) * normal_b + std::sin(theta) * normal_a;
    return std::make_pair(normal_sigma_a, normal_sigma_b);
  }

  void send_numbers_once(double a, double b) {
    std::string body = "[" + std::to_string(a) + "A" + std::to_string(b) + "]";
    cli_.Post("/", body, "text/plain");
  }

 private:
  double sigma_{};
  std::chrono::milliseconds interval_{};
  httplib::Client cli_{std::string{}};
};

int main() {
  Client client("localhost", "20000", std::chrono::milliseconds{500}, 0.1);
  client.send_numbers_regularly();
}
