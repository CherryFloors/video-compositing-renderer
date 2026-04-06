# In Progress

# TODO
- store all routines that render an asset to a texture or display in vcr_assets.h
- rename display.c to engine.h and add impl def guard
- rename video_player.c to header and add impl def guard
- rename programming_queue.c to to header and add impl def guard
- Refactor the static generator so its a renderer and not a texture generator similar to render_video_frame.
- Need a way to generalize layouts as structs
- Need a way to implement transition routines to animate layouts
- Enable queueing videos and standby

# DONE
- Create a .clangd file to treat .h as .c and define them impl guards so lsp doesnt gray out impls
- digital display data structure with active and inactive textures
- refactor digital display symbol texture creator to set container(rect) and return texture and not require app. Move to assets
- create and destroy routines for new digital display struct
- digital display renderer to draw display given a state struct
- add display to vcr app struct and rename vcr app variables
- animate digital display clock by checking time
