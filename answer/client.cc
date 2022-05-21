#include <chrono>
#include <cmath>
#include <cstdlib>
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
  // generate (a, b) ~N(0, 1) with correlation sigma ,send to server and sleep
  void send_numbers_regularly() {
    while (true) {
      auto send_pair = generate_suitable_pair();
      send_numbers_once(send_pair.first, send_pair.second);
      std::this_thread::sleep_for(interval_);
    }
  }
#pragma clang diagnostic pop

  // generate a pair of nums ~N(0, 1) with sigma
  std::pair<double, double> generate_suitable_pair() {
    std::pair<double, double> uniform = generate_uniform_random_pair();
    std::pair<double, double> normal_independent = generate_std_normal_random_pair(uniform);
    std::pair<double, double> suitable_pair = generate_random_pair_sigma(normal_independent);
    return suitable_pair;
  }

 private:
  // basic uniform random supported by only rand() function
  static std::pair<double, double> generate_uniform_random_pair() {
    double a = (rand() % 10001) / 10000.0;
    double b = (rand() % 10001) / 10000.0;
    return std::make_pair(a, b);
  }

  // use Box-Muller transform to get a,b ~N(0,1) and sigma = 0 (independent a and b)
  static std::pair<double, double> generate_std_normal_random_pair(std::pair<double, double> uniform) {
    double a = uniform.first;
    double b = uniform.second;
    const double pi = 3.1415926535;
    double normal_a = std::sqrt((-2) * std::log(a)) * std::cos(2 * pi * b);
    double normal_b = std::sqrt((-2) * std::log(a)) * std::sin(2 * pi * b);
    return std::make_pair(normal_a, normal_b);
  }

  // introduce correlation factor sigma to get a,n ~N(0,1) with sigma
  std::pair<double, double> generate_random_pair_sigma(std::pair<double, double> normal) const {
    double normal_a = normal.first;
    double normal_b = normal.second;
    double theta = 0.5 * std::asin(sigma_);
    double normal_sigma_a = std::cos(theta) * normal_a + std::sin(theta) * normal_b;
    double normal_sigma_b = std::cos(theta) * normal_b + std::sin(theta) * normal_a;
    return std::make_pair(normal_sigma_a, normal_sigma_b);
  }

  // send nums to server using "text/plain"
  void send_numbers_once(double a, double b) {
    std::string body = "[" + std::to_string(a) + "A" + std::to_string(b) + "]";
    auto res = cli_.Post("/", body, "text/plain");
    std::cout << res->body << std::endl;
  }

 private:
  double sigma_{};
  std::chrono::milliseconds interval_{};
  httplib::Client cli_{std::string{}};
};

int main(__attribute__((unused)) int argc, char** argv) {
  // argv[1] : sigma, argv[2] : interval(ms), argv[3] : port e.g.: ./client 0.1 500 20000
  char** pend1 = nullptr;
  double sigma = std::strtod(argv[1], pend1);
  char** pend2 = nullptr;
  int64_t interval = std::strtol(argv[2], pend2, 10);
  const std::string port{argv[3]};

  Client client("localhost", port, std::chrono::milliseconds{interval}, sigma);
  client.send_numbers_regularly();
}
