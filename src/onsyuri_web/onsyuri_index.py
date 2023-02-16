import os
import json
import argparse

def make_lazyindex(name, indir):
    # use npm i -g browserfs before
    os.system(f"pushd {indir} && npx make_xhrfs_index > {name}")

def make_filelist(inpath, urlbase):
    files = []
    for _root, _dirs, _files in os.walk(inpath):
        for file in _files:
            path = os.path.join(_root, file)
            relpath = os.path.relpath(path, inpath).replace('\\', '/')
            url = urlbase + relpath if urlbase!="" else ""
            if url=="": files.append({"path": relpath})
            else :files.append({"path": relpath, "url": url})
    return files

def debug():
    cmdstr="-i ./asset/test_mo2demo/ -o ./build_web/onsyuri_index.json --urlbase http://localhost:5500/build_web/ --lazyload"
    main(cmdstr)

def main(cmdstr=None):
    parser = argparse.ArgumentParser(
        description="Generate onsyuri_index.json for web\n" 
            "v0.1, developed by devseed")
    parser.add_argument('-i', '--inpath', type=str, default='./')
    parser.add_argument('-o', '--outpath', type=str)
    parser.add_argument('--title', type=str, help="game title, default indir name")
    parser.add_argument('--gamedir', type=str, help="gamedir in web fs, default /onsyuri/{title}")
    parser.add_argument('--savedir', type=str, help="savedir in web fs, default /onsyuri_save/{title}")
    parser.add_argument('--urlbase', type=str, default='', help="load files from this url default ./")
    parser.add_argument('--lazyload',  action='store_true', help="lazyload by fetching file with filemap ")
    parser.add_argument('--lazyindex', type=str, help="the index file name for lazyload by BrowerFS (deprecated)")
    parser.add_argument('--args', type=str, nargs='+', default=[], help="args except --root, --save-path to onscripter")

    if cmdstr is None: args = parser.parse_args()
    else: args = parser.parse_args(cmdstr.split(' '))
    inpath = args.inpath
    outpath = args.outpath
    title = args.title
    gamedir = args.gamedir
    savedir = args.savedir
    urlbase = args.urlbase
    lazyindex =  args.lazyindex
    lazyload = args.lazyload
    onsargs = args.args

    inpath = os.path.abspath(inpath)
    if outpath==None: outpath = os.path.join(inpath, "onsyuri_index.json")
    if not title: title = os.path.basename(inpath)
    if not gamedir: gamedir = "/onsyuri/" + title
    if not savedir: savedir = "/onsyuri_save/"+title
    
    if lazyindex is not None:
        if  lazyindex=="": lazyindex="onsyuri_lazyindex.json"
        make_lazyindex(lazyindex, inpath)
    files = make_filelist(inpath, urlbase)

    onsyuri_index = {
        'title': title,
        'gamedir': gamedir,
        'savedir': savedir,
        'args': onsargs,
        'lazyload': lazyload,
        'files':files
    }
    if lazyindex is not None: onsyuri_index["lazyindex"] = lazyindex
    with open(outpath, 'w') as fp:
        json.dump(onsyuri_index, fp, indent=2) 

if __name__ == '__main__':
    # debug()
    main()