from pathlib import Path

from Token import Token, TokenType

class Lexer:
    """
    Base Lexer class that handles basic tokenization such as blank lines, whitespace tracking, and
    generating tokens for input files.
    """
    def __init__(self, filename):
        self.filename = filename
        self.input = self._read_file(filename)
        self.tokens = []
        self.current_line = 1
        self._tokenize()

    def _read_file(self, filename):
        """Read file content into memory."""
        if not Path(filename).exists():
            raise FileNotFoundError(f"File not found: {filename}")

        try:
            with open(filename, 'r', encoding='utf-8') as file:
                return file.read()
        except FileNotFoundError:
            print(f"File not found: {filename}")
            return
        except PermissionError:
            print(f"You do not have permission to access: {filename}")
            return
        except IsADirectoryError:
            print(f"Is a directory: {filename}")
            return
        except OSError as e:
            print(f"OS error: {e}")
            return

    def get_next_token(self):
        """Return the next token from the token list."""
        if self.tokens:
            return self.tokens.pop(0)

        return Token(TokenType.END_OF_FILE, "", "", self.filename, self.current_line, 1)

    def _tokenize(self):
        """Tokenize the input file into tokens (to be customized by subclasses)."""
        raise NotImplementedError("Subclasses must implement their own tokenization logic.")
