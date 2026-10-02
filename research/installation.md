# Reproducible installation

## Sources

The immutable source tree is recorded in `research/original-files.tsv`. The
preparation pipeline verifies that manifest before and after consuming source
artifacts.

The extracted CD tree in `original/MagicMayhem_CD/` is the source for
`working/source-disc/`. The original `magic-mayhem-disc.sh` is not used because
it writes generated files inside `original/`.

## Procedure

Stage and verify the source media:

```bash
./tools/prepare-working.sh stage-media
./tools/prepare-working.sh verify
```

With no arguments, `prepare-working.sh` completes any missing preparation steps.
If all three working trees already exist, it safely verifies them instead. An
explicit reinstall prompts before replacing generated installations; `--yes`
is available for intentional non-interactive rebuilding.

For a native extraction, install `unshield` and run:

```bash
./tools/prepare-working.sh install
```

The archive is InstallShield 4/5 rather than Microsoft CAB. `7z` and `unar`
cannot extract it. The pipeline tests `data1.cab`, extracts it with `unshield`,
maps its 20 non-empty file groups to their installer destinations, and adds the
CD-resident `Realms` and `FMV` directories.

If `unshield` is unavailable, provide a clean installation made by the original
installer on Windows or Wine:

```bash
./tools/prepare-working.sh install --from-installed /path/to/clean/install
```

The pipeline creates two independently manifested trees:

- `working/game-clean/`: extracted retail installation.
- `working/game-nocd/`: clean installation with the supplied no-CD executable
  and `CDROMDrive.cfg` applied by script.

Existing output trees are never overwritten automatically.

## Evidence and confidence

- **Confirmed, high confidence:** `data1.cab` identifies as InstallShield
  Cabinet 4/5 and contains names including `Chaos.exe`, `jpeg.dll`, game CFG
  files, creatures, sprites, sounds, and interface assets.
- **Confirmed, high confidence:** the supplied no-CD `CDROMDrive.cfg` points its
  CD-ROM path at `.`; the pipeline therefore copies CD-resident realm and FMV
  assets into each working installation.
- **Confirmed, high confidence:** `unshield 1.6.2` emits 21 file groups. The
  `Shared DLLs` group is empty; the other 20 groups and their destination paths
  match the directory definitions embedded in `data1.cab`.
- **Pending runtime confirmation:** the reconstructed maximum-install tree passes
  structural and hash-manifest checks but still requires a launch smoke test.
