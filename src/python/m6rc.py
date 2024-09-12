import sys
import argparse
from pathlib import Path


class TokenType:
    """
    Enum-like class representing different types of tokens in the source file.
    """
    NONE = 0
    INDENT = 1
    OUTDENT = 2
    INCLUDE = 3
    EMBED = 4
    KEYWORD_TEXT = 5
    TEXT = 6
    TARGET = 7
    SCOPE = 8
    EXAMPLE = 9
    BAD_INDENT = 10
    BAD_OUTDENT = 11
    END_OF_FILE = 12


class Token:
    """
    Represents a token in the input stream.

    Attributes:
        type_ (TokenType): The type of the token (e.g., TEXT, TARGET).
        value (str): The actual string value of the token.
        input_ (str): The entire line of input where the token appears.
        filename (str): The file where the token was read from.
        line (int): The line number in the file where the token is located.
        column (int): The column number where the token starts.
    """
    def __init__(self, type_, value, input_, filename, line, column):
        self.type = type_
        self.value = value
        self.input = input_
        self.filename = filename
        self.line = line
        self.column = column

    def __str__(self):
        return f'Token(type={self.type}, value="{self.value}", line={self.line}, column={self.column})'


class ASTNode:
    """
    Represents a node in the Abstract Syntax Tree (AST).
    
    Attributes:
        token_type (TokenType): The type of the token the node represents.
        value (str): The value associated with the node.
        line (int): The line number where the node is located.
        column (int): The column number where the node starts.
        child_nodes (list): The list of child nodes for this node.
    """
    def __init__(self, token):
        self.token_type = token.type
        self.value = token.value
        self.line = token.line
        self.column = token.column
        self.parent_node = None
        self.child_nodes = []

    def add_child(self, child):
        """Add a child node to this ASTNode."""
        child.parent_node = self
        self.child_nodes.append(child)

    def print_tree(self, level=0):
        """Print the tree structure of this ASTNode for debugging."""
        print("  " * level + self.value)
        for child in self.child_nodes:
            child.print_tree(level + 1)


class Lexer:
    """
    Base Lexer class that handles basic tokenization such as blank lines, whitespace tracking, and generating tokens for input files.
    """
    def __init__(self, filename):
        self.filename = filename
        self.input = self._read_file(filename)
        self.tokens = []
        self.current_line = 1
        self.current_column = 1
        self._tokenize()

    def _read_file(self, filename):
        """Read file content into memory."""
        if not Path(filename).exists():
            raise FileNotFoundError(f"File not found: {filename}")

        with open(filename, 'r') as file:
            return file.read()

    def get_next_token(self):
        """Return the next token from the token list."""
        if self.tokens:
            return self.tokens.pop(0)

        return Token(TokenType.END_OF_FILE, "", "", self.filename, self.current_line, self.current_column)

    def _tokenize(self):
        """Tokenize the input file into tokens (to be customized by subclasses)."""
        raise NotImplementedError("Subclasses must implement their own tokenization logic.")


