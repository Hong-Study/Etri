import math
import pandas as pd

data = pd.read_csv('./FD220002.csv', encoding='euc-kr')

distance = data['Distance']

total = {}

for dis in distance:
    if not math.isnan(dis):
        size = total.get(dis)

        if size == None:
            total[dis] = 1
        else:
            total[dis] = size + 1


sorted_dict = sorted(total.items())
for tup in sorted_dict:
     print("{key}, {value}".format(key=tup[0],value=tup[1]))