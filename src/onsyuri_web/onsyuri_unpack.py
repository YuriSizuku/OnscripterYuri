import argparse
import json
import posixpath
from pathlib import Path

from onsyuri_archive import ArchiveSet


def safe_extension(name_bytes):
    _, sep, tail = name_bytes.rpartition(b".")
    if sep and tail.isalnum():
        return "." + tail.decode("ascii").lower()
    return ".bin"


def build_parser():
    p = argparse.ArgumentParser(
        description="Unpack NS2/NSA archives and generate onsyuri_index.json "
                    "for OnscripterYuri Web")
    p.add_argument("-i", "--inpath", default="./", help="game directory")
    p.add_argument("-o", "--outpath", help="index output (default <inpath>)")
    p.add_argument("-a", "--assetspath", help="assets output (default <outpath parent>/assets)")
    p.add_argument("--title", help="game title, default inpath name")
    p.add_argument("--gamedir", help="gamedir in web fs, default /onsyuri/{title}")
    p.add_argument("--savedir", help="savedir in web fs, default /onsyuri_save/{title}")
    p.add_argument("--urlbase", default="./assets",
                   help="url prefix for the extracted assets (default ./assets)")
    p.add_argument("--name-template", default="f{index:05d}{ext}",
                   help="safe asset name template, default f{index:05d}{ext}")
    p.add_argument("--no-lazyload", dest="lazyload", action="store_false",
                   default=True,
                   help="preload all files instead of fetching on demand")
    p.add_argument("--index-only", action="store_true",
                   help="Only generate onsyuri_index.json, do NOT write unpacked asset files to disk")
    p.add_argument("--args", nargs="+", default=[],
                   help="extra engine args")
    return p


def main(cmdstr=None):
    args = build_parser().parse_args(cmdstr.split() if cmdstr else None)

    inpath = Path(args.inpath).resolve()
    outpath = (Path(args.outpath).resolve() if args.outpath else inpath) / "onsyuri_index.json"
    assets_dir = Path(args.assetspath).resolve() if args.assetspath else (outpath.parent / "assets")
    if assets_dir.exists() and any(assets_dir.iterdir()):
        print("[error] %s is not empty; remove it before re-running" % assets_dir)
        return
    urlbase = args.urlbase
    title = args.title or inpath.name
    gamedir = args.gamedir or ("/onsyuri/" + title)
    savedir = args.savedir or ("/onsyuri_save/" + title)
    onsargs = list(args.args)
    lazyload_on = bool(args.lazyload)
    index_only = args.index_only

    archives = ArchiveSet(inpath, outpath)

    assets_dir.mkdir(parents=True, exist_ok=True)
    files = []
    seen = set()
    for i, e in enumerate(archives.entries):
        key = e.path.lower()
        if key in seen:
            continue
        seen.add(key)
        name = args.name_template.format(index=i, ext=safe_extension(e.raw_name))
        files.append({"path": e.path, "url": posixpath.join(urlbase, name)})
        if not index_only:
            with open(str(assets_dir / name), "wb") as fp:
                fp.write(e.data())

    index = {
        "title": title,
        "gamedir": gamedir,
        "savedir": savedir,
        "args": onsargs,
        "lazyload": lazyload_on,
        "files": files,
    }

    outpath.parent.mkdir(parents=True, exist_ok=True)
    with open(str(outpath), "w", encoding="utf-8", newline="\n") as fp:
        json.dump(index, fp, indent=2, ensure_ascii=True)
        fp.write("\n")


if __name__ == "__main__":
    main()