class MetaphorLexer(Lexer):
    """
    Lexer for handling the metaphor language with its specific syntax, including keywords like Target, Scope, Example, and proper indentation handling.
    """
    def __init__(self, filename, indent_spaces=4):
        """
        Initializes the MetaphorLexer.

        Args:
            filename (str): The filename to be lexed.
            indent_spaces (int): The number of spaces that make up one level of indentation (default is 4).
        """
        self.keyword_map = {
            "Include:": TokenType.INCLUDE,
            "Embed:": TokenType.EMBED,
            "Target:": TokenType.TARGET,
            "Scope:": TokenType.SCOPE,
            "Example:": TokenType.EXAMPLE
        }
        self.seen_non_whitespace = False
        self.indent_column = 1
        self.indent_offset = 0
        self.indent_spaces = indent_spaces
        super().__init__(filename)

    def _tokenize(self):
        """Tokenizes the input file into appropriate tokens."""
        lines = self.input.splitlines()
        for line in lines:
            print(f'{line}')
            self.current_column = 1

            # Process indentation and then read keywords/text
            self._process_indentation(line)
            tokens = self.read_keyword_or_text(line)
            self.tokens.extend(tokens)
            self.current_line += 1

        # Handles remaining outdents as the file has ended.
        while self.indent_column > 1:
            self.tokens.append(Token(TokenType.OUTDENT, "[Outdent]", "", self.filename, self.current_line, self.current_column))
            self.indent_column -= self.indent_spaces

    def _process_indentation(self, line):
        """Processes the indentation of the current line."""
        if len(line) == 0:
            return

        # Calculate the current indentation level (number of leading spaces)
        current_indent_column = len(line) - len(line.lstrip(' ')) + 1

        # Calculate the difference in indentation
        self.indent_offset = current_indent_column - self.indent_column

        # Handle indentation increase (INDENT)
        if self.indent_offset > 0:
            if self.indent_offset % self.indent_spaces != 0:
                self.tokens.append(Token(TokenType.BAD_INDENT, "[Bad Indent]", line, self.filename, self.current_line, self.current_column))
            else:
                while self.indent_offset > 0:
                    self.tokens.append(Token(TokenType.INDENT, "[Indent]", line, self.filename, self.current_line, self.current_column))
                    self.indent_offset -= self.indent_spaces

            self.indent_column = current_indent_column

        # Handle indentation decrease (OUTDENT)
        elif self.indent_offset < 0:
            if abs(self.indent_offset) % self.indent_spaces != 0:
                self.tokens.append(Token(TokenType.BAD_OUTDENT, "[Bad Outdent]", line, self.filename, self.current_line, self.current_column))
            else:
                while self.indent_offset < 0:
                    self.tokens.append(Token(TokenType.OUTDENT, "[Outdent]", line, self.filename, self.current_line, self.current_column))
                    self.indent_offset += self.indent_spaces

            self.indent_column = current_indent_column

    def read_keyword_or_text(self, line):
        """
        Read the next line and determine if it's a keyword followed by text.
        It also processes indentation at the start of the line.

        Args:
            line (str): The current line to process.

        Returns:
            list[Token]: A list of Token instances representing both indentation and text.
        """
        tokens = []
        stripped_line = line.strip()
        if stripped_line:
            # Split the line by spaces to check for a keyword followed by text
            words = stripped_line.split(maxsplit=1)

            # Check if the first word is a recognized keyword
            if words[0] in self.keyword_map:
                # Create a keyword token
                keyword_token = Token(self.keyword_map[words[0]], words[0], stripped_line, self.filename, self.current_line, self.current_column)
                tokens.append(keyword_token)

                # If there is text after the keyword, create a separate text token
                if len(words) > 1:
                    text_token = Token(TokenType.KEYWORD_TEXT, words[1], stripped_line, self.filename, self.current_line, self.current_column + len(words[0]) + 1)
                    tokens.append(text_token)

                return tokens

        # If no keyword is found, treat the whole line as text
        text_token = Token(TokenType.TEXT, stripped_line, stripped_line, self.filename, self.current_line, self.current_column)
        tokens.append(text_token)
        return tokens


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
            token = Token(TokenType.TEXT, line, line, self.filename, self.current_line, self.current_column)
            self.tokens.append(token)
            self.current_line += 1

        self.tokens.append(Token(TokenType.TEXT, "```", "", self.filename, self.current_line, 1))
        self.tokens.append(Token(TokenType.END_OF_FILE, "", "", self.filename, self.current_line, 1))


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

    def parse(self, filename):
        """
        Parse a file and construct the AST.

        Args:
            file (str): The input file to be parsed.

        Returns:
            bool: True if parsing was successful, False otherwise.
        """
        try:
            self.load_file(filename)
            self.lexers.append(MetaphorLexer(filename))
            token = self.get_next_token()

            if token.type != TokenType.TARGET:
                self.raise_syntax_error(token, "Expected 'Target' keyword")
                return False

            self.syntax_tree = self.parse_target(token)

            token_next = self.get_next_token()
            if token_next.type != TokenType.END_OF_FILE:
                self.raise_syntax_error(token_next, "Unexpected text after 'Target' block")

            return not self.parse_errors
        except Exception as e:
            print(f"Error: {e}")
            return False

    def get_next_token(self):
        """Get the next token from the active lexer."""
        while self.lexers:
            lexer = self.lexers[-1]
            token = lexer.get_next_token()
            print(f"token: ", token)

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
        error_message = f"{message}: line {token.line}, column {token.column}, file {token.filename}"
        self.parse_errors.append(error_message)

    def get_syntax_tree(self):
        """Return the constructed AST."""
        return self.syntax_tree

    def get_syntax_errors(self):
        """Return the list of syntax errors encountered."""
        return self.parse_errors

    def load_file(self, filename):
        """Load a file and push a corresponding lexer to the lexer stack."""
        if not Path(filename).exists():
            raise FileNotFoundError(f"File '{filename}' does not exist.")

    def parse_target(self, token):
        """Parse a target block and construct its AST node."""
        target_node = ASTNode(token)

        seen_token_type = TokenType.NONE

        init_token = self.get_next_token()
        if init_token.type == TokenType.KEYWORD_TEXT:
            target_node.add_child(self.parse_keyword_text(init_token))
            indent_token = self.get_next_token()
            if indent_token.type != TokenType.INDENT:
                self.raise_syntax_error(token, "Expected indent after keyword description for 'Target' block")
        elif init_token.type != TokenType.INDENT:
            self.raise_syntax_error(token, "Expected description or indent for 'Target' block")

        while True:
            token = self.get_next_token()
            if token.type == TokenType.TEXT:
                if seen_token_type != TokenType.NONE:
                    self.raise_syntax_error(token, "Text must come first in a 'Target' block")

                target_node.add_child(self.parse_text(token))
            elif token.type == TokenType.SCOPE:
                target_node.add_child(self.parse_scope(token))
                seen_token_type = TokenType.SCOPE
            elif token.type == TokenType.OUTDENT or token.type == TokenType.END_OF_FILE:
                return target_node
            else:
                self.raise_syntax_error(token, f"Unexpected token: {token.value} in 'Target' block")

    def parse_keyword_text(self, token):
        """Parse keyword text."""
        return ASTNode(token)

    def parse_text(self, token):
        """Parse a text block."""
        return ASTNode(token)

    def parse_scope(self, token):
        """Parse a Scope block."""
        scope_node = ASTNode(token)
        init_token = self.get_next_token()
        if init_token.type == TokenType.KEYWORD_TEXT:
            scope_node.add_child(self.parse_keyword_text(init_token))
            indent_token = self.get_next_token()
            if indent_token.type != TokenType.INDENT:
                self.raise_syntax_error(token, "Expected indent after keyword description for 'Scope' block")
        elif init_token.type != TokenType.INDENT:
            self.raise_syntax_error(token, "Expected description or indent for 'Scope' block")

        while True:
            token = self.get_next_token()
            if token.type == TokenType.TEXT:
                scope_node.add_child(self.parse_text(token))
            elif token.type == TokenType.SCOPE:
                scope_node.add_child(self.parse_scope(token))
            elif token.type == TokenType.EXAMPLE:
                scope_node.add_child(self.parse_example(token))
            elif token.type == TokenType.OUTDENT or token.type == TokenType.END_OF_FILE:
                return scope_node
            else:
                self.raise_syntax_error(token, f"Unexpected token: {token.value} in 'Scope' block")

    def parse_example(self, token):
        """Parse an Example block."""
        example_node = ASTNode(token)
        init_token = self.get_next_token()
        if init_token.type == TokenType.KEYWORD_TEXT:
            example_node.add_child(self.parse_keyword_text(init_token))
            indent_token = self.get_next_token()
            if indent_token.type != TokenType.INDENT:
                self.raise_syntax_error(token, "Expected indent after keyword description for 'Example' block")
        elif init_token.type != TokenType.INDENT:
            self.raise_syntax_error(token, "Expected description or indent for 'Example' block")

        while True:
            token = self.get_next_token()
            if token.type == TokenType.TEXT:
                example_node.add_child(self.parse_text(token))
            elif token.type == TokenType.OUTDENT or token.type == TokenType.END_OF_FILE:
                return example_node
            else:
                self.raise_syntax_error(token, f"Unexpected token: {token.value} in 'Example' block")

    def parse_include(self):
        """Parse an Include block and load the included file."""
        token_next = self.get_next_token()
        if token_next.type != TokenType.KEYWORD_TEXT:
            self.raise_syntax_error(token_next, "Expected file name for 'Include'")
            return

        filename = token_next.value
        self.load_file(filename)
        self.lexers.append(MetaphorLexer(filename))

    def parse_embed(self):
        """Parse an Embed block and load the embedded file."""
        token_next = self.get_next_token()
        if token_next.type != TokenType.KEYWORD_TEXT:
            self.raise_syntax_error(token_next, "Expected file name for 'Embed'")
            return

        filename = token_next.value
        self.load_file(filename)
        self.lexers.append(EmbedLexer(filename))


