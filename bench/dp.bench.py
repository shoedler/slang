import time
import os

__dir = os.path.abspath(os.path.dirname(__file__))

# Read input file
with open(__dir + "/dp.bench.input") as f:
    codes = [line.strip() for line in f.readlines()]

NUM_KEYS = [
    "789",
    "456",
    "123",
    " 0A"
]

DIR_KEYS = [
    " ^A",
    "<v>",
]

DIRS = {"^": (0,-1), ">": (1,0), "v": (0,1), "<": (-1,0)}

start = time.perf_counter()

def make_pad(pad):
    """Map each key to its position on the keypad. E.g. "1" -> (0,2)"""
    map = {}
    for y, line in enumerate(pad):
        for x, key in enumerate(line):
            if key != " ":
                map[key] = (x, y)
    return map

num_keypad = make_pad(NUM_KEYS)
dir_keypad = make_pad(DIR_KEYS)

def reps(s, n):
    """Repeat a string n times"""
    res = ""
    while len(res) < n:
        res += s
    return res

def unique_perms(s):
    """Generate all unique permutations of a string"""
    if len(s) == 0:
        return [tuple()]
    if len(s) == 1:
        return [(s,)]

    perms = []
    chars = list(s)
    
    def swap(i, j):
        chars[i], chars[j] = chars[j], chars[i]

    def permute(n):
        if n == 1:
            perms.append(tuple(chars))
            return
        for i in range(n):
            swap(i, n-1)
            permute(n-1)
            swap(i, n-1)

    permute(len(chars))
    return perms

DP = {}
def calc_presses(seq, depth, dirkey, cur):
    DP_KEY = (seq, depth, dirkey, cur)
    if DP_KEY in DP:
        return DP[DP_KEY]

    keypad = dir_keypad if dirkey else num_keypad

    # Base cases
    if len(seq) == 0:
        return 0
    if cur is None:
        cur = keypad["A"]

    # Calc distance to the next key
    cx, cy = cur
    px, py = keypad[seq[0]]
    dx = px - cx
    dy = py - cy

    # Generate the moves to get to the next key
    moves = ""
    if dx > 0 or dx < 0:
        moves += reps(">" if dx > 0 else "<", abs(dx))
    if dy > 0 or dy < 0:
        moves += reps("v" if dy > 0 else "^", abs(dy))

    min_len = len(moves) + 1
    if depth > 0:
        # Try all permutations of the moves to find the shortest path
        valid_lens = []
        for perm in unique_perms(moves):
            cx, cy = cur
            valid = True
            for move in perm:
                mdx, mdy = DIRS[move]
                cx += mdx
                cy += mdy
                if (cx, cy) not in keypad.values():  # Position not on keypad
                    valid = False
                    break
            if valid:
                # Recurse on the direction keypad
                path_len = calc_presses("".join(perm) + "A", depth-1, True, None)
                if path_len is not None:
                    valid_lens.append(path_len)
        if valid_lens:
            min_len = sorted(valid_lens)[0]

    res = min_len + calc_presses(seq[1:], depth, dirkey, (px, py))
    DP[DP_KEY] = res
    return res

# Calculate results
d2 = sum(int(code[:-1]) * calc_presses(code, 2, False, None) for code in codes)
d25 = sum(int(code[:-1]) * calc_presses(code, 25, False, None) for code in codes)

print(d2) # 215374
print(d25) # 260586897262600
print("elapsed: " + str(time.perf_counter() - start) + "s") 