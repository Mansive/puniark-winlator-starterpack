#!/usr/bin/env python3

from __future__ import annotations

import argparse
import filecmp
import lzma
from pathlib import Path
import shutil
import sys
import urllib.error
import urllib.request
import zipfile

UAL_BASE_URL = "https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download"
FRIDA_BASE_URL = "https://github.com/frida/frida/releases/download"

UAL_ARCHIVE_LAYOUT = {
    "win32": (
        "Win32-latest",
        ["d3d9-Win32.zip", "d3d11-Win32.zip", "d3d12-Win32.zip", "version-Win32.zip"],
    ),
    "win64": (
        "x64-latest",
        ["d3d9-x64.zip", "d3d11-x64.zip", "d3d12-x64.zip", "version-x64.zip"],
    ),
}

FRIDA_SUFFIXES = {
    "win32": "x86",
    "win64": "x86_64",
}


def log(message: str) -> None:
    print(message, flush=True)


def touch_stamp(stamp_path: Path) -> None:
    stamp_path.parent.mkdir(parents=True, exist_ok=True)
    stamp_path.write_text("ok\n", encoding="utf-8")


def copy_file_if_different(source: Path, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)

    if destination.exists() and filecmp.cmp(source, destination, shallow=False):
        return

    temp_path = destination.with_suffix(destination.suffix + ".tmp")
    shutil.copy2(source, temp_path)
    temp_path.replace(destination)


def download_file(url: str, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)

    if destination.exists():
        log(f"Using cached {destination}")
        return

    temp_path = destination.with_suffix(destination.suffix + ".tmp")
    log(f"Downloading {url}")
    try:
        with (
            urllib.request.urlopen(url, timeout=120) as response,
            temp_path.open("wb") as output_file,
        ):
            shutil.copyfileobj(response, output_file)
    except (urllib.error.URLError, TimeoutError, OSError) as error:
        temp_path.unlink(missing_ok=True)
        raise RuntimeError(f"Failed downloading {url}: {error}") from error

    temp_path.replace(destination)


def extract_zip_archive(archive_path: Path, destination: Path) -> None:
    log(f"Extracting {archive_path}")
    with zipfile.ZipFile(archive_path) as archive:
        archive.extractall(destination)


