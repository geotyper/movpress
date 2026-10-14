#!/usr/bin/env python3
"""Copy ffmpeg, ffprobe, and their non-system libraries into a macOS app bundle."""

import os
import shutil
import subprocess
import sys


def otool_deps(path):
    output = subprocess.check_output(["otool", "-L", path], text=True)
    deps = []
    for line in output.splitlines()[1:]:
        line = line.strip()
        if not line:
            continue
        deps.append(line.split(" (compatibility")[0].strip())
    return deps


def rpaths(path):
    output = subprocess.check_output(["otool", "-l", path], text=True)
    lines = output.splitlines()
    found = []
    for index, line in enumerate(lines):
        if "cmd LC_RPATH" not in line:
            continue
        for follow in lines[index:index + 6]:
            marker = "path "
            if marker in follow:
                found.append(follow.split(marker, 1)[1].split(" (", 1)[0].strip())
                break
    return found


def is_system(path):
    return path.startswith(("/usr/lib/", "/System/", "/Library/"))


def resolve_dep(dep, owner):
    if dep.startswith("@rpath/"):
        name = dep[len("@rpath/"):]
        for root in rpaths(owner):
            candidate = os.path.join(root, name)
            if os.path.exists(candidate):
                return candidate
        return ""
    if dep.startswith("@loader_path/"):
        candidate = os.path.normpath(os.path.join(os.path.dirname(owner), dep[len("@loader_path/"):]))
        return candidate if os.path.exists(candidate) else ""
    if dep.startswith("@executable_path/"):
        return ""
    return dep if os.path.exists(dep) else ""


def rewrite(path, bundled_names, executable):
    subprocess.run(["codesign", "--remove-signature", path], check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    deps = otool_deps(path)
    if not executable and deps:
        subprocess.check_call(["install_name_tool", "-id", f"@loader_path/{os.path.basename(path)}", path])
    own_id = deps[0] if deps and not executable else None
    for dep in deps:
        base = os.path.basename(dep)
        if base not in bundled_names or dep == own_id:
            continue
        new = f"@executable_path/../Frameworks/{base}" if executable else f"@loader_path/{base}"
        if dep != new:
            subprocess.check_call(["install_name_tool", "-change", dep, new, path])
    subprocess.check_call(["codesign", "--force", "--sign", "-", path])


def main():
    if len(sys.argv) != 4:
        print("usage: bundle_ffmpeg.py <Contents/MacOS> <ffmpeg> <ffprobe>", file=sys.stderr)
        return 1
    bin_dir, ffmpeg, ffprobe = sys.argv[1:]
    frameworks = os.path.normpath(os.path.join(bin_dir, "..", "Frameworks"))
    os.makedirs(frameworks, exist_ok=True)

    executables = []
    for source in (ffmpeg, ffprobe):
        destination = os.path.join(bin_dir, os.path.basename(source))
        shutil.copy2(source, destination)
        os.chmod(destination, 0o755)
        executables.append(destination)

    copied = {}
    pending = list(executables)
    seen = set(executables)
    while pending:
        current = pending.pop()
        for dep in otool_deps(current):
            resolved = resolve_dep(dep, current)
            if not resolved or is_system(resolved) or resolved in copied:
                continue
            name = os.path.basename(resolved)
            destination = os.path.join(frameworks, name)
            if name not in copied.values():
                shutil.copy2(resolved, destination)
                os.chmod(destination, 0o755)
            copied[resolved] = name
            if destination not in seen:
                seen.add(destination)
                pending.append(destination)

    names = set(copied.values())
    for path in executables:
        rewrite(path, names, True)
    for name in names:
        rewrite(os.path.join(frameworks, name), names, False)
    print(f"Bundled ffmpeg, ffprobe, and {len(names)} libraries into {frameworks}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
