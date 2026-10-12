# Native Crocodile melee lab

A stationary Crocodile-versus-Crocodile encounter using original attack ANI
and verified ordinary damage/RNG/health projections. Full original AI, reach,
stun effects and campaign equivalence remain pending; authored skirmish rules
are unchanged. No Wine or original executable is required at runtime.

```sh
python3 tools/working-retention.py run --reserve-gib 2 -- cmake -S apps/melee-lab -B working/build/melee-ns24/normal -DCMAKE_BUILD_TYPE=Debug
python3 tools/working-retention.py run --reserve-gib 2 -- cmake --build working/build/melee-ns24/normal -j6 --target mnm-melee-lab mnm-melee-cli
working/build/melee-ns24/normal/mnm-melee-lab --installation working/game-clean
```

Play/Pause/Step, restart and self-contained checkpoint save/load are available.
The headless CLI creates/resumes the SAME native service:

```sh
working/build/melee-ns24/normal/mnm-melee-cli new working/game-clean 13 1 working/tests/melee-mid.mnm
working/build/melee-ns24/normal/mnm-melee-cli resume working/tests/melee-mid.mnm 4096 0 working/tests/melee-final.mnm
working/build/melee-ns24/normal/mnm-melee-lab --load working/tests/melee-mid.mnm
```

Output paths must be new. Seed is a DWORD; tick runs are bounded at4096.
Restart uses the launch seed. Loaded encounters replay their complete saved RNG,
not a reseeded approximation. See `research/runtime/ordinary-melee.md` and the
native melee checkpoint format for precise boundaries and validation evidence.
