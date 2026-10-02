#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
REPO_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd -P)
ORIGINAL_DIR="$REPO_DIR/original"
MEDIA_SOURCE="$ORIGINAL_DIR/MagicMayhem_CD"
NOCD_SOURCE="$ORIGINAL_DIR/Arcane_Nocd"
WORKING_DIR="$REPO_DIR/working"
MEDIA_DEST="$WORKING_DIR/source-disc"
CLEAN_DEST="$WORKING_DIR/game-clean"
NOCD_DEST="$WORKING_DIR/game-nocd"
MANIFEST_DIR="$WORKING_DIR/manifests"
TEMPORARY_PATH=
ASSUME_YES=0

usage() {
    cat <<'EOF'
Usage: ./tools/prepare-working.sh [auto] [--yes]
       ./tools/prepare-working.sh stage-media
       ./tools/prepare-working.sh install [--from-installed DIRECTORY] [--yes]
       ./tools/prepare-working.sh verify

Commands:
  auto         Complete missing preparation steps, or verify an existing build
               (default).
  stage-media  Copy the manifest-verified extracted CD into working/source-disc.
  install      Build working/game-clean and working/game-nocd. By default this
               requires unshield to extract the InstallShield 4/5 data1.cab.
  verify       Verify all existing working-tree manifests.

Options:
  --from-installed DIRECTORY
               Use a clean, already-installed game directory instead of
               extracting data1.cab. Its complete contents are copied.
  --yes        Confirm replacement of existing generated installations without
               an interactive prompt.

Existing generated installations are verified by the default command and are
only replaced by an explicit install after confirmation.
EOF
}

die() {
    printf 'Error: %s\n' "$*" >&2
    exit 1
}

cleanup() {
    if [[ -n $TEMPORARY_PATH && -e $TEMPORARY_PATH ]]; then
        rm -rf -- "$TEMPORARY_PATH"
    fi
}

confirm_install_replacement() {
    local answer
    (( ASSUME_YES )) && return 0
    if [[ ! -t 0 ]]; then
        printf 'Refusing to replace existing working installations without a terminal; rerun with --yes.\n' >&2
        return 1
    fi
    read -r -p 'Replace working/game-clean and working/game-nocd? [y/N] ' answer
    [[ $answer == [yY] || $answer == [yY][eE][sS] ]]
}

remove_generated_install() {
    local target=$1
    case $target in
        "$CLEAN_DEST"|"$NOCD_DEST")
            [[ ! -e $target ]] || find "$target" -depth -delete
            ;;
        *) die "Refusing to remove unexpected path: $target" ;;
    esac
}

