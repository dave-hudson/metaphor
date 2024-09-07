# **User Manual for the m6rc Compiler**

## **Introduction**
The m6rc compiler is a tool designed to parse, simplify, and process Metaphor language code.  This document outlines
its usage, options, and functionality.  It explains how to compile source files, and produce an output file using the
m6rc compiler.

## What is Metaphor?

Metaphor is a simple declarative language designed to create Maximal Instruction Prompts (MIPs) for Large Language
Models (LLMs).

Metaphor follows a very simple design that captures a target objective for the LLM to fulfil.  This target is supported by a
hierarchical description of the scope the LLM is being asked to use to fulfil the target.

The design is natural language based but this use of natural language is slightly constrained by some keywords so m6rc can
construct more effective MIP prompts.

This approach has many advantages:

- We can iterate from a simple description to a more complex one over time.
- When using this to build software, we can quickly iterate new versions, allowing us to try out new ideas very rapidly,
  prior to committing to them.
- This approach captures the "memory" of what we're trying to achieve in the prompt as opposed to in an interactive dialogue
  with an LLM.  This means we can use the same approach with different LLMs, and can take advantage of "temporary" sessions
  with an LLM so that we don't contaminate the LLM's output based on previous experiments that may not have been fully
  successful.

### Syntax

Metaphor (m6r) files follow a very simple document-like structure.  It has only 5 keywords:

- `Target:` - defines the top-level target objective being conveyed to the LLM.  There is only one `Target:` keyword
  in any given Metaphor input.
- `Scope:` - a hierarchical description of the scope of the work we want the LLM to do and supporting information.
  `Scope:` elements may nest but must only exist within the scope of a `Target:`.
- `Example:` - defines an example of how some `Scope:` item should behave.  `Example:` can only be used within a `Scope:`.
- `Embed:` - embeds an external file into the prompt, also indicating the language involved to the LLM.
- `Inject:` - injects another Metaphor file into the current one, as if that one was directly part of the file being
  procesed, but auto-indented to the current indentation level.

### Indentation

To avoid arguments over indentation, Metaphor supports only one valid indentation strategy.  All nested items must be
indented by exactly 4 spaces.

## Using The Output

When you have generated an output file, copy its contents to the LLM prompt.  This means the same prompt can be reused
multiple times, in case adjustments are required, or if the LLM does something unexpected.  By using temporary chats with the
LLM this makes things more repeatable.

To check the LLM understands the prompt, try asking questions like:

- Is this prompt clear and unambiguous?
- Have you met all of the requirements?

## Command-Line Usage

### Basic Command Syntax

```bash
m6rc [options] <file>
```

Where `<file>` is the path to the input file containing Metaphor language code.

### Options

- **`-h, --help`**: Display help and usage information.
  
- **`-o, --outputFile <file>`**: Specify the output file where the compiler should write its results. If this option is not provided, the output is printed to the console.

- **`-d, --debug`**: Enable debug mode, which prints additional diagnostic information about the parsing process to `stderr`.

## Steps to Compile a File

1. **Prepare the Input File**: Ensure your file is written in Metaphor language and adheres to its syntax rules.
   
2. **Compile the File**: Use the following command to compile the input file:

   ```bash
   m6rc <file>
   ```

   Replace `<file>` with the path to your file. If no output file is specified, the results will be printed on the console.

3. **Specify an Output File (Optional)**:

   If you want the output written to a file, use the `-o` or `--outputFile` option:

   ```bash
   m6rc -o output.txt <file>
   ```

4. **Enable Debug Mode (Optional)**:

   To enable debug output, use the `-d` or `--debug` flag:

   ```bash
   m6rc -d <file>
   ```

   In debug mode, additional information about each token processed is printed to `stderr`.

## Common Use Cases

### Generating an Output File

To compile `example.m6r` and write the output to `result.txt`:

```bash
m6rc -o result.txt example.m6r
```

### Running in Debug Mode

To compile `example.m6r` with debug output:

```bash
m6rc -d example.m6r
```

This will print diagnostic messages to `stderr`.

### Displaying Help

To show the help message with usage instructions:

```bash
m6rc --help
```

## Error Messages

The compiler provides clear and detailed error messages if issues are detected during the parsing process. Errors typically include:
- A description of the error
- Line number
- Column number
- File name

For example:
```
Expected 'Target' keyword: line 10, column 5, file example.m6r
```
