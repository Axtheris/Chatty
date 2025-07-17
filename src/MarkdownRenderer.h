#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QFont>
#include <QColor>
#include <QHash>
#include <vector>
#include <memory>

enum class SyntaxLanguage {
    None,
    CPP,
    C,
    Python,
    JavaScript,
    TypeScript,
    Scala,
    Java,
    Rust,
    Go,
    JSON,
    XML,
    HTML,
    CSS,
    SQL,
    Bash,
    PowerShell
};

struct SyntaxToken {
    QString text;
    int type; // 0=normal, 1=keyword, 2=string, 3=comment, 4=number, 5=operator
    int start;
    int end;
};

struct CodeBlock {
    QString language;
    QString code;
    std::vector<SyntaxToken> tokens;
    bool highlighted = false;
};

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit SyntaxHighlighter(QTextDocument *parent, SyntaxLanguage language);
    void setLanguage(SyntaxLanguage language);
    void setTheme(bool darkMode);

protected:
    void highlightBlock(const QString &text) override;

private:
    void setupRules();
    
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    
    QVector<HighlightingRule> m_highlightingRules;
    SyntaxLanguage m_language;
    bool m_darkMode;
};

class MarkdownRenderer : public QObject {
    Q_OBJECT

public:
    explicit MarkdownRenderer(QObject *parent = nullptr);
    ~MarkdownRenderer();
    
    // Main rendering
    QTextDocument* renderMarkdown(const QString& markdown, int maxWidth = -1);
    QTextDocument* renderCodeBlock(const CodeBlock& block, int maxWidth = -1);
    
    // Parsing
    QVector<CodeBlock> extractCodeBlocks(const QString& markdown);
    QString parseMarkdownToPlainText(const QString& markdown);
    
    // Syntax highlighting
    void highlightCode(CodeBlock& block);
    SyntaxLanguage detectLanguage(const QString& languageStr);
    
    // Configuration
    void setCodeFont(const QFont& font);
    void setTheme(bool darkMode = true);
    void setMaxWidth(int width) { m_maxWidth = width; }
    
private:
    int m_maxWidth = 600;
    bool m_darkMode = true;
    
    // Font management
    QFont m_codeFont;
    QFont m_regularFont;
    
    // Syntax highlighting patterns
    QHash<SyntaxLanguage, QStringList> m_keywords;
    
    // Text formats
    QTextCharFormat m_normalFormat;
    QTextCharFormat m_headingFormat;
    QTextCharFormat m_boldFormat;
    QTextCharFormat m_italicFormat;
    QTextCharFormat m_strikeFormat;
    QTextCharFormat m_codeFormat;
    QTextCharFormat m_linkFormat;
    QTextCharFormat m_quoteFormat;
    
    // Methods
    void initializeKeywords();
    void initializeFormats();
    void processMarkdownLine(QTextCursor& cursor, const QString& line);
    void insertText(QTextCursor& cursor, const QString& text, const QTextCharFormat& format);
    void insertHeading(QTextCursor& cursor, const QString& text, int level);
    void insertListItem(QTextCursor& cursor, const QString& text, bool ordered = false);
    void insertQuote(QTextCursor& cursor, const QString& text);
    void insertHorizontalRule(QTextCursor& cursor);
    void insertCodeBlock(QTextCursor& cursor, const CodeBlock& block);
    void insertInlineCode(QTextCursor& cursor, const QString& code);
    
    // Syntax highlighting helpers
    std::vector<SyntaxToken> tokenizeCode(const QString& code, SyntaxLanguage language);
    bool isKeyword(const QString& word, SyntaxLanguage language);
    bool isNumber(const QString& text);
    bool isString(const QString& text);
    bool isComment(const QString& text, SyntaxLanguage language);
    
    // Utility
    QString trimWhitespace(const QString& str);
    QStringList splitLines(const QString& text);
    
    // Colors (will be set based on theme)
    QColor m_colorText;
    QColor m_colorHeading;
    QColor m_colorKeyword;
    QColor m_colorString;
    QColor m_colorComment;
    QColor m_colorNumber;
    QColor m_colorOperator;
    QColor m_colorBackground;
    QColor m_colorCodeBlock;
    QColor m_colorQuote;
    
    void updateColors();
}; 