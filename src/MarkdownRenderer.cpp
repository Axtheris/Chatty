#include "MarkdownRenderer.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QStringList>
#include <QDebug>

MarkdownRenderer::MarkdownRenderer(QObject* parent)
    : QObject(parent)
{
    initializeSyntaxHighlighting();
}

QString MarkdownRenderer::renderMarkdown(const QString& markdown) const
{
    QString html = markdown;
    
    // Convert markdown to HTML
    html = processCodeBlocks(html);
    html = processInlineCode(html);
    html = processHeaders(html);
    html = processBold(html);
    html = processItalic(html);
    html = processStrikethrough(html);
    html = processLinks(html);
    html = processImages(html);
    html = processLists(html);
    html = processBlockquotes(html);
    html = processHorizontalRules(html);
    html = processParagraphs(html);
    
    return wrapInDiv(html);
}

QString MarkdownRenderer::processCodeBlocks(const QString& text) const
{
    QString result = text;
    
    // Match fenced code blocks with optional language
    QRegularExpression codeBlockRegex(R"(```(\w+)?\n?(.*?)\n?```)");
    codeBlockRegex.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    
    QRegularExpressionMatchIterator iterator = codeBlockRegex.globalMatch(result);
    
    // Process matches in reverse order to avoid offset issues
    QList<QRegularExpressionMatch> matches;
    while (iterator.hasNext()) {
        matches.prepend(iterator.next());
    }
    
    for (const auto& match : matches) {
        QString language = match.captured(1);
        QString code = match.captured(2);
        
        QString highlightedCode = highlightCode(code, language);
        QString codeBlockHtml = QString(
            "<div class=\"code-block\">"
            "<div class=\"code-header\">%1</div>"
            "<pre class=\"code-content\"><code class=\"language-%2\">%3</code></pre>"
            "</div>"
        ).arg(language.isEmpty() ? "Code" : language.toUpper())
         .arg(language.isEmpty() ? "text" : language)
         .arg(highlightedCode);
        
        result.replace(match.capturedStart(), match.capturedLength(), codeBlockHtml);
    }
    
    return result;
}

QString MarkdownRenderer::processInlineCode(const QString& text) const
{
    QString result = text;
    
    QRegularExpression inlineCodeRegex(R"(`([^`]+)`)");
    result.replace(inlineCodeRegex, R"(<code class="inline-code">\1</code>)");
    
    return result;
}

QString MarkdownRenderer::processHeaders(const QString& text) const
{
    QString result = text;
    QStringList lines = result.split('\n');
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        
        // ATX-style headers (# ## ###)
        QRegularExpression headerRegex(R"(^(#{1,6})\s+(.+)$)");
        QRegularExpressionMatch match = headerRegex.match(line);
        
        if (match.hasMatch()) {
            int level = match.captured(1).length();
            QString content = match.captured(2);
            lines[i] = QString("<h%1>%2</h%1>").arg(level).arg(content);
        }
    }
    
    return lines.join('\n');
}

QString MarkdownRenderer::processBold(const QString& text) const
{
    QString result = text;
    
    // **bold** or __bold__
    QRegularExpression boldRegex(R"(\*\*([^\*]+)\*\*|__([^_]+)__)");
    result.replace(boldRegex, R"(<strong>\1\2</strong>)");
    
    return result;
}

QString MarkdownRenderer::processItalic(const QString& text) const
{
    QString result = text;
    
    // *italic* or _italic_ (but not part of bold)
    QRegularExpression italicRegex(R"((?<!\*)\*([^\*]+)\*(?!\*)|(?<!_)_([^_]+)_(?!_))");
    result.replace(italicRegex, R"(<em>\1\2</em>)");
    
    return result;
}

QString MarkdownRenderer::processStrikethrough(const QString& text) const
{
    QString result = text;
    
    // ~~strikethrough~~
    QRegularExpression strikeRegex(R"(~~([^~]+)~~)");
    result.replace(strikeRegex, R"(<del>\1</del>)");
    
    return result;
}

QString MarkdownRenderer::processLinks(const QString& text) const
{
    QString result = text;
    
    // [text](url) format
    QRegularExpression linkRegex(R"(\[([^\]]+)\]\(([^\)]+)\))");
    result.replace(linkRegex, R"(<a href="\2" target="_blank">\1</a>)");
    
    // Auto-link URLs
    QRegularExpression urlRegex(R"(\b(?:https?://|www\.)[^\s<>"]+)");
    QRegularExpressionMatchIterator iterator = urlRegex.globalMatch(result);
    
    QList<QRegularExpressionMatch> matches;
    while (iterator.hasNext()) {
        matches.prepend(iterator.next());
    }
    
    for (const auto& match : matches) {
        QString url = match.captured(0);
        QString href = url.startsWith("www.") ? "http://" + url : url;
        QString linkHtml = QString(R"(<a href="%1" target="_blank">%2</a>)").arg(href).arg(url);
        result.replace(match.capturedStart(), match.capturedLength(), linkHtml);
    }
    
    return result;
}