tree_manifest() {
    local root=$1 destination=$2 file relative size digest
    printf 'sha256\tsize\tpath\n' >"$destination"
    while IFS= read -r -d '' file; do
        relative=${file#"$root/"}
        size=$(stat -c '%s' -- "$file")
        digest=$(sha256sum -- "$file")
        digest=${digest%% *}
        printf '%s\t%s\t%s\n' "$digest" "$size" "$relative" >>"$destination"
    done < <(find "$root" -type f -print0 | LC_ALL=C sort -z)
}

write_manifest() {
    local root=$1 name=$2 temporary
    mkdir -p -- "$MANIFEST_DIR"
    temporary=$(mktemp "$MANIFEST_DIR/${name}.tsv.partial.XXXXXX")
    tree_manifest "$root" "$temporary"
    mv -f -- "$temporary" "$MANIFEST_DIR/${name}.tsv"
    chmod 0644 -- "$MANIFEST_DIR/${name}.tsv"
}

verify_tree() {
    local root=$1 manifest=$2 temporary
    [[ -d $root ]] || die "Working directory not found: $root"
    [[ -f $manifest ]] || die "Working manifest not found: $manifest"
    temporary=$(mktemp "$WORKING_DIR/.manifest.verify.XXXXXX")
    tree_manifest "$root" "$temporary"
    if ! cmp -s -- "$manifest" "$temporary"; then
        diff -u -- "$manifest" "$temporary" >&2 || true
        rm -f -- "$temporary"
        die "Working tree does not match its manifest: $root"
    fi
    rm -f -- "$temporary"
    printf 'Verified %s\n' "${root#"$REPO_DIR/"}"
}

verify_originals() {
    "$SCRIPT_DIR/original-manifest.sh" verify
}

stage_media() {
    local expected actual
    verify_originals
    mkdir -p -- "$WORKING_DIR" "$MANIFEST_DIR"

    if [[ -d $MEDIA_DEST ]]; then
        if [[ ! -f $MANIFEST_DIR/source-disc.tsv ]]; then
            expected=$(mktemp "$WORKING_DIR/.source.expected.XXXXXX")
            actual=$(mktemp "$WORKING_DIR/.source.actual.XXXXXX")
            tree_manifest "$MEDIA_SOURCE" "$expected"
            tree_manifest "$MEDIA_DEST" "$actual"
            if ! cmp -s -- "$expected" "$actual"; then
                rm -f -- "$expected" "$actual"
                die 'Staged source media differs from the immutable source'
            fi
            rm -f -- "$expected"
            mv -f -- "$actual" "$MANIFEST_DIR/source-disc.tsv"
            chmod 0644 -- "$MANIFEST_DIR/source-disc.tsv"
        fi
        verify_tree "$MEDIA_DEST" "$MANIFEST_DIR/source-disc.tsv"
        printf 'Source media is already staged.\n'
        return
    fi
    [[ ! -e $MEDIA_DEST ]] || die "Output exists and is not a directory: $MEDIA_DEST"

    TEMPORARY_PATH=$(mktemp -d "$WORKING_DIR/.source-disc.partial.XXXXXX")
    trap cleanup EXIT
    cp -a --reflink=auto -- "$MEDIA_SOURCE/." "$TEMPORARY_PATH/"

    expected=$(mktemp "$WORKING_DIR/.source.expected.XXXXXX")
    actual=$(mktemp "$WORKING_DIR/.source.actual.XXXXXX")
    tree_manifest "$MEDIA_SOURCE" "$expected"
    tree_manifest "$TEMPORARY_PATH" "$actual"
    if ! cmp -s -- "$expected" "$actual"; then
        rm -f -- "$expected" "$actual"
        die 'Staged source media differs from the immutable source'
    fi
    rm -f -- "$expected"

    mv -- "$TEMPORARY_PATH" "$MEDIA_DEST"
    TEMPORARY_PATH=
    trap - EXIT
    mv -f -- "$actual" "$MANIFEST_DIR/source-disc.tsv"
    chmod 0644 -- "$MANIFEST_DIR/source-disc.tsv"
    verify_originals
    printf 'Staged source media at %s\n' "${MEDIA_DEST#"$REPO_DIR/"}"
}

copy_cd_resident_assets() {
    local game_root=$1 directory
    for directory in Realms FMV; do
        if [[ -d $MEDIA_DEST/$directory ]]; then
            mkdir -p -- "$game_root/$directory"
            cp -a --reflink=auto -- "$MEDIA_DEST/$directory/." "$game_root/$directory/"
        fi
    done
}

validate_install() {
    local game_root=$1
    [[ -f $game_root/Chaos.exe ]] || die "Installed tree lacks Chaos.exe: $game_root"
    [[ -d $game_root/CFG || -d $game_root/Cfg || -d $game_root/cfg ]] ||
        die "Installed tree lacks its CFG directory: $game_root"
    [[ -d $game_root/Realms || -d $game_root/realms ]] ||
        die "Installed tree lacks its Realms directory: $game_root"
}

extract_installshield() {
    local destination=$1 raw_extract=$2 log_file=$3 group_name target_path group_source group_dest
    command -v unshield >/dev/null 2>&1 || die \
        'unshield is required for data1.cab; install it or use --from-installed DIRECTORY'

    mkdir -p -- "$MANIFEST_DIR"
    if ! unshield t "$MEDIA_DEST/data1.cab" >"$log_file" 2>&1; then
        cp -- "$log_file" "$MANIFEST_DIR/unshield.failed.log"
        die 'unshield could not validate data1.cab; see working/manifests/unshield.failed.log'
    fi
    if ! unshield -d "$raw_extract" x "$MEDIA_DEST/data1.cab" >>"$log_file" 2>&1; then
        cp -- "$log_file" "$MANIFEST_DIR/unshield.failed.log"
        die 'unshield could not extract data1.cab; see working/manifests/unshield.failed.log'
    fi

    # InstallShield stores the destination mapping in setup metadata, while
    # unshield emits one sanitized top-level directory per file group. Rebuild
    # the installer's maximum-install layout explicitly and deterministically.
    while IFS='|' read -r group_name target_path; do
        group_source="$raw_extract/$group_name"
        if [[ ! -d $group_source ]]; then
            cp -- "$log_file" "$MANIFEST_DIR/unshield.failed.log"
            die "Expected InstallShield file group is missing: $group_name"
        fi
        if [[ $target_path == . ]]; then
            group_dest=$destination
        else
            group_dest="$destination/$target_path"
            mkdir -p -- "$group_dest"
        fi
        cp -a --reflink=auto -- "$group_source/." "$group_dest/"
    done <<'EOF'
Program_Executable_Files|.
Program_DLLs|.
Readme_Files|.
Configuration_Files|CFG
Creature_Files|Creatures
Bitmaps|Bitmaps
Cursors|Cursors
Realms|Realms
Sound_Files|Sounds
Sprite_Files|Sprites
Text_Files|Text
Wizard_Files|Wizards
AI_Experience_Files|AI
Interface_Screens_-_Generic|Interface
Interface_Screens_-_Low_Res|Interface
Interface_Screens_-_High_Res|Interface
Portraits_-_Low_Res|Portraits/640x480
Portraits_-_High_Res|Portraits/800x600
SpeechBox_-_Low_Res|SpeechBox/640x480
SpeechBox_-_High_Res|SpeechBox/800x600
EOF
}

install_game() {
    local installed_source=${1:-} build_root clean_build nocd_build raw_extract log_file replace_existing=0
    stage_media

    if [[ -e $CLEAN_DEST || -e $NOCD_DEST ]]; then
        if ! confirm_install_replacement; then
            printf 'Installation replacement cancelled.\n' >&2
            return 1
        fi
        replace_existing=1
    fi

    TEMPORARY_PATH=$(mktemp -d "$WORKING_DIR/.install.partial.XXXXXX")
    trap cleanup EXIT
    build_root=$TEMPORARY_PATH
    clean_build="$build_root/game-clean"
    nocd_build="$build_root/game-nocd"
    raw_extract="$build_root/cabinet-raw"
    log_file="$build_root/unshield.log"
    mkdir -p -- "$clean_build"

    if [[ -n $installed_source ]]; then
        [[ -d $installed_source ]] || die "Installed source is not a directory: $installed_source"
        cp -a --reflink=auto -- "$installed_source/." "$clean_build/"
    else
        mkdir -p -- "$raw_extract"
        extract_installshield "$clean_build" "$raw_extract" "$log_file"
    fi

    copy_cd_resident_assets "$clean_build"
    validate_install "$clean_build"

    cp -a --reflink=auto -- "$clean_build" "$nocd_build"
    cp -a -- "$NOCD_SOURCE/Chaos.exe" "$nocd_build/Chaos.exe"
    cp -a -- "$NOCD_SOURCE/CDROMDrive.cfg" "$nocd_build/CDROMDrive.cfg"
    validate_install "$nocd_build"

    if (( replace_existing )); then
        remove_generated_install "$CLEAN_DEST"
        remove_generated_install "$NOCD_DEST"
    fi
    mv -- "$clean_build" "$CLEAN_DEST"
    mv -- "$nocd_build" "$NOCD_DEST"
    if [[ -f $log_file ]]; then
        mkdir -p -- "$MANIFEST_DIR"
        mv -- "$log_file" "$MANIFEST_DIR/unshield.log"
    fi
    cleanup
    TEMPORARY_PATH=
    trap - EXIT

    write_manifest "$CLEAN_DEST" game-clean
    write_manifest "$NOCD_DEST" game-nocd
    verify_originals
    verify_tree "$CLEAN_DEST" "$MANIFEST_DIR/game-clean.tsv"
    verify_tree "$NOCD_DEST" "$MANIFEST_DIR/game-nocd.tsv"
    printf 'Prepared clean and no-CD working installations.\n'
}

verify_working() {
    verify_originals
    [[ ! -d $MEDIA_DEST ]] || verify_tree "$MEDIA_DEST" "$MANIFEST_DIR/source-disc.tsv"
    [[ ! -d $CLEAN_DEST ]] || verify_tree "$CLEAN_DEST" "$MANIFEST_DIR/game-clean.tsv"
    [[ ! -d $NOCD_DEST ]] || verify_tree "$NOCD_DEST" "$MANIFEST_DIR/game-nocd.tsv"
}

auto_prepare() {
    if [[ -d $MEDIA_DEST && -d $CLEAN_DEST && -d $NOCD_DEST ]]; then
        verify_working
    else
        install_game "$installed_source"
    fi
}

command_name=auto
installed_source=
if [[ $# -gt 0 ]]; then
    case $1 in
        auto|stage-media|install|verify)
            command_name=$1
            shift
            ;;
        -h|--help|help)
            usage
            exit 0
            ;;
    esac
fi

while [[ $# -gt 0 ]]; do
    case $1 in
        --from-installed)
            [[ $# -ge 2 ]] || { usage >&2; exit 2; }
            installed_source=$2
            shift 2
            ;;
        --yes)
            ASSUME_YES=1
            shift
            ;;
        -h|--help|help)
            usage
            exit 0
            ;;
        *) usage >&2; exit 2 ;;
    esac
done

case $command_name in
    auto)
        auto_prepare
        ;;
    stage-media)
        [[ -z $installed_source ]] || { usage >&2; exit 2; }
        stage_media
        ;;
    install)
        install_game "$installed_source"
        ;;
    verify)
        [[ -z $installed_source ]] || { usage >&2; exit 2; }
        verify_working
        ;;
    *) usage >&2; exit 2 ;;
esac
