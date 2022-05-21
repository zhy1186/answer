from aiohttp import web
import time


class Received_data:
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
        self.next_compute_time = 0  # timestamp in ms
        self.received_data_list = []

    def compute_statistics_once(self):


    def separate_received_nums(self, received_string):
        num_a = float(received_string[1:-1].split('A')[0])
        num_b = float(received_string[1:-1].split('A')[1])
        timestamp = self.get_now_ms_timestamp()
        data = Received_data(num_a, num_b, timestamp)
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
        return int(round(time.time()))

    async def receive(self, request):
        recv = await request.text()
        self.separate_received_nums(recv)

        return web.Response(text="Data Received :)")


# async def receive(request):
#     print(request.content_type)
#     print(await request.text())
#     return web.Response(text="Hello, World")


if __name__ == '__main__':
    svr = Server(1000, 1000, 'http://localhost', 20000)
    app = web.Application()
    app.add_routes([web.post('/', svr.receive)])
    web.run_app(app, host='localhost', port=20000)
