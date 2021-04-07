# far

Find and Rename

## Draft

### UI

```ascii
+-------------------------------------------------+
| F A R                                     . ○ × |
+-------------------------------------------------+
| +-----------------------------------+ +-------+ |
| | PATTERN                           | | RUN   | |
| +-----------------------------------+ +-------+ |
| +-----------------------------------+ +-------+ |
| | TEMPLATE                          | | CONF  | |
| +-----------------------------------+ +-------+ |
| +---------------------------------------------+ |
| | TABLE_IMPORT_FILES                          | |
| | TABLE_DELETE_FILES                          | |
| | TABLE_CLEAR_FILES                           | |
| |                                             | |
| |                                             | |
| |                                             | |
| |                                             | |
| |                                             | |
| |                                             | |
| +---------------------------------------------+ |
+-------------------------------------------------+
```

## Miscellaneous

I've switched from ui to walk, then fyne, and finally Gotk3. ([ui](https://github.com/zserge/webview) is too simple. [walk](https://github.com/lxn/walk) looks ok but tableview is too buggy. [fyne](https://github.com/fyne-io/fyne) is nice but didn't support drag and drop.)

- [Golang GUI Libraries](https://golangr.com/gui/)