QString MarkdownRenderer::processImages(const QString& text) const
{
    QString result = text;
    
    // ![alt](src) format
    QRegularExpression imageRegex(R"(!\[([^\]]*)\]\(([^\)]+)\))");
    result.replace(imageRegex, R"(<img src="\2" alt="\1" class="markdown-image" />)");
    
    return result;
}

QString MarkdownRenderer::processLists(const QString& text) const
{
    QString result = text;
    QStringList lines = result.split('\n');
    
    bool inOrderedList = false;
    bool inUnorderedList = false;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        
        // Ordered list (1. 2. 3.)
        QRegularExpression orderedRegex(R"(^\s*\d+\.\s+(.+)$)");
        QRegularExpressionMatch orderedMatch = orderedRegex.match(line);
        
        if (orderedMatch.hasMatch()) {
            if (!inOrderedList) {
                if (inUnorderedList) {
                    lines[i-1] += "</ul>";
                    inUnorderedList = false;
                }
                lines[i] = "<ol><li>" + orderedMatch.captured(1) + "</li>";
                inOrderedList = true;
            } else {
                lines[i] = "<li>" + orderedMatch.captured(1) + "</li>";
            }
            continue;
        }
        
        // Unordered list (- * +)
        QRegularExpression unorderedRegex(R"(^\s*[-\*\+]\s+(.+)$)");
        QRegularExpressionMatch unorderedMatch = unorderedRegex.match(line);
        
        if (unorderedMatch.hasMatch()) {
            if (!inUnorderedList) {
                if (inOrderedList) {
                    lines[i-1] += "</ol>";
                    inOrderedList = false;
                }
                lines[i] = "<ul><li>" + unorderedMatch.captured(1) + "</li>";
                inUnorderedList = true;
            } else {
                lines[i] = "<li>" + unorderedMatch.captured(1) + "</li>";
            }
            continue;
        }
        
        // End lists if we encounter a non-list line
        if (inOrderedList) {
            lines[i-1] += "</ol>";
            inOrderedList = false;
        }
        if (inUnorderedList) {
            lines[i-1] += "</ul>";
            inUnorderedList = false;
        }
    }
    
    // Close any remaining open lists
    if (inOrderedList) {
        lines.last() += "</ol>";
    }
    if (inUnorderedList) {
        lines.last() += "</ul>";
    }
    
    return lines.join('\n');
}

QString MarkdownRenderer::processBlockquotes(const QString& text) const
{
    QString result = text;
    QStringList lines = result.split('\n');
    
    bool inBlockquote = false;
    QStringList blockquoteLines;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        
        QRegularExpression blockquoteRegex(R"(^>\s*(.*)$)");
        QRegularExpressionMatch match = blockquoteRegex.match(line);
        
        if (match.hasMatch()) {
            if (!inBlockquote) {
                inBlockquote = true;
                blockquoteLines.clear();
            }
            blockquoteLines.append(match.captured(1));
            lines[i] = ""; // Mark for removal
        } else {
            if (inBlockquote) {
                // End blockquote
                QString blockquoteContent = blockquoteLines.join("<br>");
                lines[i-1] = QString("<blockquote>%1</blockquote>").arg(blockquoteContent);
                inBlockquote = false;
            }
        }
    }
    
    // Handle blockquote at end of text
    if (inBlockquote) {
        QString blockquoteContent = blockquoteLines.join("<br>");
        lines.append(QString("<blockquote>%1</blockquote>").arg(blockquoteContent));
    }
    
    // Remove empty lines marked for removal
    lines.removeAll("");
    
    return lines.join('\n');
}

QString MarkdownRenderer::processHorizontalRules(const QString& text) const
{
    QString result = text;
    
    // --- or *** or ___
    QRegularExpression hrRegex(R"(^(?:\*{3,}|-{3,}|_{3,})$)");
    result.replace(hrRegex, "<hr>");
    
    return result;
}