def simplify_text(node):
    """
    Simplify the text content in the AST by merging adjacent text nodes.

    Args:
        node (ASTNode): The current node in the AST being simplified.
    """
    i = 0
    in_formatted_section = False

    while i < len(node.child_nodes):
        child = node.child_nodes[i]

        if child.token_type != TokenType.TEXT:
            simplify_text(child)
            i += 1
            continue

        # Preserve blank lines (empty text nodes)
        if not in_formatted_section and len(child.value) == 0:
            i += 1
            continue

        if i == len(node.child_nodes) - 1:
            i += 1
            continue

        if child.value.startswith("```"):
            in_formatted_section = not in_formatted_section

        sibling = node.child_nodes[i + 1]
        if sibling.token_type != TokenType.TEXT:
            in_formatted_section = False
            i += 1
            continue

        # Merge adjacent text nodes unless a blank line separates them
        if sibling.value == "":
            i += 1
            continue

        child.value += " " + sibling.value
        del node.child_nodes[i + 1]


def recurse(node, section, out):
    """
    Recursively traverse the AST and output formatted sections.

    Args:
        node (ASTNode): The current AST node being processed.
        section (str): The section number (e.g., "1", "1.1").
        out (file): The output stream to write to.
    """
    if node.token_type == TokenType.TEXT:
        out.write(node.value + '\n\n')
        return

    if node.token_type in (TokenType.TARGET, TokenType.SCOPE, TokenType.EXAMPLE):
        if node.child_nodes:
            child = node.child_nodes[0]
            if child.token_type == TokenType.KEYWORD_TEXT:
                out.write(f"{section} {child.value}\n\n")
            else:
                out.write(f"{section}\n\n")
        else:
            out.write(f"{section}\n\n")

    index = 0
    for child in node.child_nodes:
        if child.token_type in (TokenType.SCOPE, TokenType.EXAMPLE):
            index += 1
        recurse(child, f"{section}.{index}", out)


