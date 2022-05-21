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
      : statistic_region_W_(W), statistic_interval_S_(S), host_(std::move(host)), port_(port), next_compute_time_(0) {}

  void start_server(std::chrono::milliseconds start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())) {
    next_compute_time_ = start_time.count() + statistic_interval_S_.count();
    svr_.Post("/", [this](const httplib::Request& req, httplib::Response& res) {
      auto recv = req.body;
      auto recv_time =
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

      // save data
      for (auto i = recv.cbegin() + 1; i != recv.cend() - 1; ++i) {
        if (std::isupper(*i)) {
          double a = std::stod(std::string(recv.cbegin() + 1, i));
          double b = std::stod(std::string(i + 1, recv.cend() - 1));
          received_data_.push_back({a, b,
                                    std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::system_clock::now().time_since_epoch())});
          //          std::cout << received_data_.back().a << "  " << received_data_.back().b << "   "
          //                    << received_data_.back().timestamp.count() << std::endl;
        }
      }

      // compute or not
      if (recv_time.count() >= next_compute_time_) {
        std::cout << "vvvvvvvvvv" << std::endl;
        compute_statistics_once();
        next_compute_time_ += statistic_interval_S_.count();
        std::cout << "^^^^^^^^^^" << std::endl;
      }
    });

    svr_.listen(host_.c_str(), port_);
  }

  void compute_statistics_once() {
    auto vec_pair = separate_num_pairs_during_region();
    compute_cov_cor_and_output(vec_pair.first, vec_pair.second);
  }

 private:
  static void compute_cov_cor_and_output(std::vector<double>& vec_a, std::vector<double>& vec_b) {
    auto vec_a_times_b = veca_times_vecb(vec_a, vec_b);
    auto vec_a_info = compute_basic_statistic_variables(vec_a, "ai");
    auto vec_b_info = compute_basic_statistic_variables(vec_b, "bi");
    auto vec_ab_info = compute_basic_statistic_variables(vec_a_times_b, "ai * bi");
    auto cov_ab = vec_ab_info.first - vec_a_info.first * vec_b_info.first;
    auto cor_ab = cov_ab / (vec_a_info.second * vec_b_info.second);
    std::cout << "Cov(ai, bi): " << cov_ab << " Cor(ai, bi): " << cor_ab << std::endl;
  }

  std::pair<std::vector<double>, std::vector<double>> separate_num_pairs_during_region() {
    std::chrono::milliseconds region = std::chrono::duration_cast<std::chrono::milliseconds>(statistic_region_W_);
    auto now =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    auto start_time = now - region;
    std::vector<double> ai_vec{}, bi_vec{};
    for (auto itr = received_data_.cbegin(); itr != received_data_.end(); ++itr) {
      if (itr->timestamp < start_time) continue;
      if (itr->timestamp > now) break;
      ai_vec.push_back(itr->a);
      bi_vec.push_back(itr->b);
    }
    return std::make_pair(ai_vec, bi_vec);
  }

  static double compute_average(const std::vector<double>& vec) {
    double sum{};
    for (auto i : vec) {
      sum += i;
    }
    return sum / static_cast<double>(vec.size());
  }

  static std::pair<double, double> compute_basic_statistic_variables(std::vector<double>& vec,
                                                                     const std::string& title = "") {
    std::vector<double> res_vec = simple_bubble_sort(vec);

    double max_value = *(res_vec.cend() - 1);
    double min_value = *res_vec.cbegin();

    double average = compute_average(vec);

    double median = ((vec.size() % 2) == 0) ? ((vec.at(vec.size() / 2) + vec.at(vec.size() / 2 - 1)) / 2)
                                            : (vec.at(vec.size() / 2));

    double variance{};
    for (auto i : vec) {
      variance += (i - average) * (i - average);
    }
    variance /= static_cast<double>(vec.size());
    double std_deviation = std::sqrt(variance);

    std::cout << title << " :: "
              << " max: " << max_value << " min: " << min_value << " average: " << average
              << " standard deviation: " << std_deviation << " median: " << median << std::endl;

    return std::make_pair(average, std_deviation);
  }

  static std::vector<double> veca_times_vecb(const std::vector<double>& vec_a, const std::vector<double>& vec_b) {
    std::vector<double> times{};
    for (size_type i = 0; i < vec_a.size(); ++i) {
      times.push_back(vec_a.at(i) * vec_b.at(i));
    }
    return times;
  }

  using size_type = std::vector<double>::size_type;
  static std::vector<double>& simple_bubble_sort(std::vector<double>& vec) {
    if (vec.empty() || vec.size() < 2) return vec;

    for (size_type i = vec.size() - 1; i > 0; --i) {
      for (size_type j = 0; j < i; ++j) {
        if (vec.at(j) > vec.at(j + 1)) swap(vec, j, j + 1);
      }
    }
    return vec;
  }

  static void swap(std::vector<double>& vec, size_type i, size_type j) {
    double temp = vec.at(i);
    vec.at(i) = vec.at(j);
    vec.at(j) = temp;
  }

 private:
  std::chrono::seconds statistic_region_W_{};
  std::chrono::seconds statistic_interval_S_{};
  std::string host_{};
  int port_{};
  int64_t next_compute_time_{};
  httplib::Server svr_{};

  std::vector<received_data> received_data_{};
};
int main() {
  Server server("localhost", 20000, std::chrono::seconds{5}, std::chrono::seconds{3});
  server.start_server();
}
