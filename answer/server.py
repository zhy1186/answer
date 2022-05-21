import math

from aiohttp import web
import time
import sys


class ReceivedData:
    def __init__(self, a, b, timestamp):
        self.a_ = a
        self.b_ = b
        self.timestamp_ = timestamp


class Server:
    def __init__(self, statistic_region_W_ms, statistic_interval_S_ms, host_str, port_int):
        self.statistic_region_W_ms_ = statistic_region_W_ms
        self.statistic_interval_S_ = statistic_interval_S_ms
        self.host_str_ = host_str
        self.port_ = port_int
        self.received_data_list = []
        self.server_start_time = int(round(time.time() * 1000))
        self.next_compute_time_ = self.server_start_time + self.statistic_interval_S_  # timestamp in ms

    def compute_statistics_once(self):
        [ai_list, bi_list] = self.separate_num_pairs_during_region()
        ai_times_bi_list = self.lista_times_listb(ai_list, bi_list)
        ai_info = self.compute_basic_statistic_variables(ai_list, "  ai   ")
        bi_info = self.compute_basic_statistic_variables(bi_list, "  bi   ")
        ai_times_bi_info = self.compute_basic_statistic_variables(ai_times_bi_list, "ai * bi")

        cov_ab = ai_times_bi_info[0] - ai_info[0] * bi_info[0]
        cor_ab = cov_ab / (ai_info[1] * bi_info[1])
        print("Cov(ai, bi): ", cov_ab, " Cor(ai, bi): ", cor_ab)

    def compute_basic_statistic_variables(self, list, title=""):
        sorted_list = self.simple_bubble_sort(list)

        min = sorted_list[0]
        max = sorted_list[-1]

        sum = 0
        for i in sorted_list:
            sum += i
        average = sum / float(len(sorted_list))

        median = 0
        if len(sorted_list) % 2 == 0:
            idx = round(len(sorted_list) / 2)
            median = (sorted_list[idx] + sorted_list[idx - 1]) / 2
        elif len(sorted_list) % 2 == 1:
            idx = round(len(sorted_list) / 2)
            median = sorted_list[idx]

        variance = 0
        for i in sorted_list:
            variance += (i - average) * (i - average)
        std_deviation = math.sqrt(variance)

        print(title, " ::", " max: ", max, " min: ", min, " average: ", average, " standard deviation: ",
              std_deviation, " median: ", median)

        return [average, std_deviation]

    @staticmethod
    def simple_bubble_sort(lst):
        n = len(lst)
        for i in range(n - 1):
            for j in range(i + 1, n):
                if lst[i] > lst[j]:
                    lst[i], lst[j] = lst[j], lst[i]
        return lst

    def separate_received_nums(self, received_string):
        num_a = float(received_string[1:-1].split('A')[0])
        num_b = float(received_string[1:-1].split('A')[1])
        timestamp = self.get_now_ms_timestamp()
        data = ReceivedData(num_a, num_b, timestamp)
        self.received_data_list.append(data)

    def separate_num_pairs_during_region(self):
        region = self.statistic_region_W_ms_
        now = self.get_now_ms_timestamp()
        start_time = now - region
        ai_list = []
        bi_list = []
        for item in self.received_data_list:
            if item.timestamp_ < start_time:
                continue
            if item.timestamp_ > now:
                break
            else:
                ai_list.append(item.a_)
                bi_list.append(item.b_)
        return [ai_list, bi_list]

    @staticmethod
    def lista_times_listb(lista, listb):
        size = len(lista)
        listab = []
        for i in range(size):
            listab.append(lista[i] * listb[i])
        return listab

    @staticmethod
    def get_now_ms_timestamp():
        return int(round(time.time() * 1000))

    async def receive(self, request):
        recv = await request.text()
        recv_time = self.get_now_ms_timestamp()
        self.separate_received_nums(recv)
        if recv_time > self.next_compute_time_:
            print("==========vvvvv Python Server vvvvv==========")
            self.compute_statistics_once()
            self.next_compute_time_ = recv_time + self.statistic_interval_S_
            print("=============================================")

        return web.Response(text="Data Received :)")


if __name__ == '__main__':
    # argv[1] : port, argv[2] : W(region), argv[3] : S(interval) e.g.  20000 5 3
    port = int(sys.argv[1])
    W = int(sys.argv[2])
    S = int(sys.argv[3])
    svr = Server(W * 1000, S * 1000, 'http://localhost', port)
    app = web.Application()
    app.add_routes([web.post('/', svr.receive)])
    web.run_app(app, host='localhost', port=20000)
