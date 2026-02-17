# Contributing to ST3215 C++ Library

Thank you for considering contributing to the ST3215 C++ Library! This document provides guidelines for contributing to the project.

## Development Setup

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- CMake 3.10 or higher
- Git
- A serial device or ST3215 servo for testing (optional but recommended)

### Building from Source

```bash
git clone https://github.com/sinayanaik/python_ST3215.git
cd python_ST3215
mkdir build && cd build
cmake ..
cmake --build .
```

## Code Style

### C++ Guidelines

1. **Modern C++**: Use C++17 features where appropriate
2. **RAII**: Always use RAII for resource management
3. **Smart Pointers**: Prefer `std::unique_ptr` and `std::shared_ptr` over raw pointers
4. **Const Correctness**: Mark member functions as `const` when they don't modify state
5. **Namespace**: All code should be in the `st3215` namespace
6. **Error Handling**: Use `std::optional` for operations that can fail

### Naming Conventions

- **Classes**: PascalCase (e.g., `PortHandler`, `ST3215`)
- **Functions**: camelCase (e.g., `readPosition`, `moveTo`)
- **Variables**: snake_case for member variables with trailing underscore (e.g., `is_open_`, `port_name_`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_POSITION`, `DEFAULT_BAUDRATE`)

### Code Formatting

- Indentation: 4 spaces (no tabs)
- Line length: 120 characters maximum
- Braces: Opening brace on same line for functions and control structures

Example:
```cpp
bool PortHandler::openPort() {
    if (is_open_) {
        return true;
    }
    return setupPort();
}
```

## Making Changes

### Branch Naming

- Feature: `feature/description`
- Bug fix: `fix/description`
- Documentation: `docs/description`

### Commit Messages

Follow conventional commits format:

```
<type>(<scope>): <subject>

<body>

<footer>
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Build system or dependency updates

Example:
```
feat(port_handler): add support for custom baudrates

- Added setBaudRate method
- Updated documentation
- Added unit test for baudrate changes

Closes #42
```

### Pull Request Process

1. Fork the repository
2. Create a feature branch from `main`
3. Make your changes
4. Ensure code compiles without warnings
5. Update documentation if needed
6. Commit your changes with clear commit messages
7. Push to your fork
8. Create a Pull Request

### Pull Request Checklist

- [ ] Code compiles without errors or warnings
- [ ] All existing functionality still works
- [ ] Documentation updated (if applicable)
- [ ] Examples updated (if API changed)
- [ ] Commit messages follow convention
- [ ] PR description clearly explains the changes

## Testing

### Manual Testing

If you have ST3215 servos available:

```bash
# Test basic connectivity
./build/examples/list_servos /dev/ttyUSB0

# Test servo control
./build/examples/ping_servo /dev/ttyUSB0 1
./build/examples/move_servo /dev/ttyUSB0 1 2048
./build/examples/read_telemetry /dev/ttyUSB0 1
```

### Testing Without Hardware

You can still contribute by:
- Reviewing code for potential bugs
- Improving documentation
- Adding comments and improving code clarity
- Optimizing algorithms
- Improving error handling

## Areas for Contribution

### High Priority

- Cross-platform support (Windows, macOS)
- Unit tests with mock serial port
- Performance benchmarks
- Additional examples
- Thread safety improvements

### Medium Priority

- Group sync read/write operations
- Async communication API
- Configuration file support
- Logging framework integration

### Low Priority

- Python bindings
- ROS/ROS2 integration
- GUI control application

## Documentation

### Code Documentation

Use Doxygen-style comments:

```cpp
/**
 * @brief Brief description of the function
 * 
 * Detailed description if needed.
 * 
 * @param param_name Description of parameter
 * @return Description of return value
 * @throws std::runtime_error Description of when this is thrown
 */
ReturnType functionName(ParamType param_name);
```

### README Updates

When adding new features:
1. Update the features list
2. Add usage examples
3. Update API documentation section
4. Add to the examples section if applicable

## Questions?

- Open an issue for bug reports or feature requests
- Use GitHub Discussions for questions about usage
- Check existing issues before creating new ones

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Code of Conduct

### Our Standards

- Be respectful and inclusive
- Accept constructive criticism
- Focus on what's best for the project
- Show empathy towards other contributors

### Unacceptable Behavior

- Harassment or discriminatory language
- Trolling or insulting comments
- Public or private harassment
- Publishing others' private information

Thank you for contributing to make ST3215 C++ Library better!
