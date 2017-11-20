import json
import glob
import sys
from pprint import pprint

data = []

files = glob.glob("trace*.trace")

maxFiles = 64
nFiles = 0
filename = 'data.txt'

if 2 == len(sys.argv):
  filename = sys.argv[1]

for f in files:
    nFiles += 1
    if nFiles > maxFiles:
      break
    with open(f) as json_data:
        d = json.load(json_data)
        json_data.close()
        data = data + d

with open(filename, 'w') as outfile:
        json.dump(data, outfile, sort_keys=True, indent=4)
