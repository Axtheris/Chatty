# Chatty - Modern AI Chat Application

A beautiful, modern Qt-based chat application for interacting with multiple Large Language Models through OpenRouter API. Featuring a Stratify-inspired design with real-time streaming, file attachments, and markdown support.

![Chatty Interface](screenshots/main-interface.png)

## Features

### 🎨 **Modern UI Design**
- **Stratify-inspired interface** with clean cards and modern typography
- **Responsive layout** with sidebar navigation and welcome screen
- **Beautiful animations** and hover effects throughout the application
- **Professional color scheme** with consistent design language

### 💬 **Advanced Chat Functionality**
- **Real-time streaming** responses from LLM providers
- **Token counting** and performance metrics display
- **Message history** with persistent conversation storage
- **Multiple conversation management** with recent chat access

### 🤖 **LLM Provider Support**
- **OpenRouter API integration** supporting 100+ language models
- **Bring your own API key** - no vendor lock-in
- **Model selection** from various providers (OpenAI, Anthropic, Google, etc.)
- **Streaming support** for real-time responses

### 📎 **File Attachments**
- **Image upload** with automatic compression and resizing
- **Document support** for PDF, Word, Excel, PowerPoint files
- **Code file support** for popular programming languages
- **Drag and drop** file attachment interface

### 📝 **Rich Text Support**
- **Full markdown rendering** with syntax highlighting
- **Code block highlighting** for JavaScript, Python, C++, Scala, and more
- **Image previews** within chat messages
- **Clickable links** and proper text formatting

### ⚙️ **Powerful Settings**
- **API key management** with secure encrypted storage
- **Model configuration** and provider selection
- **Theme customization** (light/dark mode ready)
- **Import/export** settings for easy backup

## Screenshots

| Welcome Screen | Chat Interface | Settings |
|---|---|---|
| ![Welcome](screenshots/welcome.png) | ![Chat](screenshots/chat.png) | ![Settings](screenshots/settings.png) |

## Requirements

### System Requirements
- **Windows 10/11**, **macOS 10.15+**, or **Linux** (Ubuntu 20.04+, Fedora 32+)
- **Qt 5.15+** or **Qt 6.2+** development libraries
- **CMake 3.16+**
- **C++17** compatible compiler (GCC 8+, Clang 8+, MSVC 2019+)

### Development Dependencies

#### Windows
```bash
# Install Qt6 from the official installer
# Download from: https://www.qt.io/download-qt-installer

# Or use vcpkg:
vcpkg install qt6-base qt6-tools qt6-networkauth
```

#### Ubuntu/Debian
```bash
# Qt6 (recommended)
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev qt6-l10n-tools cmake build-essential

# Qt5 (alternative)
sudo apt install qt5-default qttools5-dev qttools5-dev-tools cmake build-essential
```

#### Fedora
```bash
# Qt6
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++

# Qt5 (alternative)
sudo dnf install qt5-qtbase-devel qt5-qttools-devel cmake gcc-c++
```

#### macOS
```bash
# Using Homebrew
brew install qt cmake

# Add Qt to PATH (for Qt6)
echo 'export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"' >> ~/.zshrc
```

## Building the Application

### Quick Build (Recommended)

#### Windows
```cmd
# Double-click build.bat or run in PowerShell:
.\build.bat
```

#### Linux/macOS
```bash
# Make script executable and run:
chmod +x build.sh
./build.sh
```

### Manual Build

1. **Clone and navigate to the project**:
   ```bash
   git clone <repository-url>
   cd Chatty
   ```

2. **Create build directory**:
   ```bash
   mkdir build && cd build
   ```

3. **Configure with CMake**:
   ```bash
   # Release build (recommended)
   cmake .. -DCMAKE_BUILD_TYPE=Release
   
   # Debug build (for development)
   cmake .. -DCMAKE_BUILD_TYPE=Debug
   ```

4. **Build the project**:
   ```bash
   # Build with optimal parallel jobs
   cmake --build . --config Release -j
   ```

