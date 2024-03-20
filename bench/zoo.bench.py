import time

class Zoo:
    def __init__(self):
        self.aardvark = 1
        self.baboon = 1
        self.cat = 1
        self.donkey = 1
        self.elephant = 1
        self.fox = 1

    def ant(self):
        return self.aardvark

    def banana(self):
        return self.baboon

    def tuna(self):
        return self.cat

    def hay(self):
        return self.donkey

    def grass(self):
        return self.elephant

    def mouse(self):
        return self.fox


zoo = Zoo()
duration = 5
sum = 0
batches = 0

start = time.time()
i = 0
while time.time() < start + duration:
    while i < 10000:
        sum = sum + zoo.ant() + zoo.banana() + zoo.tuna() + zoo.hay() + zoo.grass() + zoo.mouse()
        i = i + 1
    batches = batches + 1
    i = 0

print(batches)  # [Throughput]
print(sum)  # [Value]
print(duration)  # [DurationInSecs]
