# FlowHook - Asynchronous C++ Automation Engine

FlowHook is a multi-threaded, Linux-native automation engine that eliminates manual compile-and-run workflows by watching filesystem events and executing build commands automatically.

## Architecture Overview

FlowHook employs a structural composition pattern where a `TaskWatcher` central manager encapsulates both a `FileWatcher` (inotify-based event monitor) and a `SessionLogger` (JSON diagnostic recorder). The system operates via a lean callback pattern, passing raw function wrappers to the filesystem interceptor layer.

### Core Components

- **FileWatcher**: Direct Linux inotify interface with background polling threads
- **TaskWatcher**: Orchestration layer managing commands, paths, and hooks
- **SessionLogger**: JSON-based session recording with timestamps and event tracking
- **Task Struct**: Configuration container for directories, commands, and hook scripts

## Prerequisites

- Linux OS (inotify required)
- C++20 compiler (g++ 10+ or clang 12+)
- nlohmann/json library
- Make build system

## Installation

### 1. Install nlohmann/json

```bash
# Ubuntu/Debian
sudo apt-get install nlohmann-json3-dev

# Arch Linux
sudo pacman -S nlohmann-json

# From source (if package unavailable)
git clone https://github.com/nlohmann/json.git
sudo cp -r json/include/nlohmann /usr/local/include/
```

### 2. Clone and Build

```bash
git clone https://github.com/yourusername/flowhook.git
cd flowhook
make
```

The executable will be created at `bin/filewatcher`.

## Project Structure

```
flowhook/
├── include/
│   ├── filewatcher.h
│   ├── task_watcher.h
│   ├── session_logger.h
│   ├── display.h
│   └── error/
│       ├── error.h
│       └── result.h
├── src/
│   ├── main.cpp
│   ├── filewatcher.cpp
│   ├── task_watcher.cpp
│   ├── session_logger.cpp
│   └── display.cpp
├── bin/
│   └── filewatcher (generated)
└── Makefile
```

## Usage

### Basic Workflow

1. **Start FlowHook**
   ```bash
   ./bin/filewatcher
   ```

2. **Configure Task Instance**
   - Enter watch instance name (e.g., "my_project")
   - Enter directory path to monitor (e.g., "/home/user/project/src")

3. **Add Commands**
   ```
   add-command
   > g++ -Wall -o program main.cpp
   ```

4. **Add Paths to Watch**
   ```
   add-path
   > /home/user/project/src
   ```

5. **Add Hook Scripts (Optional)**
   ```
   add-on-success
   > echo "Build successful!"

   add-on-failure
   > notify-send "Build failed!"
   ```

6. **Start Monitoring**
   ```
   start
   ```

### Command Reference

| Command | Description |
|---------|-------------|
| `add-command` | Add build command to execute on file changes |
| `remove-command` | Remove a build command |
| `add-path` | Add file/directory to watch list |
| `remove-path` | Remove a path from watch list |
| `add-on-success` | Add command to execute on successful build |
| `remove-on-success` | Remove success hook |
| `add-on-failure` | Add command to execute on build failure |
| `remove-on-failure` | Remove failure hook |
| `start` | Begin monitoring and auto-execution |
| `stop` | Stop monitoring |
| `help` | Display all commands |
| `exit` | Quit FlowHook |

## How It Works

### Event Detection Pipeline

1. **Inotify Initialization**: `FileWatcher` creates an inotify instance with non-blocking I/O
2. **Watch Registration**: Directories/files are registered using `inotify_add_watch` with masks for `IN_CLOSE_WRITE`, `IN_CREATE`, `IN_DELETE`, `IN_MODIFY`, and `IN_ACCESS`
3. **Background Thread**: Isolated thread runs polling loop with `poll()` system call
4. **Event Handling**: When events occur, they're extracted from kernel buffers and demultiplexed by event mask

### Command Execution

When a filesystem event triggers:

1. **Callback Invocation**: The registered callback (wrapping `TaskWatcher::execute`) receives the event
2. **Command Chaining**: Each command runs as: `cd <working_directory> && timeout 15s <command> 2>&1`
3. **Stream Capture**: `popen()` creates a unidirectional pipe; `fgets()` reads output line-by-line
4. **Security Controls**:
   - 15-second timeout prevents hang conditions
   - STDERR redirected to STDOUT (`2>&1`) for complete capture
   - 64KB log truncation prevents memory exhaustion
5. **Exit Code Analysis**: POSIX macros (`WIFEXITED`/`WEXITSTATUS`) determine true process state
6. **Hook Dispatch**: `on_success` or `on_failure` hooks execute based on exit code
7. **Session Logging**: Event data (path, timestamp, exit code, output) written to JSON log

### Session Logging

Each session generates a JSON log file named `{task_name}.log` containing:

```json
{
    "task_name": "my_project",
    "session-timestamp": "2024-01-15T14:30:00",
    "session_log": [
        {
            "event_type": "modify",
            "file_path": "/home/user/project/src/main.cpp",
            "file_type": "file",
            "event_mask": 8,
            "build-command": ["g++ -Wall -o program main.cpp"],
            "success_code": 0,
            "terminal_msg": "Compilation successful...",
            "timestamp": "2024-01-15T14:30:05"
        }
    ]
}
```

## Security Features

- **Command Timeouts**: 15-second limit prevents infinite hangs
- **Log Truncation**: 64KB per command output prevents memory exhaustion
- **Shell Escaping**: Commands run in isolated subshells
- **RAII Resource Management**: File descriptors and threads cleaned up automatically
- **Non-blocking I/O**: Event loop never blocks on file operations

## Thread Safety

- `std::mutex` guards watch registry operations
- `std::atomic<bool>` controls background thread state
- Independent thread for event polling with joinable cleanup
- Copy constructor deletion prevents resource duplication

## Limitations (Current Version)

- Single event mask (`IN_CLOSE_WRITE`) supported in callback linkage
- WatchEngine top-level orchestrator not yet implemented
- Manual callback registration required (no automatic execution binding)
- Linux-only (depends on inotify)

## Troubleshooting

### "inotify_add_watch failed"
- Check file/directory permissions
- Verify path exists and is accessible
- Maximum inotify watches reached (check `/proc/sys/fs/inotify/max_user_watches`)

### "No commands to execute"
- Use `add-command` before calling `start`
- Commands persist for a single session lifetime

### Build fails with "nlohmann/json.hpp: No such file"
- Install nlohmann/json library
- Verify include path in Makefile (`-Iinclude`)

## Performance Considerations

- **Event Debouncing**: Poll timeout prevents event flooding
- **Buffer Size**: 4096-byte kernel buffer for event queue
- **Thread Overhead**: Single background thread minimizes contention
- **Log I/O**: JSON written only on session stop (buffered in memory)

## Future Planned Enhancements

- WatchEngine class for declarative task configuration
- Using conditional variables for polling loop instead of timeout loop to save CPU cycles
- Configuration file loading (JSON/YAML)

## License

MIT License

## Author

[mnasie | aspiring system engineer](https://github.com/mnasie)

## Used Technologies

- Linux inotify subsystem
- nlohmann/json library
- POSIX pipe and process management APIs

```