import time
import os

__dir = os.path.abspath(os.path.dirname(__file__))

digits = "0123456789"

with open(__dir + "/parse.bench.input") as f:
   prg = f.read()

def run(do_enable):
   i = 0 
   res = 0
   enable = True
   
   def is_digit():
       return prg[i] in digits
       
   def check(s):
       return prg[i:i+len(s)] == s
       
   def match(s):
       nonlocal i
       if not check(s):
           return False
       i += len(s)
       return True
       
   def num():
       nonlocal i
       n = ""
       while is_digit():
           n += prg[i]
           i += 1
       return int(n)
       
   while i < len(prg):
       if do_enable and match("don't()"):
           enable = False
       if do_enable and match("do()"):
           enable = True
           
       if match("mul("):
           a = num()
           if not match(","): 
               continue
           b = num()
           if not check(")"):
               continue
           if enable:
               res += int(a) * int(b)
       i += 1
       
   return res

start = time.perf_counter()
print(run(False))  # 168539636
print(run(True))   # 97529391
print("elapsed: " + str(time.perf_counter() - start) + "s")