def extract_xz_archive(archive_path: Path, destination: Path) -> None:
    if destination.exists():
        log(f"Using cached {destination}")
        return

    log(f"Extracting {archive_path}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    temp_path = destination.with_suffix(destination.suffix + ".tmp")
    with (
        lzma.open(archive_path, "rb") as source_file,
        temp_path.open("wb") as output_file,
    ):
        shutil.copyfileobj(source_file, output_file)
    temp_path.replace(destination)


def prepare_ultimate_asi_loader(download_root: Path) -> None:
    download_stamp = download_root / ".download-complete"
    if download_stamp.exists():
        log(f"Using cached {download_stamp}")
        return

    for arch, (tag, assets) in UAL_ARCHIVE_LAYOUT.items():
        arch_root = download_root / arch
        zip_root = arch_root / "zips"

        for asset in assets:
            archive_path = zip_root / asset
            asset_url = f"{UAL_BASE_URL}/{tag}/{asset}"
            download_file(asset_url, archive_path)
            extract_zip_archive(archive_path, arch_root)

    touch_stamp(download_stamp)


def prepare_frida_gadget(download_root: Path, frida_version: str) -> None:
    download_stamp = download_root / f".download-complete-{frida_version}"
    if download_stamp.exists():
        log(f"Using cached {download_stamp}")
        return

    for arch, suffix in FRIDA_SUFFIXES.items():
        arch_root = download_root / arch
        archive_root = arch_root / "archives"

        archive_name = f"frida-gadget-{frida_version}-windows-{suffix}.dll.xz"
        archive_path = archive_root / archive_name
        dll_name = f"frida-gadget-{frida_version}-windows-{suffix}.dll"
        dll_path = arch_root / dll_name
        asset_url = f"{FRIDA_BASE_URL}/{frida_version}/{archive_name}"

        download_file(asset_url, archive_path)
        extract_xz_archive(archive_path, dll_path)

    touch_stamp(download_stamp)


def copy_ultimate_asi_loader_dlls(assets_root: Path, output_root: Path) -> None:
    for arch in ("win32", "win64"):
        source_dir = assets_root / arch
        dest_dir = output_root / arch

        if not source_dir.is_dir():
            raise RuntimeError(f"Missing assets directory: {source_dir}")

        if dest_dir.exists():
            shutil.rmtree(dest_dir)

        dest_dir.mkdir(parents=True, exist_ok=True)

        dll_files = sorted(source_dir.glob("*.dll"))
        if not dll_files:
            raise RuntimeError(f"No DLL files found in {source_dir}")

        for dll_path in dll_files:
            copy_file_if_different(dll_path, dest_dir / dll_path.name)


def copy_frida_gadget_assets(
    assets_root: Path, output_root: Path, frida_version: str, config_template: Path
) -> None:
    for arch, suffix in FRIDA_SUFFIXES.items():
        base_name = f"frida-gadget-{frida_version}-windows-{suffix}"
        source_dll = assets_root / arch / f"{base_name}.dll"
        dest_dir = output_root / arch / "scripts"
        dest_asi = dest_dir / f"{base_name}.asi"
        dest_config = dest_dir / f"{base_name}.config"

        if not source_dll.exists():
            raise RuntimeError(f"Missing Frida DLL: {source_dll}")

        dest_dir.mkdir(parents=True, exist_ok=True)

        for stale_file in dest_dir.glob(f"frida-gadget-*-windows-{suffix}.asi"):
            stale_file.unlink()
        for stale_file in dest_dir.glob(f"frida-gadget-*-windows-{suffix}.config"):
            stale_file.unlink()

        copy_file_if_different(source_dll, dest_asi)
        copy_file_if_different(config_template, dest_config)


def stage_runtime_output(
    output_root: Path,
    executable_path: Path,
    frida_version: str,
    config_template: Path,
    run_auto_script: Path,
    run_picker_script: Path,
    ual_download_root: Path,
    frida_download_root: Path,
) -> None:
    output_root.mkdir(parents=True, exist_ok=True)

    if not executable_path.exists():
        raise RuntimeError(f"Missing executable output: {executable_path}")
    copy_file_if_different(executable_path, output_root / "bitnessscan.exe")

    copy_ultimate_asi_loader_dlls(ual_download_root, output_root)
    copy_frida_gadget_assets(
        frida_download_root, output_root, frida_version, config_template
    )

    copy_file_if_different(run_auto_script, output_root / "RUN_AUTO.bat")
    copy_file_if_different(run_picker_script, output_root / "RUN_PICKER.bat")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Prepare and stage runtime assets for bitnessscan builds"
    )
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--output-root", required=True)
    parser.add_argument("--exe-path", required=True)
    parser.add_argument("--frida-version", required=True)
    parser.add_argument("--config-template", required=True)
    parser.add_argument("--run-auto", required=True)
    parser.add_argument("--run-picker", required=True)
    parser.add_argument("--stamp", required=True)
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    source_root = Path(args.source_root).resolve()
    output_root = Path(args.output_root).resolve()
    executable_path = Path(args.exe_path).resolve()
    frida_version = args.frida_version
    config_template = Path(args.config_template).resolve()
    run_auto_script = Path(args.run_auto).resolve()
    run_picker_script = Path(args.run_picker).resolve()
    stamp_path = Path(args.stamp).resolve()

    ual_download_root = source_root / "downloads" / "ultimate-asi-loader"
    frida_download_root = source_root / "downloads" / "frida"

    try:
        if not config_template.exists():
            raise RuntimeError(f"Missing Frida config template: {config_template}")
        if not run_auto_script.exists():
            raise RuntimeError(f"Missing run script: {run_auto_script}")
        if not run_picker_script.exists():
            raise RuntimeError(f"Missing run script: {run_picker_script}")

        prepare_ultimate_asi_loader(ual_download_root)
        prepare_frida_gadget(frida_download_root, frida_version)
        stage_runtime_output(
            output_root=output_root,
            executable_path=executable_path,
            frida_version=frida_version,
            config_template=config_template,
            run_auto_script=run_auto_script,
            run_picker_script=run_picker_script,
            ual_download_root=ual_download_root,
            frida_download_root=frida_download_root,
        )
        touch_stamp(stamp_path)
    except Exception as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
