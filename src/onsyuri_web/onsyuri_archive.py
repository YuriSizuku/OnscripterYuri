#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""ONS (NS2/NSA) archive reader and asset helpers for OnscripterYuri Web.

This module owns the unpack logic so that the CLI entry point
``onsyuri_unpack.py`` stays small.

  ``ArchiveReader`` -- parses one archive and reads its entries.
  ``ArchiveSet``    -- discovers every archive plus loose files and prepares
                       AssetEntry objects (loose files first: they win on path
                       conflicts, like the engine's DirectReader).

Header parsing follows ``src/onsyuri/tool/nsadec.cpp`` and keeps ``url``
(the ASCII asset name the browser fetches) separate from ``path`` (the name
the engine fopen()s, decoded from the raw archive bytes with one code page).

  NS2  base_offset : LE32; entries '"name", length(LE32)' until a byte != '"'
  NSA  num_files   : BE16; base_offset BE32; name(NUL), compression(1B),
                     offset(BE32)+base_offset, length(BE32), original_length(BE32)
  NBZ  BE32 original length followed by a bzip2 stream
"""

import bz2
import os
import re
import struct
from pathlib import Path

NO_COMPRESSION = 0
SPB_COMPRESSION = 1
NBZ_COMPRESSION = 4

ARCHIVE_SUFFIXES = (".ns2", ".nsa")

# Fixed policy: every archive uses the same code page and safe naming.
PRIMARY_ENC = "gbk"
HEADER_BYTES = 4 * 1024 * 1024
CONTROL_CHARS = re.compile(r"[\x00-\x1f\x7f]")


# --------------------------------------------------------------- low level
def be16(buf, pos):
    return struct.unpack_from(">H", buf, pos)[0]


def be32(buf, pos):
    return struct.unpack_from(">I", buf, pos)[0]


def le32(buf, pos):
    return struct.unpack_from("<I", buf, pos)[0]


def read_head(path, size):
    with open(path, "rb") as fp:
        return fp.read(size)


def compression_of(name_bytes):
    """Mirror DirectReader::getRegisteredCompressionType (upper-case ext)."""
    dot = name_bytes.rfind(b".")
    if dot < 0:
        return NO_COMPRESSION
    ext = name_bytes[dot + 1:].upper()
    if ext == b"NBZ":
        return NBZ_COMPRESSION
    if ext == b"SPB":
        return SPB_COMPRESSION
    return NO_COMPRESSION


def normalize_str(s):
    s = s.replace("\\", "/")
    while s.startswith("/"):
        s = s[1:]
    return s


def valid_path(s):
    if not s or CONTROL_CHARS.search(s):
        return False
    if '"' in s:  # mount_game() splits on '/'; a quote would break JSON
        return False
    return True


def decode_path(raw):
    """Decode one raw archive name into the path the engine will fopen().

    The engine uses a single code page (``PRIMARY_ENC``), so no alternative
    code pages are registered.  A malformed name falls back to a hex placeholder
    so a bad entry can never abort the whole unpack.
    """
    try:
        path = normalize_str(raw.decode(PRIMARY_ENC))
    except UnicodeDecodeError:
        path = ""
    if not valid_path(path):
        path = "__unnamed_" + raw[:16].hex()
    return path


# ------------------------------------------------------------------ reader
class ArchiveEntry(object):
    """One raw entry inside an NS2/NSA archive."""

    __slots__ = ("name", "offset", "length", "compression")

    def __init__(self, name, offset, length, compression):
        self.name = name
        self.offset = offset
        self.length = length
        self.compression = compression


class ArchiveReader(object):
    """Parse one NS2/NSA archive and read its entry bytes."""

    def __init__(self, path):
        self.path = Path(path)
        self.entries = []
        head = read_head(self.path, HEADER_BYTES)
        if self.path.suffix.lower() == ".ns2":
            self._parse_ns2(head)
        else:
            self._parse_nsa(head)

    def _parse_ns2(self, head):
        base_offset = le32(head, 0)
        pos = 4
        records = []
        while pos < len(head) and head[pos] == 0x22:
            end = head.find(b'"', pos + 1)
            if end < 0 or end + 5 > len(head):
                break
            records.append((head[pos + 1:end], le32(head, end + 1)))
            pos = end + 5
        cursor = base_offset
        for name, length in records:
            self.entries.append(ArchiveEntry(name, cursor, length,
                                             compression_of(name)))
            cursor += length

    def _parse_nsa(self, head):
        num_files = be16(head, 0)
        pos = 2
        base_offset = be32(head, pos)
        pos += 4
        for _ in range(num_files):
            end = head.find(b"\x00", pos)
            if end < 0:
                break
            name = head[pos:end]
            pos = end + 1
            compression = head[pos]
            pos += 1
            data_offset = be32(head, pos) + base_offset
            pos += 4
            length = be32(head, pos)
            pos += 4
            pos += 4  # original_length; NBZ reads its own length from the stream
            if compression == NO_COMPRESSION:
                compression = compression_of(name)
            self.entries.append(ArchiveEntry(name, data_offset, length, compression))

    def read(self, entry):
        """Decode one entry; SPB is returned as-is (no Python decoder)."""
        with open(self.path, "rb") as fp:
            fp.seek(entry.offset)
            raw = fp.read(entry.length)
        if len(raw) != entry.length:
            raise IOError("short read: %s @ %d (want %d, got %d)" %
                          (self.path, entry.offset, entry.length, len(raw)))
        ctype = entry.compression
        if ctype == NO_COMPRESSION:
            return raw
        if ctype == NBZ_COMPRESSION:
            original_length = be32(raw, 0)
            data = bz2.decompress(raw[4:])
            if len(data) != original_length:
                raise IOError("NBZ length mismatch in %r" % entry.name)
            return data
        # SPB cannot be decoded here; store the raw bytes as-is.
        return raw


# ------------------------------------------------------------------ assets
class AssetEntry(object):
    """A file to extract, from an archive or from the loose directory."""

    __slots__ = ("raw_name", "path", "length",
                 "compression", "reader", "offset", "loose")

    def __init__(self, raw_name, path, length, compression, reader=None,
                 offset=0, loose=None):
        self.raw_name = raw_name
        self.path = path
        self.length = length
        self.compression = compression
        self.reader = reader
        self.offset = offset
        self.loose = loose

    def data(self):
        if self.loose is not None:
            return self.loose.read_bytes()
        return self.reader.read(ArchiveEntry(
            self.raw_name, self.offset, self.length, self.compression))


class ArchiveSet(object):
    """Discover archives + loose files and prepare AssetEntry objects."""

    def __init__(self, inpath, outpath):
        self.inpath = Path(inpath).resolve()
        self.outpath = Path(outpath).resolve()
        self.readers = []
        self.entries = []
        self._load_archives()
        self._build_entries()

    def _discover_archives(self):
        found = []
        for suffix in ARCHIVE_SUFFIXES:
            found.extend(self.inpath.rglob("*" + suffix))
        return sorted(set(found))

    def _load_archives(self):
        for path in self._discover_archives():
            self.readers.append(ArchiveReader(path))

    def _collect_loose(self):
        skip = {p.resolve() for p in (r.path for r in self.readers)}
        skip.add(self.outpath)
        loose = []
        for root, dirs, files in os.walk(str(self.inpath)):
            dirs[:] = [d for d in dirs if not d.startswith(".")]
            for name in files:
                if name.startswith(".") or name.lower().endswith(ARCHIVE_SUFFIXES):
                    continue  # never put 00.ns2 in the index: it re-enables NsaReader
                full = Path(root) / name
                if full.resolve() in skip:
                    continue
                loose.append((full.relative_to(self.inpath).as_posix(), full))
        loose.sort(key=lambda x: x[0])
        return loose

    def _build_entries(self):
        # Loose files come first: the index built downstream de-duplicates by
        # path keeping the first, mirroring NsaReader's DirectReader priority.
        kept = []
        for rel, full in self._collect_loose():
            kept.append(AssetEntry(rel.encode("utf-8"), normalize_str(rel),
                                   full.stat().st_size, NO_COMPRESSION,
                                   loose=full))
        for reader in self.readers:
            for rec in reader.entries:
                path = decode_path(rec.name)
                kept.append(AssetEntry(rec.name, path, rec.length, rec.compression,
                                       reader=reader, offset=rec.offset))
        self.entries = kept
