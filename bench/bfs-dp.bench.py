import os
import time

with open(os.getcwd() + "/bfs-dp.bench.input") as f:
   grid = [list(line.strip()) for line in f.readlines()]

ROWS = len(grid)
COLS = len(grid[0])

# Find start position
for i, row in enumerate(grid):
   if "^" in row:
       start = (i, row.index("^"))
       break

DIRS = [(-1,0), (0,1), (1,0), (0,-1)]

DP = {}
def trace(oy, ox):
   if (oy, ox) in DP:
       return DP[(oy, ox)]

   loop = {}
   seen = {}
   
   y, x = start
   d = 0  # up
   
   while True:
       if (y,x,d) in loop:
           DP[(oy, ox)] = True
           return True
           
       loop[(y,x,d)] = True
       seen[(y,x)] = True
       
       dy, dx = DIRS[d]
       ny = y + dy
       nx = x + dx
       
       if ny < 0 or ny >= ROWS or nx < 0 or nx >= COLS:
           DP[(oy, ox)] = len(seen)
           return len(seen)
           
       if grid[ny][nx] == "#" or (oy == ny and ox == nx):
           d = (d + 1) % 4  # Turn 90 deg CW
       else:
           y = ny
           x = nx

start_ = time.perf_counter()
print(trace(None, None)) # 5239

loops = 0
for oy in range(ROWS):
   for ox in range(COLS):
       if grid[oy][ox] == "#":
           continue
       if trace(oy, ox) is True:
           loops += 1
           
print(loops) # 1753
print("elapsed: " + str(time.perf_counter() - start_) + "s")
