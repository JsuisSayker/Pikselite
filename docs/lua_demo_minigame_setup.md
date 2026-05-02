# Lua Demo Mini-Game Setup

This mini-game is designed to validate all exposed Lua runtime APIs.

## Scenario

Goal:
1. Control Player.
2. Collect Crystal.
3. Reach ExitGate.
4. Avoid Hunter and mines.

Quick controls:
- WASD / Arrows: move
- Left Shift: sprint
- Space: create pixel + create pixels ring
- E: create decoy entity
- Q: delete decoy entity
- R: remove/add Player sprite component
- F: spawn a static mine

## Required Scene Entities

Create these entities in the editor with exactly these names and components.

### 1) Player
- Name: `Player`
- Components:
1. Transform: enabled=true, x=120, y=120, rotation=0, scaleX=1, scaleY=1
2. Velocity: enabled=true, vx=0, vy=0
3. Sprite: enabled=true, texturePath=`assets/dragon.png`, width=36, height=36
4. PhysicsBody: enabled=true, bodyType=dynamic, fixedRotation=true, density=1.0, friction=0.3, restitution=0.0
5. Script: enabled=true, scriptPath=`scripts/demo/player.lua`

### 2) Hunter
- Name: `Hunter`
- Components:
1. Transform: enabled=true, x=420, y=230, rotation=0, scaleX=1, scaleY=1
2. Velocity: enabled=true, vx=0, vy=0
3. Sprite: enabled=true, texturePath=`assets/icon.bmp`, width=30, height=30
4. PhysicsBody: enabled=true, bodyType=dynamic, fixedRotation=true, density=1.0, friction=0.2, restitution=0.0
5. Script: enabled=true, scriptPath=`scripts/demo/hunter.lua`

### 3) Crystal
- Name: `Crystal`
- Components:
1. Transform: enabled=true, x=300, y=120, rotation=0, scaleX=1, scaleY=1
2. Sprite: enabled=true, texturePath=`assets/icon.bmp`, width=20, height=20
3. PhysicsBody: enabled=true, bodyType=static, fixedRotation=true, density=1.0, friction=0.0, restitution=0.0
4. Script: enabled=true, scriptPath=`scripts/demo/crystal.lua`

### 4) ExitGate
- Name: `ExitGate`
- Components:
1. Transform: enabled=true, x=560, y=120, rotation=0, scaleX=1, scaleY=1
2. Sprite: enabled=true, texturePath=`assets/dragon.png`, width=44, height=44
3. PhysicsBody: enabled=true, bodyType=static, fixedRotation=true, density=1.0, friction=0.0, restitution=0.0

### 5) GameManager
- Name: `GameManager`
- Components:
1. Transform: enabled=true, x=0, y=0, rotation=0, scaleX=1, scaleY=1
2. Script: enabled=true, scriptPath=`scripts/demo/game_manager.lua`

## API Coverage Checklist

The demo uses every Lua function:
1. is_key_pressed -> Player
2. log -> all demo scripts
3. get_entity -> Player/Hunter/Crystal callbacks
4. get_entity_by_name -> Player/Hunter
5. get_entities -> GameManager
6. create_entity -> Player/GameManager
7. delete_entity -> Player/GameManager/Player collision flow
8. has_component -> Player/Hunter
9. get_component -> Player/Hunter
10. set_component -> Player/Hunter/Player gate visual switch
11. add_component -> Player (restore sprite)
12. remove_component -> Player (toggle sprite)
13. create_pixel -> Player
14. create_pixels -> Player/GameManager
15. basic_chase -> Hunter
16. set_victory -> Player
17. set_lose -> Player/Hunter
18. is_victory -> Player/Hunter
19. is_lose -> Player/Hunter
20. on_collision_enter -> Player/Hunter/Crystal
21. on_collision_exit -> Player/Hunter
22. on_victory -> Player/Hunter/Crystal/GameManager
23. on_lose -> Player/Hunter/Crystal/GameManager

## Expected Test Flow

1. Start preview, check logs for GameManager startup and spawned TimedMine.
2. Move Player and ensure velocity updates are visible in behavior.
3. Press Space to paint pixels near Player.
4. Press E to spawn Decoy, Q to remove it.
5. Press R to remove then restore Player sprite.
6. Press F to spawn a Mine entity.
7. Hunter should chase Player using basic_chase.
8. Touch Crystal, then touch ExitGate to trigger victory.
9. If Hunter or Mine touches Player first, lose should trigger.
