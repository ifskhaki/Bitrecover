# Contributing to Bitrecover

Thank you for your interest in contributing to Bitrecover! This document provides guidelines and instructions for contributing.

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help maintain a positive community

## How to Contribute

### Reporting Issues

1. Check if the issue already exists
2. Use a clear, descriptive title
3. Provide detailed steps to reproduce
4. Include system information (OS, GPU, CUDA version)
5. Attach relevant logs or error messages

### Submitting Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Test thoroughly
5. Commit with clear messages (`git commit -m 'Add amazing feature'`)
6. Push to your fork (`git push origin feature/amazing-feature`)
7. Open a Pull Request

### Coding Standards

- Follow existing code style
- Use meaningful variable names
- Add comments for complex logic
- Keep functions focused and small
- Test your changes

### Build and Test

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./bin/bitrecover --list-devices
```

## Areas for Contribution

- Performance optimizations
- Bug fixes
- Documentation improvements
- New features
- Testing
- Code cleanup

## Questions?

Open an issue with the `question` label or start a discussion.

Thank you for contributing! 🎉

