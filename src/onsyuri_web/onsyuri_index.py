import os
import json
import argparse

def make_filelist(inpath, urlbase):
    files = []
    for _root, _dirs, _files in os.walk(inpath):
        for file in _files:
            path = os.path.join(_root, file)
            relpath = os.path.relpath(path, inpath).replace('\\', '/')
            files.append({
                "path": relpath, "url": urlbase + '/'+ relpath
            })
    return files


def debug():
    cmdstr="-i ./asset/test_lua/ -o ./asset/test_lua/onsyuri_index.json --urlbase http://localhost:5500/asset/test_lua"
    main(cmdstr)

def main(cmdstr=None):
    parser = argparse.ArgumentParser(
        description="Generate onsyuri_index.json for web\n" 
            "v0.1, developed by devseed")
    parser.add_argument('-i', '--inpath', type=str, default='./')
    parser.add_argument('-o', '--outpath', type=str, default='onsyuri_index.json')
    parser.add_argument('--title', type=str, help="game title, default indir name")
    parser.add_argument('--gamedir', type=str, help="gamedir in web fs, default /onsyuri/{title}")
    parser.add_argument('--savedir', type=str, help="savedir in web fs, default /onsyuri_save/{title}")
    parser.add_argument('--urlbase', type=str, default='', help="load files from this url default ./")
    parser.add_argument('--args', type=str, nargs='+', default=[], help="args except --root, --save-path to onscripter")

    if cmdstr is None: args = parser.parse_args()
    else: args = parser.parse_args(cmdstr.split(' '))
    inpath = args.inpath
    outpath = args.outpath
    title = args.title
    gamedir = args.gamedir
    savedir = args.savedir
    urlbase = args.urlbase 
    onsargs = args.args

    inpath = os.path.abspath(inpath)
    if not title: title = os.path.basename(inpath)
    if len(urlbase)>0 and urlbase[-1]=='/': urlbase = urlbase[:-1]
    if not gamedir: gamedir = "/onsyuri/" + title
    if not savedir: savedir = "/onsyuri_save/"+title
    files = make_filelist(inpath, urlbase)

    onsyuri_index = {
        'title': title,
        'gamedir': gamedir,
        'savedir': savedir,
        'args': onsargs,
        'files':files
    }
    with open(outpath, 'w') as fp:
        json.dump(onsyuri_index, fp, indent=2) 

if __name__ == '__main__':
    debug()
    # main()