QString MarkdownRenderer::processParagraphs(const QString& text) const
{
    QString result = text;
    QStringList lines = result.split('\n');
    QStringList processedLines;
    
    QString currentParagraph;
    
    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();
        
        // Skip HTML tags and special elements
        if (trimmedLine.startsWith('<') || trimmedLine.isEmpty()) {
            if (!currentParagraph.isEmpty()) {
                processedLines.append(QString("<p>%1</p>").arg(currentParagraph.trimmed()));
                currentParagraph.clear();
            }
            if (!trimmedLine.isEmpty()) {
                processedLines.append(line);
            }
        } else {
            if (!currentParagraph.isEmpty()) {
                currentParagraph += " ";
            }
            currentParagraph += trimmedLine;
        }
    }
    
    // Handle final paragraph
    if (!currentParagraph.isEmpty()) {
        processedLines.append(QString("<p>%1</p>").arg(currentParagraph.trimmed()));
    }
    
    return processedLines.join('\n');
}

QString MarkdownRenderer::highlightCode(const QString& code, const QString& language) const
{
    QString result = code.toHtmlEscaped();
    
    if (language.isEmpty()) {
        return result;
    }
    
    // Get syntax highlighting rules for the language
    if (m_syntaxRules.contains(language.toLower())) {
        const auto& rules = m_syntaxRules[language.toLower()];
        
        for (const auto& rule : rules) {
            result.replace(rule.pattern, rule.replacement);
        }
    }
    
    return result;
}

QString MarkdownRenderer::wrapInDiv(const QString& html) const
{
    return QString(
        "<div class=\"markdown-content\">"
        "%1"
        "</div>"
        "<style>"
        ".markdown-content {"
        "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', system-ui, sans-serif;"
        "    line-height: 1.6;"
        "    color: #374151;"
        "}"
        ".markdown-content h1, .markdown-content h2, .markdown-content h3, "
        ".markdown-content h4, .markdown-content h5, .markdown-content h6 {"
        "    margin: 1.5em 0 0.5em 0;"
        "    font-weight: 600;"
        "    color: #111827;"
        "}"
        ".markdown-content h1 { font-size: 1.5em; }"
        ".markdown-content h2 { font-size: 1.3em; }"
        ".markdown-content h3 { font-size: 1.1em; }"
        ".markdown-content p {"
        "    margin: 0.5em 0;"
        "}"
        ".markdown-content code.inline-code {"
        "    background-color: #F3F4F6;"
        "    padding: 2px 4px;"
        "    border-radius: 3px;"
        "    font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;"
        "    font-size: 0.9em;"
        "}"
        ".code-block {"
        "    margin: 1em 0;"
        "    border-radius: 8px;"
        "    overflow: hidden;"
        "    border: 1px solid #E5E7EB;"
        "}"
        ".code-header {"
        "    background-color: #F9FAFB;"
        "    padding: 8px 12px;"
        "    font-size: 0.8em;"
        "    font-weight: 600;"
        "    color: #6B7280;"
        "    border-bottom: 1px solid #E5E7EB;"
        "}"
        ".code-content {"
        "    background-color: #1F2937;"
        "    color: #F9FAFB;"
        "    padding: 12px;"
        "    margin: 0;"
        "    overflow-x: auto;"
        "    font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;"
        "    font-size: 0.9em;"
        "    line-height: 1.4;"
        "}"
        ".markdown-content blockquote {"
        "    border-left: 4px solid #3B82F6;"
        "    margin: 1em 0;"
        "    padding: 0.5em 1em;"
        "    background-color: #F8FAFC;"
        "    color: #64748B;"
        "    font-style: italic;"
        "}"
        ".markdown-content ul, .markdown-content ol {"
        "    margin: 0.5em 0;"
        "    padding-left: 2em;"
        "}"
        ".markdown-content li {"
        "    margin: 0.25em 0;"
        "}"
        ".markdown-content a {"
        "    color: #3B82F6;"
        "    text-decoration: none;"
        "}"
        ".markdown-content a:hover {"
        "    text-decoration: underline;"
        "}"
        ".markdown-content hr {"
        "    border: none;"
        "    border-top: 2px solid #E5E7EB;"
        "    margin: 2em 0;"
        "}"
        ".markdown-content .markdown-image {"
        "    max-width: 100%;"
        "    height: auto;"
        "    border-radius: 8px;"
        "    margin: 1em 0;"
        "}"
        ".syntax-keyword { color: #F59E0B; font-weight: bold; }"
        ".syntax-string { color: #10B981; }"
        ".syntax-comment { color: #6B7280; font-style: italic; }"
        ".syntax-number { color: #8B5CF6; }"
        ".syntax-operator { color: #EF4444; }"
        "</style>"
    ).arg(html);
}