def print_usage(program_name):
    """Print usage information for the program."""
    print(f"Usage: {program_name} [options] <file>")
    print("Options:")
    print("  -h, --help                Print this help message")
    print("  -o, --outputFile <file>   Specify output file")
    print("  -d, --debug               Generate debug output")


def main():
    """Main entry point for the program."""
    parser = argparse.ArgumentParser()
    parser.add_argument("input_file", help="Input file to parse")
    parser.add_argument("-o", "--outputFile", help="Output file")
    parser.add_argument("-d", "--debug", action="store_true", help="Enable debug mode")

    args = parser.parse_args()

    output_file = args.outputFile
    debug = args.debug
    input_file = args.input_file

    if debug:
        print("Debug mode is ON")

    if not Path(input_file).exists():
        print(f"Error: File {input_file} not found")
        return 1

    output_stream = sys.stdout
    if output_file:
        try:
            output_stream = open(output_file, 'w')
        except OSError as e:
            print(f"Error: Could not open output file {output_file}: {e}")
            return 1

    parser = Parser()
    if not parser.parse(input_file):
        for error in parser.get_syntax_errors():
            print(f"Error: {error}")
        return -1

    syntax_tree = parser.get_syntax_tree()
    simplify_text(syntax_tree)
    recurse(syntax_tree, "1", output_stream)

    if output_file:
        output_stream.close()

    return 0


if __name__ == "__main__":
    sys.exit(main())
