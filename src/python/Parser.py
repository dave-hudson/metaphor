import os

from Token import Token, TokenType
from EmbedLexer import EmbedLexer
from MetaphorLexer import MetaphorLexer
from FileAlreadyUsedError import FileAlreadyUsedError
from ASTNode import ASTNode

class Parser:
    """
    Parser class to process tokens and build an Abstract Syntax Tree (AST).

    Attributes:
        syntax_tree (ASTNode): The root node of the AST being constructed.
        parse_errors (list): List of syntax errors encountered during parsing.
        lexers (list): Stack of lexers used for parsing multiple files.
    """
    def __init__(self):
        self.syntax_tree = None
        self.parse_errors = []
        self.lexers = []
        self.previously_seen_files = set()

    def parse(self, filename):
        """
        Parse a file and construct the AST.

        Args:
            file (str): The input file to be parsed.

        Returns:
            bool: True if parsing was successful, False otherwise.
        """
        try:
            self.check_file_not_loaded(filename)
            self.lexers.append(MetaphorLexer(filename))
            token = self.get_next_token()

            if token.type != TokenType.ACTION:
                self.raise_syntax_error(token, "Expected 'Action' keyword")
                return False

            self.syntax_tree = self.parse_action(token)

            token_next = self.get_next_token()
            if token_next.type != TokenType.END_OF_FILE:
                self.raise_syntax_error(token_next, "Unexpected text after 'Action' block")

            return not self.parse_errors
        except FileNotFoundError as e:
            print(f"Error: {e}")
            return False
        except FileAlreadyUsedError as e:
            print(f"Error: {e}")
            return False

    def get_next_token(self):
        """Get the next token from the active lexer."""
        while self.lexers:
            lexer = self.lexers[-1]
            token = lexer.get_next_token()

            if token.type == TokenType.INCLUDE:
                self.parse_include()
            elif token.type == TokenType.EMBED:
                self.parse_embed()
            elif token.type == TokenType.END_OF_FILE:
                self.lexers.pop()
            else:
                return token

        return Token(TokenType.END_OF_FILE, "", "", "", 0, 0)

    def raise_syntax_error(self, token, message):
        """Raise a syntax error and add it to the error list."""
        caret = ""
        for _ in range(1, token.column):
            caret += " "

        error_message = f"{message}: line {token.line}, column {token.column}, " \
            f"file {token.filename}\n{caret}|\n{caret}v\n{token.input}"
        self.parse_errors.append(error_message)

    def get_syntax_tree(self):
        """Return the constructed AST."""
        return self.syntax_tree

    def get_syntax_errors(self):
        """Return the list of syntax errors encountered."""
        return self.parse_errors

    def check_file_not_loaded(self, filename):
        """Check we have not already loaded a file."""
        canonical_filename = os.path.realpath(filename)
        if canonical_filename in self.previously_seen_files:
            raise FileAlreadyUsedError(filename)

        self.previously_seen_files.add(canonical_filename)

    def parse_action(self, token):
        """Parse an action block and construct its AST node."""
        action_node = ASTNode(token)

        seen_token_type = TokenType.NONE

        init_token = self.get_next_token()
        if init_token.type == TokenType.KEYWORD_TEXT:
            action_node.add_child(self.parse_keyword_text(init_token))
            indent_token = self.get_next_token()
            if indent_token.type != TokenType.INDENT:
                self.raise_syntax_error(token, "Expected indent after keyword description " \
                                        "for 'Action' block")
        elif init_token.type != TokenType.INDENT:
            self.raise_syntax_error(token, "Expected description or indent for 'Action' block")

        while True:
            token = self.get_next_token()
            if token.type == TokenType.TEXT:
                if seen_token_type != TokenType.NONE:
                    self.raise_syntax_error(token, "Text must come first in an 'Action' block")

                action_node.add_child(self.parse_text(token))
            elif token.type == TokenType.CONTEXT:
                action_node.add_child(self.parse_context(token))
                seen_token_type = TokenType.CONTEXT
            elif token.type == TokenType.OUTDENT or token.type == TokenType.END_OF_FILE:
                return action_node
            else:
                self.raise_syntax_error(token, f"Unexpected token: {token.value} in " \
                                        "'Action' block")

    def parse_keyword_text(self, token):
        """Parse keyword text."""
        return ASTNode(token)

    def parse_text(self, token):
        """Parse a text block."""
        return ASTNode(token)

    def parse_context(self, token):
        """Parse a Context block."""
        context_node = ASTNode(token)

        seen_token_type = TokenType.NONE

        init_token = self.get_next_token()
        if init_token.type == TokenType.KEYWORD_TEXT:
            context_node.add_child(self.parse_keyword_text(init_token))
            indent_token = self.get_next_token()
            if indent_token.type != TokenType.INDENT:
                self.raise_syntax_error(token, "Expected indent after keyword description " \
                                        "for 'Context' block")
        elif init_token.type != TokenType.INDENT:
            self.raise_syntax_error(token, "Expected description or indent for 'Context' block")

        while True:
            token = self.get_next_token()
            if token.type == TokenType.TEXT:
                if seen_token_type != TokenType.NONE:
                    self.raise_syntax_error(token, "Text must come first in a 'Context' block")

                context_node.add_child(self.parse_text(token))
            elif token.type == TokenType.CONTEXT:
                context_node.add_child(self.parse_context(token))
                seen_token_type = TokenType.CONTEXT
            elif token.type == TokenType.ROLE:
                context_node.add_child(self.parse_role(token))
                seen_token_type = TokenType.ROLE
            elif token.type == TokenType.OUTDENT or token.type == TokenType.END_OF_FILE:
                return context_node
            else:
                self.raise_syntax_error(token, f"Unexpected token: {token.value} in 'Context' block")

    def parse_role(self, token):
        """Parse a Role block."""
        role_node = ASTNode(token)

        init_token = self.get_next_token()
        if init_token.type == TokenType.KEYWORD_TEXT:
            role_node.add_child(self.parse_keyword_text(init_token))
            indent_token = self.get_next_token()
            if indent_token.type != TokenType.INDENT:
                self.raise_syntax_error(token, "Expected indent after keyword description for " \
                                        "'Role' block")
        elif init_token.type != TokenType.INDENT:
            self.raise_syntax_error(token, "Expected description or indent for 'Role' block")

        while True:
            token = self.get_next_token()
            if token.type == TokenType.TEXT:
                role_node.add_child(self.parse_text(token))
            elif token.type == TokenType.OUTDENT or token.type == TokenType.END_OF_FILE:
                return role_node
            else:
                self.raise_syntax_error(token, f"Unexpected token: {token.value} in " \
                                        "'Role' block")

    def parse_include(self):
        """Parse an Include block and load the included file."""
        token_next = self.get_next_token()
        if token_next.type != TokenType.KEYWORD_TEXT:
            self.raise_syntax_error(token_next, "Expected file name for 'Include'")
            return

        filename = token_next.value
        self.check_file_not_loaded(filename)
        self.lexers.append(MetaphorLexer(filename))

    def parse_embed(self):
        """Parse an Embed block and load the embedded file."""
        token_next = self.get_next_token()
        if token_next.type != TokenType.KEYWORD_TEXT:
            self.raise_syntax_error(token_next, "Expected file name for 'Embed'")
            return

        filename = token_next.value
        self.check_file_not_loaded(filename)
        self.lexers.append(EmbedLexer(filename))