void MarkdownRenderer::initializeSyntaxHighlighting()
{
    // JavaScript/TypeScript highlighting
    QList<SyntaxRule> jsRules = {
        {QRegularExpression(R"(\b(const|let|var|function|class|if|else|for|while|return|import|export|async|await|try|catch|finally)\b)"), 
         R"(<span class="syntax-keyword">\1</span>)"},
        {QRegularExpression(R"(('([^'\\]|\\.)*'|"([^"\\]|\\.)*"|`([^`\\]|\\.)*`))"), 
         R"(<span class="syntax-string">\1</span>)"},
        {QRegularExpression(R"(//.*$)", QRegularExpression::MultilineOption), 
         R"(<span class="syntax-comment">\0</span>)"},
        {QRegularExpression(R"(/\*.*?\*/)", QRegularExpression::DotMatchesEverythingOption), 
         R"(<span class="syntax-comment">\0</span>)"},
        {QRegularExpression(R"(\b\d+(\.\d+)?\b)"), 
         R"(<span class="syntax-number">\0</span>)"}
    };
    
    // Python highlighting
    QList<SyntaxRule> pythonRules = {
        {QRegularExpression(R"(\b(def|class|if|elif|else|for|while|return|import|from|try|except|finally|with|as|pass|break|continue|lambda|and|or|not|in|is)\b)"), 
         R"(<span class="syntax-keyword">\1</span>)"},
        {QRegularExpression(R"(('([^'\\]|\\.)*'|"([^"\\]|\\.)*"|'''.*?'''|""".*?"""))", QRegularExpression::DotMatchesEverythingOption), 
         R"(<span class="syntax-string">\1</span>)"},
        {QRegularExpression(R"(#.*$)", QRegularExpression::MultilineOption), 
         R"(<span class="syntax-comment">\0</span>)"},
        {QRegularExpression(R"(\b\d+(\.\d+)?\b)"), 
         R"(<span class="syntax-number">\0</span>)"}
    };
    
    // C++ highlighting
    QList<SyntaxRule> cppRules = {
        {QRegularExpression(R"(\b(int|float|double|char|bool|void|class|struct|namespace|using|template|typename|const|static|virtual|override|public|private|protected|if|else|for|while|return|include|define)\b)"), 
         R"(<span class="syntax-keyword">\1</span>)"},
        {QRegularExpression(R"(("([^"\\]|\\.)*"))"), 
         R"(<span class="syntax-string">\1</span>)"},
        {QRegularExpression(R"(//.*$)", QRegularExpression::MultilineOption), 
         R"(<span class="syntax-comment">\0</span>)"},
        {QRegularExpression(R"(/\*.*?\*/)", QRegularExpression::DotMatchesEverythingOption), 
         R"(<span class="syntax-comment">\0</span>)"},
        {QRegularExpression(R"(\b\d+(\.\d+)?[fFLl]?\b)"), 
         R"(<span class="syntax-number">\0</span>)"}
    };
    
    // Scala highlighting
    QList<SyntaxRule> scalaRules = {
        {QRegularExpression(R"(\b(val|var|def|class|object|trait|extends|with|case|match|if|else|for|while|return|import|package|private|protected|override|abstract|sealed|final|lazy|implicit)\b)"), 
         R"(<span class="syntax-keyword">\1</span>)"},
        {QRegularExpression(R"(("([^"\\]|\\.)*"|'([^'\\]|\\.)*'))"), 
         R"(<span class="syntax-string">\1</span>)"},
        {QRegularExpression(R"(//.*$)", QRegularExpression::MultilineOption), 
         R"(<span class="syntax-comment">\0</span>)"},
        {QRegularExpression(R"(/\*.*?\*/)", QRegularExpression::DotMatchesEverythingOption), 
         R"(<span class="syntax-comment">\0</span>)"},
        {QRegularExpression(R"(\b\d+(\.\d+)?[fFLl]?\b)"), 
         R"(<span class="syntax-number">\0</span>)"}
    };
    
    m_syntaxRules["javascript"] = jsRules;
    m_syntaxRules["typescript"] = jsRules;
    m_syntaxRules["js"] = jsRules;
    m_syntaxRules["ts"] = jsRules;
    m_syntaxRules["python"] = pythonRules;
    m_syntaxRules["py"] = pythonRules;
    m_syntaxRules["cpp"] = cppRules;
    m_syntaxRules["c++"] = cppRules;
    m_syntaxRules["c"] = cppRules;
    m_syntaxRules["scala"] = scalaRules;
} 