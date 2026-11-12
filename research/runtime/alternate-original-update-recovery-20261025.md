# Official update and alternate-canvas owner recovery

The supplied official update now yields a reproducible, unmodified version1.100
executable. Its alternate-canvas code matches the supplied NoCD image. This
closes a concrete recovery alternative: the NoCD edits did not remove the known
alternate initialization, display rebuild, graphics shutdown, canvas retirement
or copy bodies. It does **not** recover an original alternate allocator.

## Inputs and extraction

The immutable update `original/magicmayhemuksp1.exe` has SHA-256
`90a032fef35f7ffc21fa9cf9aea991147e9c2f92a5b550b63e6768825d8c80b3`.
Its617,641-byte inner program is compressed after the PE sections; its footer
contains `XCKJ` and file offset `0x4a00`. Ordinary7z archive listing refuses it.
`tools/unpack-original-update.py` executes its unchanged x86 extractor from
`401000` with Unicorn2.1.4 and bounded owned memory/file endpoints. It stops at
the original `CreateProcessA` request before launching the inner program.
Executable section bytes remain unchanged. No operating-system file requests
are forwarded. The payload SHA-256 is
`308a503536018b2246f4802a4f51c25f0d78d2e34371f6c76132c890043ef770`.
The [accepted extraction](alternate-original-results-20261025/update04.json)
and exact payload are retained in SQLite.

The inner program identifies itself as the1.000-to-1.100 upgrade. Resource
`20530/19274/1033` contains a length-prefixed original `PATCHW32.dll`. The
updater's own command construction supplies `/z /noa`, its program filename and
the update directory. `tools/apply-original-update.py` extracts that resource
and builds the small [owned host](../../tests/original-update-host.c). It calls
the original `RTPatchApply32@12` synchronously with a null callback and wait=1,
using that command syntax against a fresh full copy of the clean retail tree.

The retail executable is pinned to
`124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214`.
The clean installation's4,834 manifest entries are verified before and after.
The original manifest is verified before/after every producer. A read-only root
sandbox exposes only this owned output and the exclusively leased marked Wine
prefix for writing. The prefix is idle on admission and stopped before release.
Neither the game nor a shared installation is launched or patched.

The [accepted application](alternate-original-results-20261025/apply03.json)
produces2,076,755-byte `Chaos.exe`, SHA-256
`405001ab761c3d9bc3007cc9dfa2210ab66f429e37440eb9108ffcee658e8809`.
Its exact bytes are retained in SQLite. The official update also changes files
in its owned installation copy; those files are analysis inputs, not native
gameplay or balance changes.

## Official1.100 versus NoCD

[owner01](alternate-original-results-20261025/owner01.json) compares every stored
section byte and the complete section layout with NoCD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
`.rdata`, `.data` and `.rsrc` are identical. The only16 differing `.text` bytes
are conditional branches replaced by NOPs in the drive/media-checking code:

| Original address | Differing bytes |
| --- | ---: |
| `482335` |6 |
| `482365` |2 |
| `4823e1` |6 |
| `4825f8` |2 |

The nearby original `GetDriveTypeA` import and exact branch instructions are
recorded. This is a byte-level finding, not an assertion that arbitrary
upstream execution is identical between the two builds.

Six complete, inventory-bounded bodies are byte-identical: display rebuild
`4a88f0`, initializer `4e40d0`, graphics shutdown `58acf0`, canvas retirement
`58b3e0`, integer copy `594508` and x87 copy `59455c`. All five reviewed globals
are loader-zero-filled in the same virtual data tail. The exhaustive full-file
DWORD-literal classifier finds the same five absolute reads of `6e1f68` and
two reads of `6e1f6c`; it finds no literal publisher or release.

Confidence is high for reconstruction, hashes, exact byte differences and
those complete-body identities. No original code was repaired or substituted
to obtain the result. This supplemental static image is not a new registered
game execution or original/native equivalence result.

## Ownership boundary

The original allocator, natural nonzero publication, initial backing pixels,
base/source relationship, complete writer inventory and retirement ownership
remain unrecovered. Computed/bulk/imported/external writes and unresolved
indirect edges remain open. The unchanged official image rules out removal of
the reviewed bodies by the NoCD edits; it does not prove global dormancy or that
an owner never existed. The existing native owner and its live integration are
separate policy and keep their previous validation.

Further original closure requires identifying an actual original publisher
and tracing its backing generation, or a sound reachability/alias argument
covering the remaining writer frontier. A different original build containing
the owner would be another useful input. A literal-count result supplies neither
of those proofs. `RS.alternate-canvas-provenance` therefore remains partial.

## Refusals, retention and reproduction

update01 refused an overly strict owned `GetModuleFileNameA` endpoint, which
initially admitted only a null handle rather than the original image handle.
update02/03 retained instruction-budget refusals while decompression progressed.
update04 completes within its finite500-million-instruction/60-second budgets.
apply01 retained the official patcher's missing `Cfg/chatmessage.cfg` refusal
against an executable-only copy. apply02 refused a TSV header interpreted as a
file; apply03 verifies the actual file entries and completes. No failed result
is promoted. The launcher storage guard also retained a stdout refusal from a
relative tool path; all executed experiments use absolute tool paths.

[retention01](alternate-original-results-20261025/retention01.json) records exact
current/intermediate tool bytes matching all attempted source fingerprints,
the baseline manifest,25 raw host/compiler/log members and their hashes. The
unpacked payload and reconstructed executable are direct SQLite objects. Full
owned installations and all refused outputs remain under `working/.retain`.
No cleanup or evidence publication was performed.

The [final audit](alternate-original-results-20261025/audit02.json) has no errors
and retains the existing historical-staleness warnings. Original ownership stays
partial, with current implementation validation pending the expanded scope.
Python syntax and corrupted-image refusal checks pass. The first manual control
invocation omitted the tools module path and refused before execution; its
corrected invocation passes. The first audit's summary refresh refused the
immutable existing report; its old reference was retained before refreshing the
generated summary through the SQLite writer. Neither refusal changes engine
validation. [checks01](alternate-original-results-20261025/checks01.json) verifies
the retained raw members and exact attempt-source payloads.

Run the unpacker with Unicorn2.1.4 on Python's module path and a fresh report.
Run the application tool with a fresh `--output`, `--report`, verified `--retail`
and `--retail-manifest`, and an idle marked `--wine-prefix` with its
`--prefix-owner-root`. Use the storage guard, private temporary filesystem and
read-only input mounts. The review tool consumes the accepted SQLite payload
and original manifest. The fixed accepted-record inputs are deliberate pins;
new attempts use new report paths and retain their original bytes.
