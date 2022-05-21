import asyncio
import aiohttp
import time
import random
import math
import sys


class Client:
    def __init__(self, sigma, interval_ms, url):
        self.sigma_ = sigma
        self.interval_ms_ = interval_ms
        self.url = url

    # generate a pair of nums ~N(0, 1) with sigma
    def generate_suitable_pair(self):
        [a, b] = self.generate_uniform_random_pair()
        [normal_a, normal_b] = self.generate_std_normal_random_pair([a, b])
        [normal_sigma_a, normal_sigma_b] = self.generate_random_pair_sigma([normal_a, normal_b])
        return self.nums_to_str(normal_sigma_a, normal_sigma_b)

    # basic uniform random supported by only random.randint
    @staticmethod
    def generate_uniform_random_pair():
        a = random.randint(0, 10000) / 10000.0
        b = random.randint(0, 10000) / 10000.0
        return [a, b]

    # use Box-Muller transform to get a,b ~N(0,1) and sigma = 0 (independent a and b)
    @staticmethod
    def generate_std_normal_random_pair(list_ab):
        a = list_ab[0]
        b = list_ab[1]
        pi = 3.1415926535
        normal_a = math.sqrt((-2) * math.log(a)) * math.cos(2 * pi * b)
        normal_b = math.sqrt((-2) * math.log(a)) * math.sin(2 * pi * b)
        return [normal_a, normal_b]

    # introduce correlation factor sigma to get a,n ~N(0,1) with sigma
    def generate_random_pair_sigma(self, list_normal_ab):
        normal_a = list_normal_ab[0]
        normal_b = list_normal_ab[1]
        theta = 0.5 * math.asin(self.sigma_)
        normal_sigma_a = math.cos(theta) * normal_a + math.sin(theta) * normal_b
        normal_sigma_b = math.sin(theta) * normal_a + math.cos(theta) * normal_b
        return [normal_sigma_a, normal_sigma_b]

    # helper function
    @staticmethod
    def nums_to_str(a, b):
        return '[' + str(a) + 'A' + str(b) + ']'

    # send pair of nums to server and get rtn status
    async def send_once(self):
        send_string = self.generate_suitable_pair()
        async with aiohttp.ClientSession() as session:
            async with session.post(self.url, data=send_string) as resp:
                print(await resp.text())


if __name__ == '__main__':
    # argv[1] : sigma, argv[2] : interval(ms), argv[3] : port e.g.:  0.1 500 20000
    sigma = float(sys.argv[1])
    interval = int(sys.argv[2])
    port = str(sys.argv[3])
    url = 'http://localhost:' + port

    client = Client(sigma, interval, url)
    while True:
        loop = asyncio.get_event_loop()
        loop.run_until_complete(client.send_once())
        time.sleep(1)
