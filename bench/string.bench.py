import time

start = time.process_time()

for i in range(0, 10000):
    a = str(1000)
    b = str(1.1245)
    c = str("hello")
    d = str(True)
    e = str(None)
    f = str([1, 2, 3])
    g = str({"a": 1, "b": 2})
    h = a + b + c + d + e + f + g

print("elapsed: " + str(time.process_time() - start) + "ms")
