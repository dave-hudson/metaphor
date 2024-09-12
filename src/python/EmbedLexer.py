from Token import Token, TokenType
from Lexer import Lexer

class EmbedLexer(Lexer):
    """
    Lexer for handling embedded content like code blocks.
    """
    def _tokenize(self):
        """Tokenizes the input file and handles embedded content."""
        self.tokens.append(Token(TokenType.TEXT, f"File: {self.filename}", "", self.filename, 0, 1))
        self.tokens.append(Token(TokenType.TEXT, "```", "", self.filename, 0, 1))

        lines = self.input.splitlines()
        for line in lines:
            token = Token(TokenType.TEXT, line, line, self.filename, self.current_line, 1)
            self.tokens.append(token)
            self.current_line += 1

        self.tokens.append(Token(TokenType.TEXT, "```", "", self.filename, self.current_line, 1))
        self.tokens.append(Token(TokenType.END_OF_FILE, "", "", self.filename, self.current_line, 1))
