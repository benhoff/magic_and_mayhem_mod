# Selected native CD compatibility

Behavior `HOST.native-mci-cd-adapter` has passed a synthetic PE32 wrapper/installation fixture. The PE adapter recognizes CDAUDIO open and its owned virtual device,
translates selected TMSF/status/whole-track play/pause/resume/stop/close commands
to the versioned native music worker, and keeps notification HWND/token handling
inside the PE. No host pointer enters the channel. Data-track play is refused;
unsupported native CD commands are counted. Other MCI devices retain a separately
counted legacy path. Native requests and status snapshots are bounded.

The hook guards the MCI import pointer and original CD mixer
enumeration call/volume prologue before mutation. The native policy uses no
desktop mixers; music volume scopes the original0..15 level to its own output.
Original playlist/control functions remain active. The synthetic installer rejects
a changed MCI import, mixer call or volume prologue before mutation; its accepted
patch executes the zero-mixer thunk and the x86 thiscall volume trampoline. A
fixture original-volume tail returns success, so this is ABI/dispatch evidence,
not a comparison of the full original volume function. Level7 reaches native
volume30583.

The actual PE32/native worker fixture passes three plays, a completed-track
notification, explicit stop/abort notifications, pause/resume, virtual track
count/type/length, last-track play, data-track/unsupported/closed-device refusals,
and caller LastError preservation. One intentionally unsupported CD call and one
legacy-other-device call are counted separately. The synthetic PostMessage
callback validates HWND/message/kind/device arguments; real window delivery is
pending. Readiness is withdrawn on native shutdown. Evidence is
[native-mci-cd-synthetic-20261012.json](native-mci-cd-synthetic-20261012.json).

Live installation, original driver/window notifications, actual output loss and
listening, original timing, arbitrary MCI/CD endpoints, complete protocol abuse
branches and whole WinMM/library retirement remain separate milestones. Failed
freestanding fixture compilation and the earlier wrapper-only result are retained
under working/tests/native-mci-cd; their hashes have not been refreshed.
