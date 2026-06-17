# mod-auto-gather-tracking

AzerothCore module that automatically keeps herb and mining resource tracking enabled for players who know Herbalism, Mining, or both.

The module updates the server-side `PLAYER_TRACK_RESOURCES` mask, so players can continue tracking other resource types while herb and mineral tracking are restored automatically when appropriate.

## Features

- Automatically tracks herbs for players with Herbalism
- Automatically tracks minerals for players with Mining
- Supports characters with either profession or both professions
- Preserves other minimap resource tracking bits instead of replacing the whole tracking state
- Reapplies tracking on login, map change, zone update, skill changes, and after resource-tracking auras are removed
- Configurable with a module-owned config file

## Installation

1. Place this module in `modules/mod-auto-gather-tracking/`.
2. Re-run CMake so AzerothCore discovers the module.
3. Rebuild `worldserver`.
4. Copy `conf/mod_auto_gather_tracking.conf.dist` to your server config directory as `mod_auto_gather_tracking.conf`.
5. Set `AutoGatherTracking.Enable = 1` in `mod_auto_gather_tracking.conf`.

No database SQL is required.

## Configuration

| Setting | Default | Description |
|---------|---------|-------------|
| `AutoGatherTracking.Enable` | `0` | Enables automatic herb and mining resource tracking for players with the matching profession. |

## Notes

This module does not teach professions, grant tracking spells, or change node spawn behavior. It only keeps the resource tracking mask aligned with the professions the player already knows.

If a player only has Herbalism, only herb tracking is restored. If a player only has Mining, only mineral tracking is restored. If a player has both, both are restored.
