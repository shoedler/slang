let s = [1,2,3,4]

print s.pop() // [expect] 4
print s       // [expect] [1, 2, 3]
print s.pop() // [expect] 3
print s       // [expect] [1, 2]
print s.pop() // [expect] 2
print s       // [expect] [1]
print s.pop() // [expect] 1
print s       // [expect] []
print s.pop() // [expect] nil
print s       // [expect] []
