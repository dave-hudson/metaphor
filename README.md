
# **User Manual for the m6rc Compiler**

## **Introduction**
The m6rc compiler is a tool designed to parse, simplify, and process Metaphor language code. This document outlines its usage, options, and functionality. It explains how to compile a source file, generate syntax trees, and produce output files using the m6rc compiler.

---

## **Command-Line Usage**

### **Basic Command Syntax**

```bash
m6rc [options] <file>
```

Where `<file>` is the path to the input file containing Metaphor language code.

### **Options**

- **`-h, --help`**: Display help and usage information.
  
- **`-o, --outputFile <file>`**: Specify the output file where the compiler should write its results. If this option is not provided, the output is printed to the console.

- **`-d, --debug`**: Enable debug mode, which prints additional diagnostic information about the parsing process to `stderr`.

---

## **Steps to Compile a File**

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

---

## **Common Use Cases**

### **Generating an Output File**

To compile `example.m6r` and write the output to `result.txt`:

```bash
m6rc -o result.txt example.m6r
```

### **Running in Debug Mode**

To compile `example.m6r` with debug output:

```bash
m6rc -d example.m6r
```

This will print diagnostic messages to `stderr`.

### **Displaying Help**

To show the help message with usage instructions:

```bash
m6rc --help
```

---

## **Error Messages**

The compiler provides clear and detailed error messages if issues are detected during the parsing process. Errors typically include:
- Line number
- Column number
- A description of the error

For example:
```
Syntax error: Expected 'Target' keyword at line 10, column 5, file example.m6r
```

---

## **Using The Output**

When you have generated an output file, copy its contents to the LLM prompt.  This means the same prompt can be reused
multiple times, in case adjustments are required, or if the LLM does something unexpected.  By using temporary chats with the
LLM this makes things more repeatable.

To check the LLM understands the prompt, try asking questions like:

- Is this prompt clear and unambiguous?
- Have you met all of the requirements?
