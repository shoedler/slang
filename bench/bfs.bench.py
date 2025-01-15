import time
import os 
from collections import deque

__dir = os.path.abspath(os.path.dirname(__file__))

with open(__dir + "/bfs.bench.input") as f:
    map_data = [list(line.strip()) for line in f.readlines()]

start = time.perf_counter()

ROWS = len(map_data)
COLS = len(map_data[0])
DIRS = [(-1, 0), (0, 1), (1, 0), (0, -1)]

prices1 = []
prices2 = []

SEEN = set()
for y in range(ROWS):
    for x in range(COLS):
        pos = (y,x)
        if pos in SEEN:
            continue

        crop_type = map_data[y][x]
        field = set()
        fences = {} # map of all directions to the set of field-positions that have a fence in that direction
        perimeter = 0

        for d in DIRS:
            fences[d] = set()

        # field-floodfill
        Q = deque([pos])
        while len(Q) > 0:
            cur_pos = Q.popleft()
            cy, cx = cur_pos

            if map_data[cy][cx] != crop_type or cur_pos in field:
                continue
            field.add(cur_pos)
            SEEN.add(cur_pos)

            for dir_idx, (dy, dx) in enumerate(DIRS):
                ny, nx = cy + dy, cx + dx
                if 0 <= ny < ROWS and 0 <= nx < COLS and map_data[ny][nx] == crop_type:
                    Q.append((ny, nx)) # in map & same crop-type
                else:
                    perimeter += 1
                    fences[DIRS[dir_idx]].add(cur_pos)

        # count sides
        sides = 0
        for dir_key in fences:
            fields = fences[dir_key]
            SEEN2 = set()
            for field_pos in fields:
                if field_pos not in SEEN2:
                    sides += 1

                    Q = deque([field_pos])
                    while len(Q) > 0:
                        pos = Q.popleft()
                        
                        if pos in SEEN2:
                            continue
                        SEEN2.add(pos)

                        for check_dir in DIRS:
                            npos = (pos[0] + check_dir[0], pos[1] + check_dir[1])
                            if npos in fields and npos not in SEEN2:
                                Q.append(npos)

        prices1.append(perimeter * len(field))
        prices2.append(sides * len(field))

print(sum(prices1))
print(sum(prices2))
print("elapsed: " + str(time.perf_counter() - start) + "s")