5. **Run the application**:
   ```bash
   # Windows
   .\bin\Chatty.exe
   
   # Linux/macOS
   ./bin/Chatty
   ```

## Configuration

### First-Time Setup

1. **Get an OpenRouter API Key**:
   - Visit [OpenRouter.ai](https://openrouter.ai)
   - Create an account and generate an API key
   - Fund your account (costs typically $0.001-$0.02 per message)

2. **Configure the Application**:
   - Launch Chatty
   - Click the **Settings** button in the sidebar
   - Enter your OpenRouter API key
   - Select your preferred default model
   - Save settings

### Configuration Files

Settings are stored in platform-specific locations:
- **Windows**: `%APPDATA%/Chatty/settings.ini`
- **Linux**: `~/.config/Chatty/settings.ini`
- **macOS**: `~/Library/Preferences/com.chatty.Chatty.plist`

## Usage Guide

### Starting a Conversation

1. **Welcome Screen**: Use quick action buttons or template cards
2. **Chat Interface**: Type your message and press Enter or click Send
3. **File Attachments**: Drag files into the chat or use the attachment button
4. **Model Selection**: Choose different LLM models from the settings

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+Enter` | Send message |
| `Ctrl+N` | New conversation |
| `Ctrl+S` | Save conversation |
| `Ctrl+,` | Open settings |
| `Ctrl+Q` | Quit application |
| `F11` | Toggle fullscreen |

### File Support

#### Supported Image Formats
- JPEG, PNG, GIF, BMP, WebP, SVG
- Automatic compression and resizing
- Click to preview full-size images

#### Supported Document Formats
- Text files (TXT, MD, CSV)
- Microsoft Office (DOC, DOCX, XLS, XLSX, PPT, PPTX)
- PDF documents
- Code files (C, C++, Java, Python, Scala, JavaScript, etc.)

## Development

### Project Structure

```
Chatty/
├── src/                    # Source files
│   ├── main.cpp           # Application entry point
│   ├── MainWindow.cpp     # Main window implementation
│   ├── ChatWidget.cpp     # Chat interface
│   ├── WelcomeWidget.cpp  # Welcome screen
│   ├── MessageWidget.cpp  # Individual message display
│   ├── OpenRouterAPI.cpp  # API client
│   ├── FileManager.cpp    # File handling
│   ├── MarkdownRenderer.cpp # Markdown processing
│   └── Settings.cpp       # Configuration management
├── include/               # Header files
├── resources/             # Resources (icons, stylesheets)
│   ├── styles/           # QSS stylesheets
│   └── icons/            # Application icons
├── CMakeLists.txt        # Build configuration
├── build.bat            # Windows build script
├── build.sh             # Linux/macOS build script
└── README.md            # This file
```

### Architecture

The application follows a modern Qt architecture with:

- **Signal/Slot communication** for loose coupling
- **Model-View separation** with custom widgets
- **Resource management** using Qt's QRC system
- **Settings persistence** with QSettings
- **Network handling** via QNetworkAccessManager
- **File operations** through Qt's file APIs

### Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Troubleshooting

### Common Issues

**Qt not found during build**:
- Ensure Qt development libraries are installed
- Add Qt to your system PATH
- Specify Qt path: `cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt`

**OpenRouter API errors**:
- Verify your API key is correct
- Check your account has sufficient credits
- Ensure internet connectivity
- Try different models if one is unavailable

**File attachment issues**:
- Check file size limits (default: 10MB)
- Verify file format is supported
- Ensure sufficient disk space

**Performance issues**:
- Close other applications to free memory
- Try smaller image attachments
- Use simpler LLM models for faster responses

### Debug Mode

Build in debug mode for detailed logging:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **Qt Framework** for the excellent cross-platform UI toolkit
- **OpenRouter** for providing access to multiple LLM providers
- **Stratify** for design inspiration
- **The open-source community** for tools and libraries

## Support

For support, feature requests, or bug reports:
- Create an issue on GitHub
- Check the [FAQ](docs/FAQ.md)
- Read the [troubleshooting guide](docs/TROUBLESHOOTING.md)

---

**Built with ❤️ using Qt and modern C++** 