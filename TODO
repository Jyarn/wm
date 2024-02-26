- [ ] Window manager
 - [x] substructre redirection??
 - [x] create event handler
 - [x] window focus changing
 - [x] override Redirect Flag
 - [x] logging
  - [x] create error handler
  - [x] basic logging
- [x] arg processing
- [x] floating (tinywm-ish)
 - [x] moving windows
 - [x] resizing windows
 - [x] hiding windows
- [x] multi-monitor support (randr)
- [x] fullscreen
- [x] multiple workspaces
- [ ] borders
- [ ] reparenting
- [ ] compositing (xcomposite)
 - [ ] CLI
 - [x] --debug flag
 - [x] --log flag
 - [ ] --monitor
- [ ] EWMH compliance
- [ ] ICCM compliance
- [ ] key chording
- [ ] replace xlib with xcb
- [ ] quality of life features
 - [x] snapping windows
 - [x] seperate tabbing for hidden and non-hidden windows

toolbar

animated wallpaper

clickable desktop icons (kind of like windows)

fluid simulation wallpaper

ISSUES:
[ ] input freezes
 How to reproduce:
  1. Open Steam
  2. Open friends
  3. Switch to different workspace
  4. Open terminal
  1. Create unmanaged window
  2. Switch workspace
  3. Open something
  4. Try to close it
  1. Open Firefox
  2. ...
[x] easyeffects window not appearing
notes: not running, or slowing down execution seems to make it appear => event handler issue
       setup configure request (broken again)
[x] clicking buttons in firefox, and steam don't work
[x] unable to select text in firefox
[ ] unfocusing window makes window dissapear
[x] go back shortcut doesn't work in firefox
[ ] popup windows aren't able to recieve commands
[x] alacritty crashing the window manager
notes: no clue what happened but it got fixed. now whenever it crashes it doesn't give me any debug info :D
[x] typing into (completley?) overlapped windows
note: looks like it only breaks if easyeffects isn't running
segfault wm.c:158
[x] memory leak at wm.c:204
notes: not closing log file
[x] Enabling XSelectInput for newly managed windows causes a BadAccess error when opening alacritty
notes: it looks like the error is coming from XNextEvent instead of XSelectInput which is weird (need to verify on desktop)
On laptop, wm doesn't even start I assume this is the same issue
On laptop, changing the mask in wm_manage to the one used in dwm seems to fix the issue
[x] Closing alacritty causes a BadWindow
[x] dragging stops X from sending events to client D:
notes:
jesus christ finally fixed it
[x] only able to move a window once, after which motion handler doesn't seem to receive events
notes:
cleanup is not always run before if a motion func does not reset
[x] movewindow warps the top left corner of the window to the mouse
notes:
it looks funny
[x] button presses do not register for certain windows
notes: changed grabmode in grabMouse to Sync
[ ] unable to select text

REFACTORS:
 [x] Cleanup coding style
 [x] Refactor out move binds
 [x] Remove redundant function args
 [x] Refactor user function signature to allow use a union instead of a void pointer
[x] Cleanup coding style
[x] Refactor out move binds
[x] Remove redundant function args

OPTIMIZATIONS:
[ ] Remove geom in Client struct
[ ] Optimize stack managment for +-1 incs
