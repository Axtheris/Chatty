# Chatty - AI Chat Assistant

A modern, responsive AI chatbot application built with C++ and Qt Framework, featuring real-time streaming, multiple LLM providers, and rich content support.

## Features

- 🤖 **Multiple LLM Providers**: Support for various AI models through OpenRouter API
- 🔄 **Real-time Streaming**: Live token streaming with tokens-per-second counter
- 📎 **File Attachments**: Support for image and file uploads
- 📝 **Markdown Rendering**: Full markdown support with syntax highlighting
- 🎨 **Modern UI**: Beautiful, responsive interface with dark/light themes
- ⚙️ **Configurable**: Easy API key management and model selection
- 🔒 **Privacy-focused**: Bring your own API key, no data stored externally

## Requirements

- CMake 3.16+
- C++17 compatible compiler
- Qt5.15+ or Qt6 development libraries
- Git (for version control)

## Dependencies

The project uses the following libraries:
- **Qt Framework**: Cross-platform application framework (Qt5 or Qt6)
- **Qt Widgets**: Native GUI components
- **Qt Network**: HTTP requests and networking
- **Qt WebEngine**: Advanced web content rendering (optional)
- **QTextDocument**: Rich text and markdown rendering

## Building

### Windows (Visual Studio)

```bash
git clone --recursive https://github.com/yourusername/Chatty.git
cd Chatty
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### Linux/macOS

```bash
git clone --recursive https://github.com/yourusername/Chatty.git
cd Chatty
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Setup

1. **Get an OpenRouter API Key**:
   - Visit [OpenRouter](https://openrouter.ai/)
   - Create an account and generate an API key
   - Add credits to your account

2. **Configure Chatty**:
   - Launch the application
   - Go to Settings (gear icon)
   - Enter your OpenRouter API key
   - Select your preferred LLM model

## Usage

### Basic Chat
- Type your message in the input field
- Press Enter or click Send
- Watch responses stream in real-time

### File Attachments
- Click the attachment button (📎)
- Select images or text files
- Files are automatically included in your message context

### Markdown Support
- All messages support full markdown formatting
- Code blocks include syntax highlighting
- SCALA and other languages are supported

### Performance Monitoring
- Real-time tokens per second display
- Message timestamp and token count
- Response time tracking

## Configuration

Settings are stored in `config.json` and include:
- API key (encrypted)
- Selected model
- Theme preferences
- Message history settings
- File upload preferences

## Supported Models

Through OpenRouter API, Chatty supports:
- GPT-4 and GPT-3.5 variants
- Claude 2 and Claude Instant
- PaLM 2
- Llama 2 variants
- And many more!

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Qt Company for the excellent Qt Framework
- OpenRouter for providing unified LLM API access
- All the open-source libraries that make this project possible 