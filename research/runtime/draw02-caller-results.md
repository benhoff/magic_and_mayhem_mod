# DRAW-02 caller-result and failure comparisons

`tools/drawing_api_results.py` compares HRESULT and LastError independently of
pixel equality. Call order, entry identities, method, logical object/alias,
original caller, entry LastError and recorded flags must agree. Pointer-backed
argument contents and complete call-input identity remain a separate gate. Missing calls, unknown
results and identity drift refuse comparison. A successful pixel comparison
cannot accept a different return value or error.

`tools/check-drawing-failure-results.py` supplies owned fault schedules to
`reconstruction/rendering/surface_retry.hpp`, then compares the native events
externally against 4,480 retained unchanged-original/scripted COM traces. The
original wrappers are partial fill `0x58bac0`, full fill `0x58bc10`, opaque copy
`0x58c4a0` and keyed copy `0x58c8a0` in the pinned NoCD build. Expected original
wrapper events remain outside native inputs. Draw, source/destination Restore
and key-set outcomes are controlled driver inputs.

Complete schedules must preserve every event, consumed draw count and returned
outcome. Two-draw bounded runs must preserve the exact original event prefix and
report exhaustion separately from return. All 4,480 complete traces matched;
1,536 bounded runs exhausted their budget. Zero and 65,537 draw budgets are
rejected. Comparator guards cover altered HRESULT/LastError, missing/reordered
calls, unknown results, incorrect Restore outcomes and false successful return
on exhaustion.

The existing native GL restoration fixture separately passed 30 checks: unknown
contents cannot be read/presented/copied, partial writes validate only overwritten
regions, keyed/masked dependencies remain required, complete explicit reloads
restore readable contents, and exhausted recovery does not imply known pixels.
This is an intentional native storage policy and scoped recovered scheduling
comparison. Physical driver loss, restored original pixels/palettes, reentrant
asset reloads and live whole-game recovery remain pending. It does not complete
the 21 operation/failure gates or authorize drawing suppression.
