import lzma
import pathlib
import shutil
import sys


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: extract_xz.py <input.xz> <output>", file=sys.stderr)
        return 2

    src = pathlib.Path(sys.argv[1])
    dst = pathlib.Path(sys.argv[2])
    tmp = dst.with_suffix(dst.suffix + ".tmp")

    dst.parent.mkdir(parents=True, exist_ok=True)

    with lzma.open(src, "rb") as infile, open(tmp, "wb") as outfile:
        shutil.copyfileobj(infile, outfile)

    tmp.replace(dst)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
