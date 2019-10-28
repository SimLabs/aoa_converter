import os, sys
import re
import json
from zipfile import ZipFile
from collections import defaultdict

if __name__ == '__main__':
    dir = sys.argv[1]
    statsdict = defaultdict(set)
    key_re = re.compile('^#(\w+)')

    for dirpath, dirnames, filenames in os.walk(dir):
        if 'airports-db' in dirnames:
            dirnames.remove('airports-db')
            print('removed airports-db')
        for file in filenames:
            if os.path.splitext(file)[1] == '.arsc':
                archive_path = os.path.join(dirpath, file)
                with ZipFile(archive_path) as archive:
                    for name in archive.namelist():
                        if os.path.splitext(name)[1] == '.aoa':
                            print(f'{name}: {file}')
