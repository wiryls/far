# far

Find and Rename

## Design

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

### FSM

| Event \ State         | `Idle` | `Import` | `Differ` | `Rename` | `Report` |
| ----------------      | ------ | -------- | -------- | -------- | -------- |
| Exit                  | EXIT | × | EXIT | × | EXIT |
| Pattern Changed       | try `Differ` or . | × | `Differ` | × | `Differ` |
| Pattern Empty         | . | × | `Idle` | × | `Idle` |
| Template Changed      | . | × | `Differ` | × | `Differ` |
| Template Empty        | . | × | `Idle` | × | `Idle` |
| Differ Conf Changed | . | × | `Differ` | × | `Differ` |
| Import Files          | `Import` | × | `Import` | × | `Import` |
| Import Done           | × | try `Differ` or `Idle`  | × | × | × |
| Rename Files          | × | × | try `Rename` or × | × | × |
| Rename Done           | × | × | × | `Report` | × |
| Delete Files          | . | × | . | × | × |
| Clear Files           | . | × | `Idle` | × | `Idle` |
