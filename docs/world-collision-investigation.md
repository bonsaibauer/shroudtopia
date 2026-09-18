# Native paste collision investigation — 2026-09-18

Status: the 2026-09-18 live test confirmed collision for a freshly pasted
region after synchronous full-region readback. Persistence across another game
restart still requires explicit confirmation.

## Evidence in the pinned client

EXE SHA256: AF2F5A1227911D8AA06B3908D6BD0211838211CAE14EA91099CB57D0DF990781.
Addresses below are RVAs.

- The normal edit path at 98BB33 calls E8CB20, the same region writer used by
  WorldEngine. Its immediate cleanup at 991E30 is not a physics commit.
- E8CB20 writes chunks through E8C790 and E8BF40 into E9ABD0.
- E9ABD0 marks subscriber bitsets (E9AFA0), sets store+C1 and, at level 6,
  enqueues changed bounds through E8C1B0 (call at E9B0B2).
- Therefore the assertion that our writer skips *all* change notifications is
  incorrect. The effect of those notifications on physics remains unverified.
- Captured occupied cells were 00C0. This alone does not prove missing density
  or separate Building/ECS colliders. Do not change density speculatively.

## Implemented checks

WriteGrid now reads back and compares the entire aligned region, including
preserved border cells. Fault, unavailable readback, or mismatch returns an
error. A match is explicitly logged as data-only verification.

## Confirmed live result

- F8 pasted `capture-4` at 3747.5 / 843 / 1410.
- The aligned 24x16x16 region contained 6,144 cells; all read back identically.
- The mod writer and the game's own writer used the same world identity
  (`3300277868560`). The mod ran on thread 12556; the observed game edit path
  ran on thread 24352.
- The player reported that the pasted structure blocked movement normally.
- Subsequent native remove events addressed 0.5 m cells in the pasted area,
  which is additional evidence that the game recognized those cells normally.

This establishes current-session collision behavior. It does not by itself
prove save/reload persistence, and it does not isolate whether readback is the
required synchronization mechanism or merely coincided with completed engine
processing.

The bounded diagnostic records latest native_region_write samples at E8CB20:
world identity, thread, caller and caller RVA (zero if outside game image).
It uses the existing register-only mailbox; no transient argument buffers are
dereferenced. Samples may be dropped or overwritten; this is not a full trace.

## Next controlled in-game comparison

Use a disposable test world. Within the five-minute capture window:

1. Place one normal wall/block; wait two seconds.
2. Copy and paste a small matching region; wait two seconds; test collision.
3. Place one normal block touching the pasted region; test collision again.

Compare writer world identities and threads before selecting a fix. If step 3
restores collision, investigate physics cache invalidation. If not, compare
authoritative versus client world contexts and cell encoding. No unverified
native refresh function or speculative public API was added.

Open issue: mod writes run on the loader thread. Its mutex does not synchronize
with the engine. Matching readback does not prove thread safety. A verified
engine dispatch boundary is required before claiming production readiness.
