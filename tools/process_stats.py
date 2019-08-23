import json

with open('stats2.json') as f:
    stats_dict = json.load(f)
    stats = []
    for k, v in stats_dict.items():
        # print(k, 'count', len(v))
        stats.append((k, len(v)))
    stats.sort(key=lambda x: (x[1], x[0]))
    for l in stats:
        print(*l)