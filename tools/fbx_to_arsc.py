import os, sys
import re
from zipfile import ZipFile
from collections import defaultdict
from pathlib import Path
import tempfile
from subprocess import check_call, CalledProcessError, STDOUT

OSG_DIST_PATH = Path(r'E:\repos\aoa_converter\build\bin')
ORIGINAL_PATH = Path(r'E:\repos\aurora\!work\scenes\objects\buildings\asia')
IN_DIR = Path(r'E:\repos\aurora\!work\objects\test\all_static')
OUT_DIR = Path(r'E:\repos\aurora\!work\scenes\objects\buildings\asia_new')

ENV = {'OSG_NOTIFY_LEVEL': 'DEBUG'}

path_diff = lambda x, y: Path(x).relative_to(Path(y))

if __name__ == '__main__':
    os.chdir(str(OSG_DIST_PATH))

    statsdict = defaultdict(set)
    key_re = re.compile('^#(\w+)')

    for dirpath, dirnames, filenames in os.walk(str(ORIGINAL_PATH)):
        if 'airports-db' in dirnames:
            dirnames.remove('airports-db')
            print('removed airports-db')
        for file in filenames:
            if os.path.splitext(file)[1] == '.arsc':
                archive_path = Path(dirpath) / file
                dir_that_was_one_archive = IN_DIR / path_diff(archive_path.with_suffix(''), ORIGINAL_PATH)
                with tempfile.TemporaryDirectory() as temp_dir:
                    for archive_dirpath, _, archive_filenames in os.walk(str(dir_that_was_one_archive)):
                        for filename in archive_filenames:
                            if filename.endswith('.fbx'):
                                in_path = str(Path(archive_dirpath) / filename)
                                out_path = str(Path(temp_dir) / path_diff(archive_dirpath, dir_that_was_one_archive) / Path(filename).with_suffix('.aoa'))
                                log_path = Path(out_path).with_name('log.txt')
                                try:
                                    Path(log_path).parent.mkdir(parents=True, exist_ok=True)
                                    with open(log_path, 'w') as log_file:
                                        check_call(['osgconvd', '--convert-textures', 'dds', in_path, out_path],
                                                   env=ENV, stdout=log_file, stderr=STDOUT)
                                except CalledProcessError as e:
                                    # raise
                                    print(in_path, 'was not converted', file=sys.stderr)

                    # write archive
                    arsc_path = OUT_DIR /  path_diff(archive_path, ORIGINAL_PATH)
                    arsc_path.parent.mkdir(parents=True, exist_ok=True)
                    with ZipFile(arsc_path, mode='w') as archive:
                        for tempdir_path, _, tempdir_files in os.walk(temp_dir):
                            for tempdir_file in tempdir_files:
                                archive.write(Path(tempdir_path) / tempdir_file, arcname=str(path_diff(tempdir_path, temp_dir) / tempdir_file))




