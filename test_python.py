# Simplified test of the Python logic
m = "[.##.]"
print(f"m = {m}")
print(f"m[-2:0:-1] = {m[-2:0:-1]}")
result = m[-2:0:-1].replace('#', '1').replace('.', '0')
print(f"after replace = {result}")
m_num = int(result, 2)
print(f"m_num = {m_num}")
