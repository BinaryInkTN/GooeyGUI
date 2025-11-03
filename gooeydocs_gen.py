#!/usr/bin/env python3

import os
import re
import argparse
import json
import shutil
from pathlib import Path
from typing import List, Dict, Tuple, Optional, Any
from dataclasses import dataclass
import html
from datetime import datetime
import markdown
from reportlab.lib.pagesizes import letter, A4
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib import colors
from reportlab.lib.units import inch
import sys
import time

@dataclass
class FunctionParam:
    type: str
    name: str
    description: str = ""

@dataclass
class StructMember:
    type: str
    name: str
    description: str = ""

@dataclass
class FunctionInfo:
    name: str
    return_type: str
    parameters: List[FunctionParam]
    description: str
    brief_description: str = ""
    file: str = ""
    line_number: int = 0
    is_public: bool = True

@dataclass
class DataTypeInfo:
    name: str
    type: str
    definition: str
    description: str = ""
    brief_description: str = ""
    file: str = ""
    line_number: int = 0
    members: List[StructMember] = None

    def __post_init__(self):
        if self.members is None:
            self.members = []

@dataclass
class HeaderFile:
    filename: str
    functions: List[FunctionInfo]
    data_types: List[DataTypeInfo]
    description: str = ""
    brief_description: str = ""
    file_path: str = ""

    def __post_init__(self):
        self.functions.sort(key=lambda x: x.name)
        self.data_types.sort(key=lambda x: x.name)

@dataclass
class MarkdownPage:
    filename: str
    title: str
    content: str
    html_content: str = ""
    order: int = 999
    section: str = "Guides"

@dataclass
class FileTreeNode:
    name: str
    path: str
    children: List['FileTreeNode']
    is_directory: bool
    header_file: Optional[HeaderFile] = None
    markdown_page: Optional[MarkdownPage] = None
    level: int = 0

class DocumentationGenerator:
    def __init__(self):
        self.headers: List[HeaderFile] = []
        self.markdown_pages: List[MarkdownPage] = []
        self.file_tree: Optional[FileTreeNode] = None
        self.config = {
            'project_name': 'Gooey',
            'project_version': '1.0.2',
            'output_dir': 'website/docs',
            'search_dirs': ['include'],
            'markdown_dirs': ['.'],
            'exclude_patterns': ['*_internal.h', '*_private.h', '*.c', '*.cpp'],
            'exclude_dirs': ['.git', 'build', 'dist', 'node_modules', '__pycache__', 'third_party'],
            'include_private': False,
            'generate_index': True,
            'generate_search': True,
            'generate_pdf': False,
            'pdf_output': 'documentation.pdf',
            'theme': 'auto'
        }

    def print_progress(self, current, total, message=""):
        bar_length = 40
        progress = float(current) / float(total)
        block = int(round(bar_length * progress))
        text = "\r[{0}] {1}/{2} {3}".format(
            "=" * block + "-" * (bar_length - block),
            current, total, message
        )
        sys.stdout.write(text)
        sys.stdout.flush()

    def load_config(self, config_file: str = "docs_config.json"):
        if os.path.exists(config_file):
            with open(config_file, 'r') as f:
                user_config = json.load(f)
                self.config.update(user_config)

    def find_header_files(self) -> List[str]:
        header_files = []
        processed_files = set()

        for search_dir in self.config['search_dirs']:
            if not os.path.exists(search_dir):
                continue
            for root, dirs, files in os.walk(search_dir):
                dirs[:] = [d for d in dirs if d not in self.config['exclude_dirs']]
                for file in files:
                    if file.endswith('.h') and not self._should_exclude_file(file):
                        full_path = os.path.join(root, file)
                        abs_path = os.path.abspath(full_path)
                        real_path = os.path.realpath(abs_path)

                        if real_path not in processed_files:
                            header_files.append(real_path)
                            processed_files.add(real_path)
        return header_files

    def find_markdown_files(self) -> List[str]:
        markdown_files = []
        processed_files = set()

        for markdown_dir in self.config['markdown_dirs']:
            if not os.path.exists(markdown_dir):
                continue
            for root, dirs, files in os.walk(markdown_dir):
                dirs[:] = [d for d in dirs if d not in self.config['exclude_dirs']]
                for file in files:
                    if file.endswith('.md'):
                        full_path = os.path.join(root, file)
                        abs_path = os.path.abspath(full_path)
                        real_path = os.path.realpath(abs_path)

                        if real_path not in processed_files:
                            markdown_files.append(real_path)
                            processed_files.add(real_path)
        return markdown_files

    def _should_exclude_file(self, filename: str) -> bool:
        for pattern in self.config['exclude_patterns']:
            if pattern.startswith('*') and filename.endswith(pattern[1:]):
                return True
            if pattern == filename:
                return True
        return False

    def parse_header_files(self) -> List[HeaderFile]:
        print("Parsing header files...")
        header_files = self.find_header_files()
        parsed_headers = []

        processed_files = {}

        for i, header_file in enumerate(header_files):
            self.print_progress(i + 1, len(header_files), f"Parsing {os.path.basename(header_file)}")
            parsed_header = self.parse_single_header(header_file)
            if parsed_header:
                if parsed_header.filename not in processed_files:
                    parsed_headers.append(parsed_header)
                    processed_files[parsed_header.filename] = parsed_header
                else:
                    print(f"  Warning: Skipping duplicate file {parsed_header.filename}")

        print()
        self.headers = parsed_headers
        return parsed_headers

    def parse_markdown_files(self) -> List[MarkdownPage]:
        print("Parsing markdown files...")
        markdown_files = self.find_markdown_files()
        parsed_pages = []

        processed_files = {}

        for i, md_file in enumerate(markdown_files):
            self.print_progress(i + 1, len(markdown_files), f"Parsing {os.path.basename(md_file)}")
            parsed_page = self.parse_single_markdown(md_file)
            if parsed_page:
                if parsed_page.filename not in processed_files:
                    parsed_pages.append(parsed_page)
                    processed_files[parsed_page.filename] = parsed_page
                else:
                    print(f"  Warning: Skipping duplicate file {parsed_page.filename}")

        print()
        parsed_pages.sort(key=lambda x: x.order)
        self.markdown_pages = parsed_pages
        return parsed_pages

    def parse_single_header(self, file_path: str) -> Optional[HeaderFile]:
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
                lines = content.split('\n')

            filename = os.path.basename(file_path)
            file_doc = self._extract_file_documentation(content)
            functions = self._extract_functions_with_docs_advanced(lines, filename)
            data_types = self._extract_data_types_with_docs(lines, filename, content)

            return HeaderFile(
                filename=filename,
                functions=functions,
                data_types=data_types,
                description=file_doc.get('description', ''),
                brief_description=file_doc.get('brief', ''),
                file_path=file_path
            )
        except Exception as e:
            print(f"Error parsing {file_path}: {e}")
            return None

    def _clean_html_content(self, html_content: str) -> str:
        return html_content

    def parse_single_markdown(self, file_path: str) -> Optional[MarkdownPage]:
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()

            filename = os.path.basename(file_path)
            title = self._extract_markdown_title(content, filename)
            order, section = self._extract_markdown_metadata(content)

            content_without_frontmatter = re.sub(
                r'^---\s*\n.*?\n---\s*\n', '', content, flags=re.DOTALL
            ) if '---' in content.split('\n')[0] else content

            content_cleaned = content_without_frontmatter
            escape_chars = ['#', '*', '_', '`', '[', ']', '(', ')', '{', '}', '+', '-', '.', '!']

            for char in escape_chars:
                content_cleaned = content_cleaned.replace(f'\\{char}', char)

            content_cleaned = content_cleaned.replace('\\\\#', '#')

            html_content = markdown.markdown(
                content,
                extensions=[
                    'extra',
                    'toc',
                    'tables',
                    'fenced_code',
                    'codehilite',
                    'admonition',
                    'nl2br',
                    'sane_lists',
                    'smarty'
                ],
                extension_configs={
                    'codehilite': {
                        'css_class': 'highlight',
                        'guess_lang': True
                    },
                    'toc': {
                        'title': 'Table of Contents',
                        'permalink': True
                    }
                }
            )

            return MarkdownPage(
                filename=filename,
                title=title,
                content=content,
                html_content=html_content,
                order=order,
                section=section
            )
        except Exception as e:
            print(f"Error parsing {file_path}: {e}")
            import traceback
            traceback.print_exc()
            return None

    def _extract_markdown_title(self, content: str, filename: str) -> str:
        first_heading = re.search(r'^#\s+(.+)$', content, re.MULTILINE)
        if first_heading:
            return first_heading.group(1).strip()

        name_without_ext = os.path.splitext(filename)[0]
        return name_without_ext.replace('_', ' ').replace('-', ' ').title()

    def _extract_markdown_metadata(self, content: str) -> Tuple[int, str]:
        order, section = 999, "Guides"

        frontmatter_match = re.search(r'^---\s*\n(.*?)\n---\s*\n', content, flags=re.DOTALL)
        if frontmatter_match:
            frontmatter = frontmatter_match.group(1)
            order_match = re.search(r'order:\s*(\d+)', frontmatter)
            section_match = re.search(r'section:\s*"([^"]+)"', frontmatter)
            if order_match:
                order = int(order_match.group(1))
            if section_match:
                section = section_match.group(1)

        return order, section

    def _extract_file_documentation(self, content: str) -> Dict[str, str]:
        file_doc = {'brief': '', 'description': ''}
        file_comment_match = re.search(r'/\*\*.*?@file.*?\*/(.*?)(?=/\*|\w|#)', content, re.DOTALL)
        if file_comment_match:
            file_doc = self._parse_doxygen_comment(file_comment_match.group(0))
        return file_doc

    def _extract_functions_with_docs_advanced(self, lines: List[str], filename: str) -> List[FunctionInfo]:
        functions = []
        i = 0
        while i < len(lines):
            line = lines[i]
            if '/**' in line:
                comment_start = i
                comment_lines = []
                while i < len(lines) and '*/' not in lines[i]:
                    comment_lines.append(lines[i])
                    i += 1
                if i < len(lines) and '*/' in lines[i]:
                    comment_lines.append(lines[i])
                    comment_content = '\n'.join(comment_lines)

                    func_start = i + 1
                    while func_start < len(lines) and not self._looks_like_function_declaration(lines[func_start]):
                        func_start += 1

                    if func_start < len(lines):
                        func_lines = []
                        j = func_start
                        paren_count = 0
                        found_semicolon = False

                        while j < len(lines) and not found_semicolon:
                            current_line = lines[j]
                            func_lines.append(current_line)

                            paren_count += current_line.count('(') - current_line.count(')')

                            if ';' in current_line and paren_count <= 0:
                                found_semicolon = True
                                break
                            j += 1

                        if found_semicolon:
                            func_content = '\n'.join(func_lines)
                            func_info = self._parse_function_declaration(func_content, comment_content, filename)
                            if func_info:
                                functions.append(func_info)
                                i = j
            i += 1
        return functions

    def _extract_data_types_with_docs(self, lines: List[str], filename: str, content: str) -> List[DataTypeInfo]:
        data_types = []
        i = 0
        while i < len(lines):
            line = lines[i]
            if '/**' in line:
                comment_start = i
                comment_lines = []
                while i < len(lines) and '*/' not in lines[i]:
                    comment_lines.append(lines[i])
                    i += 1
                if i < len(lines) and '*/' in lines[i]:
                    comment_lines.append(lines[i])
                    comment_content = '\n'.join(comment_lines)
                    type_start = i + 1

                    found_type = False
                    for lookahead in range(type_start, min(type_start + 50, len(lines))):
                        type_line = lines[lookahead].strip()
                        if type_line and not type_line.startswith('//') and not type_line.startswith('/*'):
                            if any(keyword in type_line for keyword in ['struct', 'enum', 'union', 'typedef']):
                                data_type = self._parse_data_type_definition(
                                    type_line, comment_content, filename, content, lookahead
                                )
                                if data_type:
                                    data_types.append(data_type)
                                    found_type = True
                                    i = lookahead
                                break
                    if not found_type:
                        i += 1
                else:
                    i += 1
            else:
                i += 1
        return data_types

    def _parse_data_type_definition(self, type_line: str, comment_content: str,
                                    filename: str, full_content: str, line_index: int) -> Optional[DataTypeInfo]:
        doc_info = self._parse_doxygen_comment(comment_content)
        lines = full_content.split('\n')

        definition_lines = []
        i = line_index
        brace_count = 0
        found_opening_brace = False
        found_semicolon = False

        while i < len(lines) and not found_semicolon:
            current_line = lines[i]
            definition_lines.append(current_line)

            if '{' in current_line:
                found_opening_brace = True
                brace_count += current_line.count('{')

            brace_count -= current_line.count('}')

            if not found_opening_brace and ';' in current_line:
                found_semicolon = True
                break

            if found_opening_brace and brace_count == 0 and ';' in current_line:
                found_semicolon = True
                break

            i += 1
            if i - line_index > 200:
                break

        full_definition = '\n'.join(definition_lines).strip()

        struct_match = re.search(r'struct\s+(\w+)', type_line)
        if struct_match:
            struct_name = struct_match.group(1)
            members = self._parse_struct_members(full_content, line_index, doc_info.get('members', {}))
            return DataTypeInfo(
                name=struct_name,
                type='struct',
                definition=full_definition,
                description=doc_info.get('description', ''),
                brief_description=doc_info.get('brief', ''),
                file=filename,
                members=members
            )

        typedef_struct_match = re.search(r'typedef\s+struct\s*(\w*)\s*\{', type_line)
        if typedef_struct_match:
            struct_name_match = re.search(r'}\s*(\w+)\s*;', full_definition)
            if struct_name_match:
                struct_name = struct_name_match.group(1)
                members = self._parse_struct_members(full_content, line_index, doc_info.get('members', {}))
                return DataTypeInfo(
                    name=struct_name,
                    type='struct',
                    definition=full_definition,
                    description=doc_info.get('description', ''),
                    brief_description=doc_info.get('brief', ''),
                    file=filename,
                    members=members
                )

        enum_match = re.search(r'enum\s+(\w+)', type_line)
        if enum_match:
            return DataTypeInfo(
                name=enum_match.group(1),
                type='enum',
                definition=full_definition,
                description=doc_info.get('description', ''),
                brief_description=doc_info.get('brief', ''),
                file=filename
            )

        typedef_enum_match = re.search(r'typedef\s+enum\s*\{', type_line)
        if typedef_enum_match:
            enum_name_match = re.search(r'}\s*(\w+)\s*;', full_definition)
            if enum_name_match:
                return DataTypeInfo(
                    name=enum_name_match.group(1),
                    type='enum',
                    definition=full_definition,
                    description=doc_info.get('description', ''),
                    brief_description=doc_info.get('brief', ''),
                    file=filename
                )

        union_match = re.search(r'union\s+(\w+)', type_line)
        if union_match:
            return DataTypeInfo(
                name=union_match.group(1),
                type='union',
                definition=full_definition,
                description=doc_info.get('description', ''),
                brief_description=doc_info.get('brief', ''),
                file=filename
            )

        typedef_match = re.search(r'typedef\s+.*?\s+(\w+)\s*;', full_definition, re.DOTALL)
        if typedef_match:
            return DataTypeInfo(
                name=typedef_match.group(1),
                type='typedef',
                definition=full_definition,
                description=doc_info.get('description', ''),
                brief_description=doc_info.get('brief', ''),
                file=filename
            )

        return None

    def _parse_struct_members(self, content: str, start_line: int, member_docs: Dict[str, str]) -> List[StructMember]:
        lines = content.split('\n')
        members = []
        brace_count = 0
        i = start_line

        while i < len(lines) and '{' not in lines[i]:
            i += 1

        if i >= len(lines):
            return members

        brace_count = lines[i].count('{') - lines[i].count('}')
        i += 1

        while i < len(lines) and brace_count > 0:
            line = lines[i].strip()

            brace_count += line.count('{') - line.count('}')

            if (not line or
                    line.startswith('//') or
                    line.startswith('/*') or
                    line.startswith('*') or
                    line.startswith('}') or
                    line == '{'):
                i += 1
                continue

            if brace_count <= 0:
                break

            if ';' in line:
                member_line = line.split(';')[0].strip()
                if '//' in member_line:
                    member_line = member_line.split('//')[0].strip()

                if member_line and not member_line.startswith('}'):
                    member_line = re.sub(r'/\*.*?\*/', '', member_line)
                    member_line = member_line.strip()

                    if member_line:
                        parts = member_line.rsplit(None, 1)
                        if len(parts) >= 2:
                            member_type = parts[0].strip()
                            member_name = parts[-1].strip()

                            member_name = re.sub(r'\[.*?\]', '', member_name)

                            members.append(StructMember(
                                type=member_type,
                                name=member_name,
                                description=member_docs.get(member_name, "")
                            ))

            i += 1

        return members

    def _looks_like_function_declaration(self, line: str) -> bool:
        line = line.strip()
        if not line or line.startswith('//') or line.startswith('/*') or line.startswith('*') or line.startswith('#'):
            return False

        has_parens = '(' in line and ')' in line
        has_semicolon = ';' in line
        has_equals = '=' in line

        return has_parens and (has_semicolon or line.endswith(')')) and not has_equals

    def _parse_function_declaration(self, func_content: str, comment_content: str, filename: str) -> Optional[FunctionInfo]:
        func_content = re.sub(r'\s+', ' ', func_content).strip()
        
        # Enhanced function pattern to handle complex function pointers and multi-line declarations
        func_patterns = [
            # Standard function with function pointer parameters
            r'^(.+?)\s+(\w+)\s*\(([^)]*)\)\s*;',
            # Function with pointer return type
            r'^(.+?\s*\*+)\s+(\w+)\s*\(([^)]*)\)\s*;',
            # Function with pointer in name
            r'^(.+?)\s+(\*\w+)\s*\(([^)]*)\)\s*;',
            # Multi-line function declaration
            r'^(.+?)\s+(\w+)\s*\((?:[^)]|\n)*\)\s*;'
        ]

        for pattern in func_patterns:
            match = re.search(pattern, func_content, re.DOTALL)
            if match:
                return_type = match.group(1).strip()
                func_name = match.group(2).strip()
                params_str = match.group(3).strip() if match.group(3) else ""

                # Clean up multi-line parameter strings
                params_str = re.sub(r'\s+', ' ', params_str)

                if not self.config['include_private'] and (func_name.startswith('_') or 'internal' in func_name.lower()):
                    return None

                doc_info = self._parse_doxygen_comment(comment_content)
                parameters = self._parse_function_parameters_advanced(params_str, doc_info.get('params', {}))
                
                return FunctionInfo(
                    name=func_name,
                    return_type=return_type,
                    parameters=parameters,
                    description=doc_info.get('description', ''),
                    brief_description=doc_info.get('brief', ''),
                    file=filename
                )

        return None

    def _parse_function_parameters_advanced(self, params_str: str, param_docs: Dict[str, str]) -> List[FunctionParam]:
        if not params_str or params_str == 'void':
            return []
        
        parameters = []
        param_list = self._split_parameters_advanced(params_str)
        
        for param in param_list:
            if param.strip():
                param_type, param_name = self._parse_parameter_advanced(param.strip())
                if param_type:
                    parameters.append(FunctionParam(
                        type=param_type,
                        name=param_name,
                        description=param_docs.get(param_name, "")
                    ))
        return parameters

    def _split_parameters_advanced(self, params_str: str) -> List[str]:
        params, current_param, paren_depth, bracket_depth, angle_depth = [], "", 0, 0, 0
        
        i = 0
        while i < len(params_str):
            char = params_str[i]
            
            if char == '(': 
                paren_depth += 1
            elif char == ')': 
                paren_depth -= 1
            elif char == '[': 
                bracket_depth += 1
            elif char == ']': 
                bracket_depth -= 1
            elif char == '<':
                angle_depth += 1
            elif char == '>':
                angle_depth -= 1
            elif char == ',' and paren_depth == 0 and bracket_depth == 0 and angle_depth == 0:
                params.append(current_param.strip())
                current_param = ""
                i += 1
                continue
            
            current_param += char
            i += 1
        
        if current_param.strip():
            params.append(current_param.strip())
        
        return params

    def _parse_parameter_advanced(self, param: str) -> Tuple[str, str]:
        param = param.strip()
        
        # Handle function pointer parameters
        func_ptr_pattern = r'^(.*?\(\s*\*\s*(\w*)\s*\)\s*\([^)]*\))\s*(?:\s*(\w+))?$'
        func_ptr_match = re.match(func_ptr_pattern, param)
        if func_ptr_match:
            param_type = func_ptr_match.group(1).strip()
            param_name = func_ptr_match.group(3) or ""
            return param_type, param_name
        
        # Handle complex types with templates/pointers
        if '<' in param and '>' in param:
            # For template types, find the last word as parameter name
            words = param.split()
            if words and words[-1].isidentifier():
                param_name = words[-1]
                param_type = ' '.join(words[:-1])
                return param_type, param_name
        
        # Standard parameter parsing
        pattern = r'^((?:const\s+|volatile\s+|static\s+|extern\s+)*\s*(?:struct\s+|enum\s+|union\s+)?\s*(?:unsigned\s+|signed\s+)?\s*(?:short\s+|long\s+)?\s*\w+(?:\s*<\s*[^>]*\s*>)?(?:\s*\*+\s*|\s+const\s*|\s+volatile\s*)*)\s*(\w+(?:\s*\[\s*\d*\s*\])?)?$'

        match = re.match(pattern, param)
        if match:
            param_type = match.group(1).strip()
            param_name = match.group(2).strip() if match.group(2) else ""

            if not param_name and param_type:
                # Try to extract name from the end
                words = param_type.split()
                if words and words[-1].isidentifier() and not words[-1] in ['const', 'volatile', 'static', 'extern']:
                    param_name = words[-1]
                    param_type = ' '.join(words[:-1])

            return param_type, param_name

        # Fallback: split on spaces and take last word as name
        words = param.split()
        if not words:
            return "", ""

        type_keywords = {
            'const', 'volatile', 'struct', 'enum', 'union',
            'unsigned', 'signed', 'short', 'long', 'static',
            'extern', 'auto', 'register'
        }

        i = 0
        while i < len(words):
            word = words[i]
            if (word in type_keywords or
                    word.endswith('*') or
                    word.endswith('*const') or
                    (i > 0 and words[i-1] in type_keywords) or
                    re.match(r'^\w+$', word)):
                i += 1
                continue
            if word.isidentifier():
                break
            i += 1

        if i >= len(words):
            return param, ""

        param_type = ' '.join(words[:i])
        param_name = ' '.join(words[i:])

        param_name = re.sub(r'\[.*\]', '', param_name).strip()

        return param_type, param_name

    def _parse_doxygen_comment(self, comment: str) -> Dict[str, str]:
        doc_info = {'brief': '', 'description': '', 'params': {}, 'members': {}}
        if not comment:
            return doc_info
        comment = re.sub(r'^\s*/\*\*|\*/\s*$', '', comment, flags=re.MULTILINE)
        lines = comment.split('\n')
        clean_lines = [re.sub(r'^\s*\*?\s*', '', line).strip() for line in lines if line.strip()]
        full_comment = ' '.join(clean_lines)

        brief_match = re.search(r'@brief\s+(.*?)(?=@|$)', full_comment)
        if brief_match:
            doc_info['brief'] = self._clean_comment_text(brief_match.group(1))

        param_matches = re.finditer(r'@param\s+(\w+)\s+(.*?)(?=@|$)', full_comment)
        for match in param_matches:
            doc_info['params'][match.group(1)] = self._clean_comment_text(match.group(2))

        member_matches = re.finditer(r'@member\s+(\w+)\s+(.*?)(?=@|$)', full_comment)
        for match in member_matches:
            doc_info['members'][match.group(1)] = self._clean_comment_text(match.group(2))

        description_text = re.sub(r'@\w+\s+.*?(?=@|$)', '', full_comment)
        doc_info['description'] = self._clean_comment_text(description_text)

        return doc_info

    def _clean_comment_text(self, text: str) -> str:
        if not text:
            return ""
        text = re.sub(r'^\s*\*?\s*', '', text)
        text = re.sub(r'\s*\*?\s*$', '', text)
        text = re.sub(r'\s+', ' ', text)
        return text.strip()

    def build_file_tree(self) -> FileTreeNode:
        root = FileTreeNode(name="Project", path="", children=[], is_directory=True)

        processed_paths = {}

        for header in self.headers:
            file_path = header.file_path
            rel_path = os.path.relpath(file_path, start='.')

            if rel_path not in processed_paths:
                self._add_file_to_tree(root, file_path, header=header)
                processed_paths[rel_path] = True

        for page in self.markdown_pages:
            file_found = False
            for markdown_dir in self.config['markdown_dirs']:
                potential_path = os.path.join(markdown_dir, page.filename)
                if os.path.exists(potential_path):
                    rel_path = os.path.relpath(potential_path, start='.')

                    if rel_path not in processed_paths:
                        self._add_file_to_tree(root, potential_path, markdown_page=page)
                        processed_paths[rel_path] = True
                        file_found = True
                        break

            if not file_found:
                rel_path = page.filename
                if rel_path not in processed_paths:
                    self._add_file_to_tree(root, rel_path, markdown_page=page)
                    processed_paths[rel_path] = True

        self.file_tree = root
        return root

    def _add_file_to_tree(self, root: FileTreeNode, file_path: str,
                          header: Optional[HeaderFile] = None,
                          markdown_page: Optional[MarkdownPage] = None):
        if os.path.exists(file_path):
            rel_path = os.path.relpath(file_path, start='.')
        else:
            rel_path = file_path

        path_parts = rel_path.split(os.sep)

        current = root
        current_path = ""

        for i, part in enumerate(path_parts[:-1]):
            current_path = os.path.join(current_path, part) if current_path else part

            found = None
            for child in current.children:
                if child.name == part and child.is_directory:
                    found = child
                    break

            if not found:
                found = FileTreeNode(
                    name=part,
                    path=current_path,
                    children=[],
                    is_directory=True,
                    level=i
                )
                current.children.append(found)

            current = found

        filename = path_parts[-1]

        file_exists = False
        for child in current.children:
            if child.name == filename and not child.is_directory:
                file_exists = True
                break

        if not file_exists:
            file_node = FileTreeNode(
                name=filename,
                path=rel_path,
                children=[],
                is_directory=False,
                header_file=header,
                markdown_page=markdown_page,
                level=len(path_parts) - 1
            )
            current.children.append(file_node)

    def _generate_sidebar_content(self) -> str:
        file_explorer = self._generate_file_explorer()
        outline = self._generate_outline()

        return f"""
        <div class="sidebar-tabs">
            <div class="sidebar-tab-buttons">
                <button class="tab-button active" data-tab="explorer">
                    <i class="fas fa-folder"></i>
                    <span>Explorer</span>
                </button>
                <button class="tab-button" data-tab="outline">
                    <i class="fas fa-list"></i>
                    <span>Outline</span>
                </button>
            </div>
            
            <div class="sidebar-tab-content">
                <div class="tab-pane active" id="explorer-tab">
                    {file_explorer}
                </div>
                <div class="tab-pane" id="outline-tab">
                    {outline}
                </div>
            </div>
        </div>
        """

    def _generate_file_explorer(self) -> str:
        if not self.file_tree:
            return '<div class="empty-state">No files found</div>'

        return self._generate_file_tree_html(self.file_tree, is_root=True)

    def _generate_outline(self) -> str:
        html_parts = ['<div class="outline-content">']

        for header in self.headers:
            if header.functions or header.data_types:
                file_id = f"file-{header.filename.replace('.', '-').lower()}"
                html_parts.append(f'''
                <div class="outline-file">
                    <div class="outline-file-header" onclick="toggleOutlineFile(this)">
                        <i class="fas fa-chevron-right outline-file-icon"></i>
                        <i class="fas fa-file-code"></i>
                        <span>{header.filename}</span>
                    </div>
                    <div class="outline-file-content">
                        {self._generate_header_outline_html(header)}
                    </div>
                </div>
                ''')

        for page in self.markdown_pages:
            page_id = page.filename.replace('.md', '').lower()
            html_parts.append(f'''
            <div class="outline-item">
                <a href="#{page_id}" class="outline-link" data-type="page">
                    <i class="fas fa-file-alt"></i>
                    <span>{page.title}</span>
                </a>
            </div>
            ''')

        html_parts.append('</div>')
        return '\n'.join(html_parts)

    def _generate_header_outline_html(self, header: HeaderFile) -> str:
        html_parts = ['<div class="outline-items">']

        if header.data_types:
            html_parts.append('<div class="outline-section">')
            html_parts.append('<div class="outline-section-header">Data Types</div>')
            for data_type in header.data_types:
                html_parts.append(f'''
                <div class="outline-item">
                    <a href="#type-{data_type.name.lower()}" class="outline-link" data-type="data_type">
                        <i class="fas fa-cube"></i>
                        <span>{data_type.type} {data_type.name}</span>
                    </a>
                </div>
                ''')
            html_parts.append('</div>')

        if header.functions:
            html_parts.append('<div class="outline-section">')
            html_parts.append('<div class="outline-section-header">Functions</div>')
            for func in header.functions:
                html_parts.append(f'''
                <div class="outline-item">
                    <a href="#api-{func.name.lower()}" class="outline-link" data-type="function">
                        <i class="fas fa-code"></i>
                        <span>{func.name}</span>
                    </a>
                </div>
                ''')
            html_parts.append('</div>')

        html_parts.append('</div>')
        return '\n'.join(html_parts)

    def _generate_file_tree_html(self, node: FileTreeNode, is_root: bool = False) -> str:
        if not node.children:
            return '<div class="empty-state">No files found</div>'

        html_parts = []

        if not is_root:
            html_parts.append(f'<div class="nav-section">')
            html_parts.append(f'<h2 class="nav-title">{node.name}</h2>')

        html_parts.append('<ul class="nav-links">')

        node.children.sort(key=lambda x: (not x.is_directory, x.name.lower()))

        for child in node.children:
            if child.is_directory:
                html_parts.append(f'''
                <li class="nav-directory">
                    <div class="nav-folder" onclick="toggleFolder(this)">
                        <i class="fas fa-chevron-right folder-icon"></i>
                        <i class="fas fa-folder folder-closed"></i>
                        <i class="fas fa-folder-open folder-open"></i>
                        <span>{child.name}</span>
                    </div>
                    <div class="nav-folder-content">
                        {self._generate_file_tree_html(child)}
                    </div>
                </li>
                ''')
            else:
                if child.header_file:
                    file_id = f"file-{child.header_file.filename.replace('.', '-').lower()}"
                    html_parts.append(f'''
                    <li class="nav-file">
                        <a href="#{file_id}" class="nav-file-header" data-type="header_file">
                            <i class="fas fa-file-code file-icon-main"></i>
                            <span>{child.name}</span>
                        </a>
                    </li>
                    ''')
                elif child.markdown_page:
                    page_id = child.markdown_page.filename.replace('.md', '').lower()
                    html_parts.append(f'''
                    <li class="nav-file">
                        <a href="#{page_id}" class="nav-link" data-type="page">
                            <i class="fas fa-file-alt"></i>
                            {child.markdown_page.title}
                        </a>
                    </li>
                    ''')

        html_parts.append('</ul>')

        if not is_root:
            html_parts.append('</div>')

        return '\n'.join(html_parts)

    def generate_docs(self) -> None:
        output_dir = Path(self.config['output_dir'])
        output_dir.mkdir(exist_ok=True)

        print("Generating documentation...")

        print("Building file tree...")
        self.build_file_tree()

        self.generate_html_docs()

        if self.config.get('generate_pdf', True):
            self.generate_pdf_docs()

        print(f"\nDocumentation generated in {output_dir}")
        print(f"Generated by GooeyDocs on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    def generate_html_docs(self) -> None:
        output_dir = Path(self.config['output_dir'])
        print("Generating HTML documentation...")

        complete_html_content = self._generate_complete_html_content()

        template = self._get_qt_style_template()
        sidebar_content = self._generate_sidebar_content()
        search_data = self._generate_search_data()

        content = template.format(
            project_name=self.config['project_name'],
            project_version=self.config['project_version'],
            nav_sections=sidebar_content,
            generated_date=datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            search_data=json.dumps(search_data),
            initial_content=complete_html_content
        )

        with open(output_dir / 'index.html', 'w', encoding='utf-8') as f:
            f.write(content)

        if self.config['generate_search']:
            self._generate_search_index(output_dir)
        print("HTML documentation complete")

    def _generate_complete_html_content(self) -> str:
        content_parts = []

        content_parts.append(self._generate_initial_content())

        content_parts.append(self._generate_api_reference_content())

        content_parts.append(self._generate_markdown_pages_content())

        return '\n'.join(content_parts)

    def _generate_api_reference_content(self) -> str:
        api_content = ['<section id="api-reference">']
        api_content.append('<h1>API Reference</h1>')

        for header in self.headers:
            if header.functions or header.data_types:
                file_id = f"file-{header.filename.replace('.', '-').lower()}"
                api_content.append(f'<section id="{file_id}" class="header-file-section">')
                api_content.append(f'<h2>{header.filename}</h2>')
                if header.description:
                    api_content.append(f'<p class="file-description">{html.escape(header.description)}</p>')

                if header.data_types:
                    api_content.append('<h3>Data Types</h3>')
                    for data_type in header.data_types:
                        api_content.append(self._generate_data_type_html(data_type))

                if header.functions:
                    api_content.append('<h3>Functions</h3>')
                    for func in header.functions:
                        api_content.append(self._generate_function_html(func))

                api_content.append('</section>')

        api_content.append('</section>')
        return '\n'.join(api_content)

    def _generate_markdown_pages_content(self) -> str:
        if not self.markdown_pages:
            return ""

        md_content = ['<section id="guides">']
        md_content.append('<h1>Guides & Documentation</h1>')

        for page in self.markdown_pages:
            page_id = page.filename.replace('.md', '').lower()
            md_content.append(f'<section id="{page_id}" class="markdown-page">')
            md_content.append(f'<h2>{html.escape(page.title)}</h2>')
            md_content.append(f'<div class="markdown-content">{page.html_content}</div>')
            md_content.append('</section>')

        md_content.append('</section>')
        return '\n'.join(md_content)

    def _generate_initial_content(self) -> str:
        total_functions = sum(len(header.functions) for header in self.headers)
        total_data_types = sum(len(header.data_types) for header in self.headers)
        total_pages = len(self.markdown_pages)
        total_headers = len(self.headers)

        content = [f"""
        <section id="welcome">
            <div class="hero-section">
                <div class="hero-background"></div>
                <div class="hero-content">
                    <h1>{self.config['project_name']} Documentation</h1>
                    <p class="hero-subtitle">Version {self.config['project_version']}</p>
                    <p class="hero-description">
                        Complete API reference and developer guides
                    </p>
                    
                    <div class="action-buttons">
                        <a href="#api-reference" class="action-btn primary">
                            <i class="fas fa-book"></i>
                            API Reference
                        </a>
                        <a href="#guides" class="action-btn secondary">
                            <i class="fas fa-graduation-cap"></i>
                            Getting Started
                        </a>
                    </div>
                </div>
            </div>
            
            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-icon">
                        <i class="fas fa-file-code"></i>
                    </div>
                    <div class="stat-content">
                        <div class="stat-number">{total_headers}</div>
                        <div class="stat-label">Header Files</div>
                    </div>
                </div>
                
                <div class="stat-card">
                    <div class="stat-icon">
                        <i class="fas fa-code"></i>
                    </div>
                    <div class="stat-content">
                        <div class="stat-number">{total_functions}</div>
                        <div class="stat-label">Functions</div>
                    </div>
                </div>
                
                <div class="stat-card">
                    <div class="stat-icon">
                        <i class="fas fa-cube"></i>
                    </div>
                    <div class="stat-content">
                        <div class="stat-number">{total_data_types}</div>
                        <div class="stat-label">Data Types</div>
                    </div>
                </div>
                
                <div class="stat-card">
                    <div class="stat-icon">
                        <i class="fas fa-book"></i>
                    </div>
                    <div class="stat-content">
                        <div class="stat-number">{total_pages}</div>
                        <div class="stat-label">Guides</div>
                    </div>
                </div>
            </div>

        </section>
        """]

        return '\n'.join(content)

    def _generate_search_data(self) -> Dict[str, Any]:
        search_data = {
            'functions': [],
            'data_types': [],
            'pages': [],
            'header_files': []
        }

        for header in self.headers:
            file_id = f"file-{header.filename.replace('.', '-').lower()}"
            search_data['header_files'].append({
                'name': header.filename,
                'brief': header.brief_description,
                'description': header.description,
                'url': f'#{file_id}',
                'type': 'header_file'
            })

            for func in header.functions:
                search_data['functions'].append({
                    'name': func.name,
                    'brief': func.brief_description,
                    'description': func.description,
                    'file': func.file,
                    'url': f'#api-{func.name.lower()}',
                    'type': 'function'
                })

            for data_type in header.data_types:
                search_data['data_types'].append({
                    'name': data_type.name,
                    'type': data_type.type,
                    'brief': data_type.brief_description,
                    'description': data_type.description,
                    'file': data_type.file,
                    'url': f'#type-{data_type.name.lower()}',
                    'type': 'data_type'
                })

        for page in self.markdown_pages:
            search_data['pages'].append({
                'title': page.title,
                'filename': page.filename,
                'url': f'#{page.filename.replace(".md", "").lower()}',
                'type': 'page'
            })

        return search_data

    def _generate_data_type_html(self, data_type: DataTypeInfo) -> str:
        members_html = ""
        if data_type.type == 'struct' and data_type.members:
            members_html = "<div class='api-members'><h4>Members</h4><table class='member-table'><thead><tr><th>Type</th><th>Name</th><th>Description</th></tr></thead><tbody>"
            for member in data_type.members:
                members_html += f"<tr><td><code>{html.escape(member.type)}</code></td><td><code>{html.escape(member.name)}</code></td><td>{html.escape(member.description)}</td></tr>"
            members_html += "</tbody></table></div>"

        return f"""
        <section id="type-{data_type.name.lower()}" class="api-item" data-searchable="true">
            <div class="api-header">
                <h3 class="api-name">{data_type.type} {data_type.name}</h3>
                <span class="api-badge api-badge-type">{data_type.type}</span>
            </div>
            {f"<p class='api-brief'>{html.escape(data_type.brief_description)}</p>" if data_type.brief_description else ""}
            {f"<p class='api-description'>{html.escape(data_type.description)}</p>" if data_type.description else ""}
            {members_html}
            <div class="api-signature">
                <div class="code-header">
                    <span>Definition</span>
                    <button class="copy-button" onclick="copyCode(this)">
                        <i class="fas fa-copy"></i>
                        Copy
                    </button>
                </div>
                <pre><code class="language-c">{html.escape(data_type.definition)}</code></pre>
            </div>
        </section>
        """

    def _generate_function_html(self, func: FunctionInfo) -> str:
        params_html = ""
        if func.parameters:
            params_html = "<div class='api-params'><h4>Parameters</h4><table class='param-table'><thead><tr><th>Type</th><th>Name</th><th>Description</th></tr></thead><tbody>"
            for param in func.parameters:
                params_html += f"<tr><td><code>{html.escape(param.type)}</code></td><td><code>{html.escape(param.name)}</code></td><td>{html.escape(param.description)}</td></tr>"
            params_html += "</tbody></table></div>"

        signature_parts = []
        for param in func.parameters:
            signature_parts.append(f"{html.escape(param.type)} {html.escape(param.name)}")

        signature = f"{html.escape(func.return_type)} {func.name}({', '.join(signature_parts)});"

        return f"""
        <section id="api-{func.name.lower()}" class="api-item" data-searchable="true">
            <div class="api-header">
                <h3 class="api-name">{func.name}</h3>
                <span class="api-badge api-badge-function">function</span>
            </div>
            {f"<p class='api-brief'>{html.escape(func.brief_description)}</p>" if func.brief_description else ""}
            {f"<p class='api-description'>{html.escape(func.description)}</p>" if func.description else ""}
            <div class="api-signature">
                <div class="code-header">
                    <span>Signature</span>
                    <button class="copy-button" onclick="copyCode(this)">
                        <i class="fas fa-copy"></i>
                        Copy
                    </button>
                </div>
                <pre><code class="language-c">{signature}</code></pre>
            </div>
            {params_html}
            <div class="api-returns">
                <h4>Return Value</h4>
                <p><code>{html.escape(func.return_type)}</code></p>
            </div>
        </section>
        """

    def _generate_search_index(self, output_dir: Path) -> None:
        search_data = self._generate_search_data()

        with open(output_dir / 'search_index.json', 'w', encoding='utf-8') as f:
            json.dump(search_data, f, indent=2)

    def generate_pdf_docs(self) -> None:
        try:
            output_dir = Path(self.config['output_dir'])
            pdf_path = output_dir / self.config.get('pdf_output', 'documentation.pdf')
            print(f"Generating PDF documentation: {pdf_path}")

            doc = SimpleDocTemplate(
                str(pdf_path),
                pagesize=letter,
                rightMargin=72,
                leftMargin=72,
                topMargin=72,
                bottomMargin=72
            )

            styles = getSampleStyleSheet()

            story = []

            title_style = ParagraphStyle(
                name='Title',
                parent=styles['Title'],
                fontSize=24,
                spaceAfter=30,
                alignment=1
            )

            story.append(Paragraph(f"{self.config['project_name']} Documentation", title_style))
            story.append(Paragraph(f"Version {self.config['project_version']}", styles['Heading2']))
            story.append(Paragraph(f"Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}", styles['Italic']))
            story.append(Spacer(1, 36))
            story.append(PageBreak())

            story.append(Paragraph("Table of Contents", styles['Heading1']))
            story.append(Spacer(1, 12))

            toc_data = [['Section', 'Page']]

            for header in self.headers:
                if header.functions or header.data_types:
                    toc_data.append([f"API: {header.filename}", ""])

            for page in self.markdown_pages:
                toc_data.append([page.title, ""])

            toc_table = Table(toc_data, colWidths=[400, 50])
            toc_table.setStyle(TableStyle([
                ('FONT', (0, 0), (-1, 0), 'Helvetica-Bold'),
                ('ALIGN', (0, 0), (-1, -1), 'LEFT'),
                ('FONTSIZE', (0, 0), (-1, -1), 10),
                ('BOTTOMPADDING', (0, 0), (-1, -1), 6),
                ('TOPPADDING', (0, 0), (-1, -1), 6),
            ]))
            story.append(toc_table)
            story.append(PageBreak())

            story.append(Paragraph("API Reference", styles['Heading1']))
            story.append(Spacer(1, 12))

            for header in self.headers:
                if header.functions or header.data_types:
                    story.append(Paragraph(header.filename, styles['Heading2']))
                    if header.description:
                        story.append(Paragraph(header.description, styles['Normal']))
                    story.append(Spacer(1, 6))

                    if header.functions:
                        story.append(Paragraph("Functions", styles['Heading3']))
                        for func in header.functions:
                            param_list = ', '.join([f"{p.type} {p.name}" for p in func.parameters])
                            signature = f"{func.return_type} {func.name}({param_list})"

                            story.append(Paragraph(func.name, styles['Heading3']))
                            if func.brief_description:
                                story.append(Paragraph(f"<b>Description:</b> {func.brief_description}", styles['Normal']))
                            story.append(Paragraph(f"<b>Signature:</b>", styles['Normal']))
                            story.append(Paragraph(signature, styles['Normal']))

                            if func.parameters:
                                story.append(Paragraph("<b>Parameters:</b>", styles['Normal']))
                                param_data = [['Name', 'Type', 'Description']]
                                for param in func.parameters:
                                    param_data.append([param.name, param.type, param.description])

                                param_table = Table(param_data, colWidths=[100, 150, 200])
                                param_table.setStyle(TableStyle([
                                    ('FONT', (0, 0), (-1, 0), 'Helvetica-Bold'),
                                    ('FONTSIZE', (0, 0), (-1, -1), 8),
                                    ('GRID', (0, 0), (-1, -1), 0.5, colors.grey),
                                    ('BACKGROUND', (0, 0), (-1, 0), colors.lightgrey),
                                ]))
                                story.append(param_table)

                            story.append(Spacer(1, 12))

                    if header.data_types:
                        story.append(Paragraph("Data Types", styles['Heading3']))
                        for data_type in header.data_types:
                            story.append(Paragraph(f"{data_type.type} {data_type.name}", styles['Heading3']))
                            if data_type.brief_description:
                                story.append(Paragraph(f"<b>Description:</b> {data_type.brief_description}", styles['Normal']))

                            if data_type.type == 'struct' and data_type.members:
                                story.append(Paragraph("<b>Members:</b>", styles['Normal']))
                                member_data = [['Type', 'Name', 'Description']]
                                for member in data_type.members:
                                    member_data.append([member.type, member.name, member.description])

                                member_table = Table(member_data, colWidths=[150, 100, 200])
                                member_table.setStyle(TableStyle([
                                    ('FONT', (0, 0), (-1, 0), 'Helvetica-Bold'),
                                    ('FONTSIZE', (0, 0), (-1, -1), 8),
                                    ('GRID', (0, 0), (-1, -1), 0.5, colors.grey),
                                    ('BACKGROUND', (0, 0), (-1, 0), colors.lightgrey),
                                ]))
                                story.append(member_table)

                            story.append(Paragraph("<b>Definition:</b>", styles['Normal']))
                            story.append(Paragraph(data_type.definition, styles['Normal']))
                            story.append(Spacer(1, 12))

                    story.append(PageBreak())

            if self.markdown_pages:
                story.append(Paragraph("Guides & Documentation", styles['Heading1']))
                story.append(Spacer(1, 12))

                for page in self.markdown_pages:
                    story.append(Paragraph(page.title, styles['Heading2']))

                    content = page.content
                    content = re.sub(r'#+\s+', '', content)
                    content = re.sub(r'\*\*(.*?)\*\*', r'\1', content)
                    content = re.sub(r'\*(.*?)\*', r'\1', content)
                    content = re.sub(r'`(.*?)`', r'\1', content)

                    paragraphs = content.split('\n\n')
                    for para in paragraphs:
                        if para.strip():
                            story.append(Paragraph(para.strip(), styles['Normal']))
                            story.append(Spacer(1, 6))

                    story.append(PageBreak())

            doc.build(story)
            print("PDF documentation complete")

        except Exception as e:
            print(f"Error generating PDF documentation: {e}")

    def _get_qt_style_template(self) -> str:
        return """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{project_name} {project_version} Documentation</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
    <link id="prism-theme" rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/prism/1.9.0/prism.min.css"/>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github.min.css">

    <style>
        :root {{
            --gooey-primary: #3c6382;
            --gooey-primary-dark: #38ada9;
            --gooey-secondary: #38ada9;
            --gooey-accent: #FF9800;
            --gooey-error: #F44336;
            --gooey-bg: #FFFFFF;
            --gooey-surface: #F5F5F5;
            --gooey-surface-elevated: #FFFFFF;
            --gooey-on-surface: #212121;
            --gooey-text-primary: #424242;
            --gooey-text-secondary: #757575;
            --gooey-text-tertiary: #9E9E9E;
            --gooey-border: #E0E0E0;
            --gooey-border-light: #EEEEEE;
            --gooey-radius: 8px;
            --gooey-radius-sm: 4px;
            --gooey-shadow: 0 2px 4px rgba(0,0,0,0.1);
            --gooey-shadow-md: 0 4px 8px rgba(0,0,0,0.12);
            --gooey-shadow-lg: 0 8px 16px rgba(0,0,0,0.15);
        }}

        .dark-theme {{
       --gooey-primary: #3c6382;
            --gooey-primary-dark: #38ada9;
            --gooey-secondary: #38ada9;
            --gooey-accent: #FFB74D;
            --gooey-error: #EF5350;
            --gooey-bg: #121212;
            --gooey-surface: #1E1E1E;
            --gooey-surface-elevated: #2D2D2D;
            --gooey-on-surface: #FFFFFF;
            --gooey-text-primary: #E0E0E0;
            --gooey-text-secondary: #9E9E9E;
            --gooey-text-tertiary: #666666;
            --gooey-border: #333333;
            --gooey-border-light: #252525;
            --gooey-shadow: 0 2px 4px rgba(0,0,0,0.3);
            --gooey-shadow-md: 0 4px 8px rgba(0,0,0,0.4);
            --gooey-shadow-lg: 0 8px 16px rgba(0,0,0,0.5);
        }}

        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}

        body {{
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
            color: var(--gooey-text-primary);
            background: var(--gooey-bg);
            line-height: 1.6;
            display: grid;
            grid-template-columns: 280px 1fr;
            min-height: 100vh;
            transition: all 0.3s ease;
        }}

        /* gooey-style Hero Section */
        .hero-section {{
            position: relative;
            padding: 80px 0 60px 0;
            background: linear-gradient(135deg, var(--gooey-primary) 0%, var(--gooey-primary-dark) 100%);
            color: white;
            text-align: center;
            overflow: hidden;
            width: 100%;
            border-radius: var(--gooey-radius);
            margin-bottom: 40px;
        }}

        .hero-background {{
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: url('data:image/svg+xml,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1000 1000" opacity="0.1"><polygon fill="white" points="0,1000 1000,0 1000,1000"/></svg>');
            background-size: cover;
        }}

        .hero-content {{
            position: relative;
            z-index: 2;
            max-width: 800px;
            margin: 0 auto;
            padding: 0 20px;
        }}

        .hero-section h1 {{
            font-size: 2.5rem;
            font-weight: 700;
            margin-bottom: 16px;
            color: white;
        }}

        .hero-subtitle {{
            font-size: 1.2rem;
            font-weight: 400;
            margin-bottom: 16px;
            opacity: 0.9;
            color: white;
        }}

        .hero-description {{
            font-size: 1rem;
            margin-bottom: 40px;
            opacity: 0.8;
            max-width: 600px;
            margin-left: auto;
            margin-right: auto;
            line-height: 1.6;
            color: white;
        }}

        .action-buttons {{
            display: flex;
            gap: 16px;
            justify-content: center;
            flex-wrap: wrap;
        }}

        .action-btn {{
            display: inline-flex;
            align-items: center;
            gap: 10px;
            padding: 12px 24px;
            font-size: 0.95rem;
            font-weight: 600;
            text-decoration: none;
            border-radius: var(--gooey-radius);
            transition: all 0.3s ease;
            border: 2px solid transparent;
        }}

        .action-btn.primary {{
            background: white;
            color: var(--gooey-primary);
            box-shadow: var(--gooey-shadow-lg);
        }}

        .action-btn.primary:hover {{
            background: var(--gooey-surface);
            transform: translateY(-2px);
            box-shadow: var(--gooey-shadow-lg), 0 20px 25px -5px rgba(0, 0, 0, 0.1);
        }}

        .action-btn.secondary {{
            background: transparent;
            color: white;
            border-color: rgba(255, 255, 255, 0.3);
        }}

        .action-btn.secondary:hover {{
            background: rgba(255, 255, 255, 0.1);
            border-color: white;
            transform: translateY(-2px);
        }}

        /* Stats Grid */
        .stats-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin: 40px 0;
        }}

        .stat-card {{
            background: var(--gooey-surface-elevated);
            padding: 24px;
            border-radius: var(--gooey-radius);
            text-align: center;
            box-shadow: var(--gooey-shadow);
            border: 1px solid var(--gooey-border);
            transition: all 0.3s ease;
            display: flex;
            align-items: center;
            gap: 16px;
        }}

        .stat-card:hover {{
            transform: translateY(-2px);
            box-shadow: var(--gooey-shadow-md);
        }}

        .stat-icon {{
            width: 50px;
            height: 50px;
            background: linear-gradient(135deg, var(--gooey-primary) 0%, var(--gooey-primary-dark) 100%);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-size: 1.2rem;
            flex-shrink: 0;
        }}

        .stat-content {{
            text-align: left;
        }}

        .stat-number {{
            font-size: 1.8rem;
            font-weight: 700;
            color: var(--gooey-primary);
            line-height: 1;
        }}

        .stat-label {{
            font-size: 0.9rem;
            color: var(--gooey-text-secondary);
            margin-top: 4px;
        }}

        /* Getting Started */
        .getting-started {{
            margin: 60px 0;
        }}

        .getting-started h2 {{
            text-align: center;
            margin-bottom: 40px;
            font-size: 2rem;
            font-weight: 600;
            color: var(--gooey-on-surface);
        }}

        .steps {{
            display: grid;
            gap: 24px;
            max-width: 800px;
            margin: 0 auto;
        }}

        .step {{
            display: flex;
            align-items: flex-start;
            gap: 20px;
            padding: 24px;
            background: var(--gooey-surface-elevated);
            border-radius: var(--gooey-radius);
            border: 1px solid var(--gooey-border);
        }}

        .step-number {{
            width: 40px;
            height: 40px;
            background: linear-gradient(135deg, var(--gooey-primary) 0%, var(--gooey-primary-dark) 100%);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-size: 1.1rem;
            font-weight: 700;
            flex-shrink: 0;
        }}

        .step-content h3 {{
            font-size: 1.2rem;
            font-weight: 600;
            margin-bottom: 8px;
            color: var(--gooey-on-surface);
        }}

        .step-content p {{
            color: var(--gooey-text-secondary);
            line-height: 1.6;
        }}

        /* Code Styling */
        .code-header {{
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: var(--gooey-surface);
            border-bottom: 1px solid var(--gooey-border);
            padding: 8px 16px;
            border-radius: var(--gooey-radius-sm) var(--gooey-radius-sm) 0 0;
            font-size: 12px;
            font-weight: 600;
            color: var(--gooey-text-secondary);
        }}

        .copy-button {{
            background: var(--gooey-primary);
            color: white;
            border: none;
            border-radius: var(--gooey-radius-sm);
            padding: 4px 8px;
            font-size: 11px;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 4px;
            transition: all 0.2s ease;
        }}

        .copy-button:hover {{
            background: var(--gooey-primary-dark);
            transform: translateY(-1px);
        }}

        .copy-button.copied {{
            background: var(--gooey-secondary);
        }}

        .top-bar {{
            position: fixed;
            top: 0;
            left: 280px;
            right: 0;
            background: var(--gooey-surface-elevated);
            border-bottom: 1px solid var(--gooey-border);
            padding: 12px 32px;
            display: flex;
            align-items: center;
            gap: 16px;
            z-index: 999;
            box-shadow: var(--gooey-shadow);
        }}

        .top-bar-search {{
            flex: 1;
            max-width: 400px;
            position: relative;
        }}

        .search-box {{
            display: flex;
            align-items: center;
            background: var(--gooey-surface);
            border: 1px solid var(--gooey-border);
            border-radius: var(--gooey-radius);
            padding: 10px 14px;
            width: 100%;
            transition: all 0.3s ease;
            position: relative;
        }}

        .search-box:focus-within {{
            border-color: var(--gooey-primary);
            box-shadow: 0 0 0 2px rgba(65, 205, 82, 0.1);
        }}

        .search-box input {{
            border: none;
            background: transparent;
            color: var(--gooey-text-primary);
            font-size: 14px;
            width: 100%;
            outline: none;
            font-family: 'Inter', sans-serif;
        }}

        .search-box input::placeholder {{
            color: var(--gooey-text-tertiary);
        }}

        .search-box i {{
            color: var(--gooey-text-tertiary);
            margin-right: 10px;
            transition: color 0.3s ease;
        }}

        .search-shortcut {{
            position: absolute;
            right: 10px;
            top: 50%;
            transform: translateY(-50%);
            background: var(--gooey-surface);
            border: 1px solid var(--gooey-border);
            border-radius: 4px;
            padding: 2px 6px;
            font-size: 11px;
            color: var(--gooey-text-tertiary);
            font-family: 'JetBrains Mono', monospace;
        }}

        .search-box:focus-within .search-shortcut {{
            opacity: 0;
        }}

        .search-results {{
            position: absolute;
            top: 100%;
            left: 0;
            right: 0;
            background: var(--gooey-surface-elevated);
            border: 1px solid var(--gooey-border);
            border-radius: var(--gooey-radius);
            margin-top: 8px;
            max-height: 400px;
            overflow-y: auto;
            display: none;
            z-index: 1001;
            box-shadow: var(--gooey-shadow-lg);
        }}

        .search-result-item {{
            padding: 12px 16px;
            border-bottom: 1px solid var(--gooey-border-light);
            cursor: pointer;
            transition: all 0.2s ease;
            display: flex;
            align-items: center;
            gap: 10px;
        }}

        .search-result-item:hover {{
            background: var(--gooey-primary);
            color: white;
        }}

        .search-result-item.active {{
            background: var(--gooey-primary);
            color: white;
        }}

        .search-result-item:last-child {{
            border-bottom: none;
        }}

        .search-result-icon {{
            width: 18px;
            text-align: center;
            opacity: 0.7;
        }}

        .search-result-content {{
            flex: 1;
        }}

        .search-result-title {{
            font-weight: 600;
            font-size: 14px;
            margin-bottom: 2px;
        }}

        .search-result-meta {{
            font-size: 12px;
            opacity: 0.7;
        }}

        .search-result-item:hover .search-result-meta {{
            opacity: 0.9;
        }}

        .theme-toggle {{
            background: var(--gooey-primary);
            color: white;
            border: none;
            border-radius: var(--gooey-radius);
            padding: 8px 14px;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 6px;
            font-size: 13px;
            font-weight: 500;
            transition: all 0.3s ease;
        }}

        .theme-toggle:hover {{
            background: var(--gooey-primary-dark);
            transform: translateY(-1px);
        }}

        .sidebar {{
            background: var(--gooey-surface);
            border-right: 1px solid var(--gooey-border);
            padding: 0;
            position: sticky;
            top: 0;
            height: 100vh;
            overflow-y: auto;
            transition: all 0.3s ease;
        }}

        .sidebar-header {{
            padding: 20px;
            border-bottom: 1px solid var(--gooey-border);
            background: var(--gooey-surface-elevated);
        }}

        .sidebar-title {{
            font-size: 18px;
            font-weight: 700;
            margin: 0;
            color: var(--gooey-on-surface);
        }}

        .sidebar-version {{
            font-size: 13px;
            color: var(--gooey-text-secondary);
            margin: 4px 0 0;
            font-weight: 500;
        }}

        .sidebar-tabs {{
            display: flex;
            flex-direction: column;
            height: calc(100vh - 80px);
        }}

        .sidebar-tab-buttons {{
            display: flex;
            background: var(--gooey-surface-elevated);
            border-bottom: 1px solid var(--gooey-border);
        }}

        .tab-button {{
            flex: 1;
            background: none;
            border: none;
            padding: 12px 16px;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 13px;
            font-weight: 500;
            color: var(--gooey-text-secondary);
            transition: all 0.2s ease;
            border-bottom: 2px solid transparent;
        }}

        .tab-button:hover {{
            background: var(--gooey-surface);
            color: var(--gooey-text-primary);
        }}

        .tab-button.active {{
            color: var(--gooey-primary);
            border-bottom-color: var(--gooey-primary);
            background: var(--gooey-surface);
        }}

        .sidebar-tab-content {{
            flex: 1;
            overflow-y: auto;
        }}

        .tab-pane {{
            display: none;
            height: 100%;
            overflow-y: auto;
        }}

        .tab-pane.active {{
            display: block;
        }}

        .outline-content {{
            padding: 16px;
        }}

        .outline-file {{
            margin-bottom: 8px;
        }}

        .outline-file-header {{
            display: flex;
            align-items: center;
            gap: 8px;
            padding: 8px 12px;
            cursor: pointer;
            user-select: none;
            transition: all 0.2s ease;
            border-radius: var(--gooey-radius-sm);
            font-weight: 500;
            font-size: 13px;
        }}

        .outline-file-header:hover {{
            background: rgba(65, 205, 82, 0.08);
            color: var(--gooey-primary);
        }}

        .outline-file-header.expanded .outline-file-icon {{
            transform: rotate(90deg);
        }}

        .outline-file-icon {{
            width: 12px;
            transition: transform 0.2s ease;
            font-size: 10px;
        }}

        .outline-file-content {{
            display: none;
            margin-left: 16px;
            border-left: 1px solid var(--gooey-border-light);
        }}

        .outline-file-header.expanded + .outline-file-content {{
            display: block;
        }}

        .outline-section {{
            margin: 8px 0;
        }}

        .outline-section-header {{
            font-size: 11px;
            font-weight: 600;
            color: var(--gooey-text-secondary);
            padding: 8px 12px 4px;
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }}

        .outline-items {{
            margin-left: 8px;
        }}

        .outline-item {{
            margin: 2px 0;
        }}

        .outline-link {{
            display: flex;
            align-items: center;
            gap: 8px;
            padding: 6px 12px 6px 24px;
            color: var(--gooey-text-primary);
            text-decoration: none;
            font-size: 13px;
            transition: all 0.2s;
            border-radius: var(--gooey-radius-sm);
            border-left: 2px solid transparent;
        }}

        .outline-link:hover {{
            background: rgba(65, 205, 82, 0.08);
            color: var(--gooey-primary);
        }}

        .outline-link.active {{
            background: rgba(65, 205, 82, 0.12);
            color: var(--gooey-primary);
            border-left-color: var(--gooey-primary);
        }}

        .outline-link i {{
            width: 14px;
            text-align: center;
            font-size: 11px;
            opacity: 0.7;
        }}

        .empty-state {{
            padding: 40px 20px;
            text-align: center;
            color: var(--gooey-text-tertiary);
            font-size: 14px;
        }}

        .nav-section {{
            margin-bottom: 8px;
        }}

        .nav-title {{
            font-size: 13px;
            font-weight: 600;
            color: var(--gooey-text-secondary);
            padding: 12px 16px 8px;
            margin: 0;
            text-transform: uppercase;
            letter-spacing: 0.05em;
            transition: color 0.3s ease;
        }}

        .nav-links {{
            list-style: none;
            padding: 0;
            margin: 0;
        }}

        .nav-link {{
            display: flex;
            align-items: center;
            gap: 8px;
            padding: 8px 16px 8px 24px;
            color: var(--gooey-text-primary);
            text-decoration: none;
            font-size: 13px;
            transition: all 0.2s;
            border-left: 2px solid transparent;
            font-weight: 500;
        }}

        .nav-link:hover {{
            background: rgba(65, 205, 82, 0.08);
            color: var(--gooey-primary);
        }}

        .nav-link.active {{
            background: rgba(65, 205, 82, 0.12);
            color: var(--gooey-primary);
            border-left-color: var(--gooey-primary);
        }}

        .nav-link i {{
            width: 14px;
            text-align: center;
            font-size: 11px;
            opacity: 0.7;
        }}

        .nav-directory, .nav-file {{
            list-style: none;
        }}

        .nav-folder, .nav-file-header {{
            display: flex;
            align-items: center;
            gap: 8px;
            padding: 8px 16px 8px 24px;
            cursor: pointer;
            user-select: none;
            transition: all 0.2s ease;
            border-left: 2px solid transparent;
            text-decoration: none;
            color: var(--gooey-text-primary);
            font-size: 13px;
            font-weight: 500;
        }}

        .nav-folder:hover, .nav-file-header:hover {{
            background: rgba(65, 205, 82, 0.08);
            color: var(--gooey-primary);
        }}

        .nav-folder.active, .nav-file-header.active {{
            background: rgba(65, 205, 82, 0.12);
            color: var(--gooey-primary);
            border-left-color: var(--gooey-primary);
        }}

        .folder-icon, .file-icon {{
            width: 12px;
            transition: transform 0.2s ease;
            font-size: 10px;
        }}

        .folder-open, .folder-closed {{
            width: 14px;
            font-size: 12px;
        }}

        .folder-open {{
            display: none;
        }}

        .folder-closed {{
            display: inline-block;
        }}

        .nav-folder.expanded .folder-icon {{
            transform: rotate(90deg);
        }}

        .nav-folder.expanded .folder-open {{
            display: inline-block;
        }}

        .nav-folder.expanded .folder-closed {{
            display: none;
        }}

        .nav-file.expanded .file-icon {{
            transform: rotate(90deg);
        }}

        .nav-folder-content, .nav-file-content {{
            display: none;
            margin-left: 16px;
            border-left: 1px solid var(--gooey-border-light);
        }}

        .nav-folder.expanded + .nav-folder-content,
        .nav-file.expanded + .nav-file-content {{
            display: block;
        }}

        .file-icon-main {{
            width: 14px;
            font-size: 12px;
            color: var(--gooey-accent);
        }}

        .content {{
            padding: 80px 32px 32px 32px;
            max-width: 1200px;
            margin: 0 auto;
            width: 100%;
        }}

        h1 {{
            font-size: 32px;
            font-weight: 700;
            margin: 0 0 20px;
            padding-bottom: 12px;
            border-bottom: 2px solid var(--gooey-border);
            color: var(--gooey-on-surface);
        }}

        h2 {{
            font-size: 24px;
            font-weight: 600;
            margin: 40px 0 16px;
            color: var(--gooey-on-surface);
        }}

        h3 {{
            font-size: 20px;
            font-weight: 600;
            margin: 32px 0 12px;
            color: var(--gooey-on-surface);
        }}

        h4 {{
            font-size: 16px;
            font-weight: 600;
            margin: 24px 0 8px;
            color: var(--gooey-on-surface);
        }}

        p, li {{
            color: var(--gooey-text-primary);
            font-size: 15px;
            line-height: 1.6;
        }}

        a {{
            color: var(--gooey-primary);
            text-decoration: none;
            font-weight: 500;
            transition: color 0.2s ease;
        }}

        a:hover {{
            color: var(--gooey-primary-dark);
            text-decoration: underline;
        }}

        pre {{
            background: var(--gooey-surface-elevated);
            border-radius: 0 0 var(--gooey-radius) var(--gooey-radius);
            padding: 16px;
            overflow-x: auto;
            font-family: 'JetBrains Mono', monospace;
            font-size: 13px;
            line-height: 1.5;
            margin: 0 0 16px 0;
            border: 1px solid var(--gooey-border);
            border-top: none;
            box-shadow: var(--gooey-shadow);
            transition: all 0.3s ease;
        }}

        code {{
            font-family: 'JetBrains Mono', monospace;
            background: var(--gooey-surface);
            padding: 2px 6px;
            border-radius: var(--gooey-radius-sm);
            font-size: 12px;
            border: 1px solid var(--gooey-border-light);
            transition: all 0.3s ease;
        }}

        pre code {{
            background: none;
            padding: 0;
            border: none;
            border-radius: 0;
        }}

        .header-file-section {{
            margin-bottom: 48px;
            padding: 24px;
            background: var(--gooey-surface-elevated);
            border-radius: var(--gooey-radius);
            border: 1px solid var(--gooey-border);
            box-shadow: var(--gooey-shadow);
        }}

        .file-description {{
            font-size: 15px;
            color: var(--gooey-text-secondary);
            margin-bottom: 20px;
            padding: 12px;
            background: var(--gooey-surface);
            border-radius: var(--gooey-radius-sm);
            border-left: 4px solid var(--gooey-primary);
        }}

        .api-item {{
            margin-bottom: 32px;
            padding: 20px;
            background: var(--gooey-surface-elevated);
            border-radius: var(--gooey-radius);
            border: 1px solid var(--gooey-border);
            box-shadow: var(--gooey-shadow);
            transition: all 0.3s ease;
        }}

        .api-item:hover {{
            box-shadow: var(--gooey-shadow-md);
            transform: translateY(-1px);
        }}

        .api-header {{
            display: flex;
            align-items: center;
            gap: 12px;
            margin-bottom: 12px;
            flex-wrap: wrap;
        }}

        .api-name {{
            font-family: 'JetBrains Mono', monospace;
            font-size: 18px;
            color: var(--gooey-primary);
            margin: 0;
        }}

        .api-badge {{
            display: inline-flex;
            padding: 4px 8px;
            border-radius: 12px;
            font-size: 11px;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }}

        .api-badge-type {{
            background: var(--gooey-secondary);
            color: white;
        }}

        .api-badge-function {{
            background: var(--gooey-primary);
            color: white;
        }}

        .api-brief {{
            font-size: 15px;
            color: var(--gooey-text-primary);
            margin: 12px 0;
            font-weight: 500;
        }}

        .api-description {{
            margin: 12px 0;
            color: var(--gooey-text-secondary);
        }}

        .api-params, .api-returns, .api-members {{
            margin: 16px 0;
        }}

        .api-params h4, .api-returns h4, .api-members h4 {{
            font-size: 15px;
            margin-bottom: 10px;
            color: var(--gooey-on-surface);
        }}

        .param-table, .member-table {{
            width: 100%;
            border-collapse: collapse;
            margin: 10px 0;
            border-radius: var(--gooey-radius-sm);
            overflow: hidden;
            box-shadow: var(--gooey-shadow);
        }}

        .param-table thead, .member-table thead {{
            background: var(--gooey-surface);
        }}

        .param-table th, .param-table td, .member-table th, .member-table td {{
            padding: 10px 14px;
            text-align: left;
            border-bottom: 1px solid var(--gooey-border);
        }}

        .param-table th, .member-table th {{
            font-weight: 600;
            color: var(--gooey-on-surface);
            font-size: 13px;
        }}

        .param-table tr:last-child td, .member-table tr:last-child td {{
            border-bottom: none;
        }}

        .param-table tr:hover td, .member-table tr:hover td {{
            background: rgba(65, 205, 82, 0.04);
        }}

        .callout {{
            padding: 16px;
            border-radius: var(--gooey-radius);
            margin: 16px 0;
            background: var(--gooey-surface-elevated);
            border-left: 4px solid var(--gooey-primary);
            box-shadow: var(--gooey-shadow);
        }}

        .callout.warning {{
            border-left-color: var(--gooey-accent);
            background: rgba(255, 152, 0, 0.08);
        }}

        .callout.important {{
            border-left-color: var(--gooey-error);
            background: rgba(244, 67, 54, 0.08);
        }}

        .markdown-content {{
            line-height: 1.7;
            color: var(--gooey-text-primary);
        }}

        .markdown-content h1 {{
            font-size: 2.2em;
            font-weight: 700;
            margin: 1.2em 0 0.6em 0;
            padding-bottom: 0.3em;
            border-bottom: 3px solid var(--gooey-primary);
            color: var(--gooey-on-surface);
        }}

        .markdown-content h2 {{
            font-size: 1.8em;
            font-weight: 600;
            margin: 1.2em 0 0.6em 0;
            padding-bottom: 0.3em;
            border-bottom: 2px solid var(--gooey-border);
            color: var(--gooey-on-surface);
        }}

        .markdown-content h3 {{
            font-size: 1.4em;
            font-weight: 600;
            margin: 1.2em 0 0.6em 0;
            color: var(--gooey-on-surface);
        }}

        .markdown-content h4 {{
            font-size: 1.2em;
            font-weight: 600;
            margin: 1.2em 0 0.6em 0;
            color: var(--gooey-on-surface);
        }}

        .markdown-content h5 {{
            font-size: 1.1em;
            font-weight: 600;
            margin: 1.2em 0 0.6em 0;
            color: var(--gooey-on-surface);
        }}

        .markdown-content h6 {{
            font-size: 1em;
            font-weight: 600;
            margin: 1.2em 0 0.6em 0;
            color: var(--gooey-text-secondary);
        }}

        .markdown-content p {{
            margin: 1em 0;
            font-size: 15px;
            line-height: 1.6;
        }}

        .markdown-content ul, .markdown-content ol {{
            margin: 1em 0;
            padding-left: 1.5em;
        }}

        .markdown-content li {{
            margin: 0.4em 0;
            line-height: 1.5;
        }}

        .markdown-content ul li {{
            list-style-type: disc;
        }}

        .markdown-content ol li {{
            list-style-type: decimal;
        }}

        .markdown-content blockquote {{
            border-left: 4px solid var(--gooey-primary);
            margin: 1.2em 0;
            padding: 1em 1.2em;
            background: var(--gooey-surface);
            border-radius: 0 var(--gooey-radius) var(--gooey-radius) 0;
            font-style: italic;
            color: var(--gooey-text-secondary);
            box-shadow: var(--gooey-shadow);
        }}

        .markdown-content blockquote p {{
            margin: 0;
        }}

        .markdown-content table {{
            width: 100%;
            border-collapse: collapse;
            margin: 1.2em 0;
            border-radius: var(--gooey-radius);
            overflow: hidden;
            box-shadow: var(--gooey-shadow);
            font-size: 13px;
        }}

        .markdown-content th, .markdown-content td {{
            padding: 10px 14px;
            text-align: left;
            border-bottom: 1px solid var(--gooey-border);
        }}

        .markdown-content th {{
            background: var(--gooey-surface);
            font-weight: 600;
            color: var(--gooey-on-surface);
            font-size: 13px;
        }}

        .markdown-content tr:hover td {{
            background: rgba(65, 205, 82, 0.04);
        }}

        .markdown-content code {{
            background: var(--gooey-surface);
            padding: 2px 5px;
            border-radius: 3px;
            font-size: 0.85em;
            font-family: 'JetBrains Mono', monospace;
            color: var(--gooey-primary);
            border: 1px solid var(--gooey-border-light);
        }}

        .markdown-content pre {{
            background: var(--gooey-surface-elevated);
            padding: 0;
            border-radius: var(--gooey-radius);
            overflow-x: auto;
            margin: 1.2em 0;
            border: 1px solid var(--gooey-border);
            box-shadow: var(--gooey-shadow);
        }}

        .markdown-content pre .code-header {{
            border-radius: var(--gooey-radius) var(--gooey-radius) 0 0;
        }}

        .markdown-content pre code {{
            background: none;
            padding: 1.2em;
            border: none;
            color: inherit;
            font-size: 12px;
            line-height: 1.4;
            display: block;
        }}

        .markdown-content img {{
            max-width: 100%;
            height: auto;
            border-radius: var(--gooey-radius);
            margin: 1.2em 0;
            box-shadow: var(--gooey-shadow);
        }}

        .markdown-content hr {{
            border: none;
            height: 2px;
            background: var(--gooey-border);
            margin: 1.5em 0;
            border-radius: 1px;
        }}

        .markdown-content .highlight {{
            margin: 1.2em 0;
            border-radius: var(--gooey-radius);
            overflow: hidden;
            box-shadow: var(--gooey-shadow);
        }}

        .markdown-content .highlight pre {{
            margin: 0;
            border-radius: 0;
        }}

        .markdown-content .admonition {{
            padding: 1em 1.2em;
            margin: 1.2em 0;
            border-radius: var(--gooey-radius);
            border-left: 4px solid var(--gooey-primary);
            background: var(--gooey-surface);
            box-shadow: var(--gooey-shadow);
        }}

        .markdown-content .admonition-title {{
            font-weight: 600;
            margin-bottom: 0.4em;
            color: var(--gooey-on-surface);
            display: flex;
            align-items: center;
            gap: 6px;
        }}

        .markdown-content .admonition.note {{
            border-left-color: var(--gooey-primary);
            background: rgba(65, 205, 82, 0.08);
        }}

        .markdown-content .admonition.warning {{
            border-left-color: var(--gooey-accent);
            background: rgba(255, 152, 0, 0.08);
        }}

        .markdown-content .admonition.danger {{
            border-left-color: var(--gooey-error);
            background: rgba(244, 67, 54, 0.08);
        }}

        .markdown-content .toc {{
            background: var(--gooey-surface);
            padding: 1.2em;
            border-radius: var(--gooey-radius);
            margin: 1.2em 0;
            box-shadow: var(--gooey-shadow);
        }}

        .markdown-content .toc ul {{
            margin: 0;
            padding-left: 0.8em;
        }}

        .markdown-content .toc li {{
            margin: 0.2em 0;
            list-style-type: none;
        }}

        .markdown-content .toc a {{
            color: var(--gooey-text-primary);
            text-decoration: none;
        }}

        .markdown-content .toc a:hover {{
            color: var(--gooey-primary);
            text-decoration: underline;
        }}

        .footer {{
            margin-top: 3em;
            padding-top: 1.5em;
            border-top: 1px solid var(--gooey-border);
            color: var(--gooey-text-secondary);
            font-size: 0.85em;
            text-align: center;
        }}

        @media (max-width: 1024px) {{
            body {{
                grid-template-columns: 1fr;
            }}

            .top-bar {{
                left: 0;
            }}

            .sidebar {{
                position: static;
                height: auto;
                border-right: none;
                border-bottom: 1px solid var(--gooey-border);
            }}

            .content {{
                padding: 120px 20px 20px 20px;
            }}
            
            .theme-toggle {{
                padding: 6px 10px;
            }}

            .top-bar-search {{
                max-width: 300px;
            }}

            .hero-section h1 {{
                font-size: 2.2rem;
            }}

            .action-buttons {{
                flex-direction: column;
                align-items: center;
            }}

            .action-btn {{
                width: 100%;
                max-width: 280px;
                justify-content: center;
            }}

            .stats-grid {{
                grid-template-columns: 1fr;
            }}

            .steps {{
                grid-template-columns: 1fr;
            }}
        }}

        @media (max-width: 768px) {{
            .hero-section {{
                padding: 60px 0 40px 0;
            }}

            .hero-section h1 {{
                font-size: 1.8rem;
            }}

            .top-bar {{
                flex-direction: column;
                gap: 10px;
                padding: 10px 16px;
            }}

            .top-bar-search {{
                max-width: 100%;
            }}

            .content {{
                padding: 140px 16px 16px 16px;
            }}

            .markdown-content h1 {{
                font-size: 1.8em;
            }}

            .markdown-content h2 {{
                font-size: 1.5em;
            }}

            .markdown-content h3 {{
                font-size: 1.3em;
            }}
        }}

        ::-webkit-scrollbar {{
            width: 6px;
        }}

        ::-webkit-scrollbar-track {{
            background: var(--gooey-surface);
        }}

        ::-webkit-scrollbar-thumb {{
            background: var(--gooey-border);
            border-radius: 3px;
        }}

        ::-webkit-scrollbar-thumb:hover {{
            background: var(--gooey-text-tertiary);
        }}

        .dark-theme ::-webkit-scrollbar-thumb {{
            background: var(--gooey-surface-elevated);
        }}

        .dark-theme ::-webkit-scrollbar-thumb:hover {{
            background: var(--gooey-text-secondary);
        }}
    </style>
</head>
<body>
    <div class="top-bar">
        <div class="top-bar-search">
            <div class="search-box">
                <i class="fas fa-search"></i>
                <input type="text" id="searchInput" placeholder="Search functions, types, documentation...">
                <div class="search-shortcut" title="Press / to search">/</div>
            </div>
            <div class="search-results" id="searchResults"></div>
        </div>
        <button class="theme-toggle" id="themeToggle">
            <i class="fas fa-moon" id="themeIcon"></i>
            <span>Toggle Theme</span>
        </button>
    </div>

    <aside class="sidebar">
        <div class="sidebar-header">
            <h1 class="sidebar-title">{project_name}</h1>
            <p class="sidebar-version">v{project_version}</p>
        </div>

        {nav_sections}
    </aside>

    <main class="content">
        {initial_content}
    </main>

    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/prism.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-c.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-bash.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-python.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-javascript.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-json.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
    
    <script>
        const searchData = {search_data};
        
        const themeToggle = document.getElementById('themeToggle');
        const themeIcon = document.getElementById('themeIcon');
        const prismTheme = document.getElementById('prism-theme');
        const searchInput = document.getElementById('searchInput');
        const searchResults = document.getElementById('searchResults');
        
        function copyCode(button) {{
            const pre = button.closest('.api-signature').querySelector('pre') || button.closest('.markdown-content').querySelector('pre');
            const code = pre.querySelector('code');
            const text = code.textContent;
            
            navigator.clipboard.writeText(text).then(() => {{
                const originalText = button.innerHTML;
                button.innerHTML = '<i class="fas fa-check"></i> Copied!';
                button.classList.add('copied');
                
                setTimeout(() => {{
                    button.innerHTML = originalText;
                    button.classList.remove('copied');
                }}, 2000);
            }}).catch(err => {{
                console.error('Failed to copy text: ', err);
            }});
        }}
        
        function initializeTheme() {{
            const savedTheme = localStorage.getItem('theme');
            const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
            
            if (savedTheme === 'dark' || (!savedTheme && prefersDark)) {{
                enableDarkTheme();
            }} else {{
                enableLightTheme();
            }}
        }}
        
        function enableDarkTheme() {{
            document.documentElement.classList.add('dark-theme');
            prismTheme.href = 'https://cdnjs.cloudflare.com/ajax/libs/prism-themes/1.9.0/prism-one-dark.min.css';
            themeIcon.classList.remove('fa-moon');
            themeIcon.classList.add('fa-sun');
            localStorage.setItem('theme', 'dark');
        }}
        
        function enableLightTheme() {{
            document.documentElement.classList.remove('dark-theme');
            prismTheme.href = 'https://cdnjs.cloudflare.com/ajax/libs/prism-themes/1.9.0/prism-one-light.min.css';
            themeIcon.classList.remove('fa-sun');
            themeIcon.classList.add('fa-moon');
            localStorage.setItem('theme', 'light');
        }}
        
        function toggleTheme() {{
            if (document.documentElement.classList.contains('dark-theme')) {{
                enableLightTheme();
            }} else {{
                enableDarkTheme();
            }}
        }}
        
        themeToggle.addEventListener('click', toggleTheme);
        
        document.addEventListener('keydown', function(e) {{
            if ((e.ctrlKey || e.metaKey) && e.key === 'k') {{
                e.preventDefault();
                searchInput.focus();
                searchInput.select();
            }}
            
            if (e.key === '/' && document.activeElement !== searchInput && !e.ctrlKey && !e.metaKey) {{
                e.preventDefault();
                searchInput.focus();
                searchInput.select();
            }}
            
            if (e.key === 'Escape') {{
                if (document.activeElement === searchInput) {{
                    searchInput.blur();
                }}
                searchResults.style.display = 'none';
            }}
            
            if (searchResults.style.display === 'block') {{
                const results = searchResults.querySelectorAll('.search-result-item');
                const activeResult = searchResults.querySelector('.search-result-item.active');
                let currentIndex = activeResult ? Array.from(results).indexOf(activeResult) : -1;
                
                if (e.key === 'ArrowDown') {{
                    e.preventDefault();
                    currentIndex = (currentIndex + 1) % results.length;
                    updateActiveResult(results, currentIndex);
                }} else if (e.key === 'ArrowUp') {{
                    e.preventDefault();
                    currentIndex = (currentIndex - 1 + results.length) % results.length;
                    updateActiveResult(results, currentIndex);
                }} else if (e.key === 'Enter' && activeResult) {{
                    e.preventDefault();
                    activeResult.click();
                }}
            }}
        }});
        
        function updateActiveResult(results, index) {{
            results.forEach(result => result.classList.remove('active'));
            if (results[index]) {{
                results[index].classList.add('active');
                results[index].scrollIntoView({{ block: 'nearest' }});
            }}
        }}
        
        function getIconForType(type) {{
            switch(type) {{
                case 'function': return 'fa-code';
                case 'data_type': return 'fa-cube';
                case 'page': return 'fa-file-alt';
                case 'header_file': return 'fa-file-code';
                default: return 'fa-search';
            }}
        }}
        
        function searchContent(query) {{
            const results = [];
            const lowerQuery = query.toLowerCase();
            
            searchData.header_files.forEach(item => {{
                const score = calculateRelevance(item, lowerQuery);
                if (score > 0) {{
                    results.push({{...item, score}});
                }}
            }});
            
            searchData.functions.forEach(item => {{
                const score = calculateRelevance(item, lowerQuery);
                if (score > 0) {{
                    results.push({{...item, score}});
                }}
            }});
            
            searchData.data_types.forEach(item => {{
                const score = calculateRelevance(item, lowerQuery);
                if (score > 0) {{
                    results.push({{...item, score}});
                }}
            }});
            
            searchData.pages.forEach(item => {{
                const score = calculateRelevance(item, lowerQuery);
                if (score > 0) {{
                    results.push({{...item, score}});
                }}
            }});
            
            results.sort((a, b) => b.score - a.score);
            return results.slice(0, 10);
        }}
        
        function calculateRelevance(item, query) {{
            let score = 0;
            const name = item.name || item.title || '';
            const brief = item.brief || '';
            const description = item.description || '';
            const file = item.file || '';
            
            if (name.toLowerCase().includes(query)) {{
                score += 100;
            }}
            
            if (name.toLowerCase().indexOf(query) !== -1) {{
                score += 50;
            }}
            
            if (brief.toLowerCase().includes(query)) {{
                score += 30;
            }}
            
            if (description.toLowerCase().includes(query)) {{
                score += 20;
            }}
            
            if (file.toLowerCase().includes(query)) {{
                score += 10;
            }}
            
            return score;
        }}
        
        function displaySearchResults(results) {{
            if (results.length === 0) {{
                searchResults.innerHTML = `
                    <div class="search-result-item">
                        <div class="search-result-icon">
                            <i class="fas fa-search"></i>
                        </div>
                        <div class="search-result-content">
                            <div class="search-result-title">No results found</div>
                            <div class="search-result-meta">Try different keywords</div>
                        </div>
                    </div>
                `;
            }} else {{
                searchResults.innerHTML = results.map((item, index) => `
                    <div class="search-result-item ${{index === 0 ? 'active' : ''}}" onclick="window.location.href='${{item.url}}'">
                        <div class="search-result-icon">
                            <i class="fas ${{getIconForType(item.type)}}"></i>
                        </div>
                        <div class="search-result-content">
                            <div class="search-result-title">${{item.name || item.title}}</div>
                            <div class="search-result-meta">
                                ${{item.type === 'function' ? 'Function' : 
                                  item.type === 'data_type' ? `${{item.type}} • ${{item.file}}` : 
                                  item.type === 'header_file' ? 'Header File' :
                                  'Documentation Page'}}
                                ${{item.brief ? `• ${{item.brief}}` : ''}}
                            </div>
                        </div>
                    </div>
                `).join('');
            }}
            searchResults.style.display = 'block';
        }}
        
        searchInput.addEventListener('input', function() {{
            const query = this.value.trim();
            if (query.length < 2) {{
                searchResults.style.display = 'none';
                return;
            }}
            
            const results = searchContent(query);
            displaySearchResults(results);
        }});
        
        document.addEventListener('click', function(e) {{
            if (!searchInput.contains(e.target) && !searchResults.contains(e.target)) {{
                searchResults.style.display = 'none';
            }}
        }});
        
        const sections = document.querySelectorAll('section');
        const navLinks = document.querySelectorAll('.nav-link, .nav-file-header, .outline-link');
        
        function updateActiveNavLink() {{
            let current = '';
            const scrollPosition = window.scrollY + 100;
            
            sections.forEach(section => {{
                const sectionTop = section.offsetTop;
                const sectionHeight = section.offsetHeight;
                if (scrollPosition >= sectionTop && scrollPosition < sectionTop + sectionHeight) {{
                    current = section.getAttribute('id');
                }}
            }});
            
            navLinks.forEach(link => {{
                link.classList.remove('active');
                const href = link.getAttribute('href');
                if (href && href === '#' + current) {{
                    link.classList.add('active');
                }}
            }});
        }}
        
        window.addEventListener('scroll', updateActiveNavLink);
        window.addEventListener('load', updateActiveNavLink);
        
        document.addEventListener('DOMContentLoaded', function() {{
            initializeTheme();
            updateActiveNavLink();
            
            const tabButtons = document.querySelectorAll('.tab-button');
            const tabPanes = document.querySelectorAll('.tab-pane');
            
            tabButtons.forEach(button => {{
                button.addEventListener('click', function() {{
                    const targetTab = this.getAttribute('data-tab');
                    
                    tabButtons.forEach(btn => btn.classList.remove('active'));
                    this.classList.add('active');
                    
                    tabPanes.forEach(pane => {{
                        pane.classList.remove('active');
                        if (pane.id === targetTab + '-tab') {{
                            pane.classList.add('active');
                        }}
                    }});
                }});
            }});
            
            document.querySelectorAll('pre code').forEach((block) => {{
                hljs.highlightElement(block);
            }});
            
            if (typeof Prism !== 'undefined') {{
                Prism.highlightAll();
            }}
            
            const activeLink = document.querySelector('.nav-link.active, .outline-link.active');
            if (activeLink) {{
                let parent = activeLink.closest('.nav-folder-content, .outline-file-content');
                while (parent) {{
                    const folder = parent.previousElementSibling;
                    if (folder && (folder.classList.contains('nav-folder') || folder.classList.contains('outline-file-header'))) {{
                        folder.classList.add('expanded');
                    }}
                    parent = parent.parentElement.closest('.nav-folder-content, .outline-file-content');
                }}
            }}
            
            navLinks.forEach(link => {{
                link.addEventListener('click', function(e) {{
                    const href = this.getAttribute('href');
                    if (href && href.startsWith('#')) {{
                        e.preventDefault();
                        const target = document.querySelector(href);
                        if (target) {{
                            target.scrollIntoView({{
                                behavior: 'smooth',
                                block: 'start'
                            }});
                        }}
                    }}
                }});
            }});
        }});
        
        function toggleFolder(element) {{
            element.classList.toggle('expanded');
        }}
        
        function toggleFile(element) {{
            element.classList.toggle('expanded');
        }}
        
        function toggleOutlineFile(element) {{
            element.classList.toggle('expanded');
        }}
    </script>
</body>
</html>"""

def main():
    parser = argparse.ArgumentParser(description='Generate Gooey documentation')
    parser.add_argument('--config', '-c', default='docs_config.json', help='Configuration file')
    parser.add_argument('--output', '-o', help='Output directory')
    parser.add_argument('--source', '-s', nargs='+', help='Source directories to search')
    parser.add_argument('--no-pdf', action='store_true', help='Skip PDF generation')
    args = parser.parse_args()

    generator = DocumentationGenerator()
    generator.load_config(args.config)

    if args.output:
        generator.config['output_dir'] = args.output
    if args.source:
        generator.config['search_dirs'] = args.source
    if args.no_pdf:
        generator.config['generate_pdf'] = False

    print("Gooey Documentation Generator")
    print("=============================")

    generator.parse_header_files()
    generator.parse_markdown_files()
    generator.generate_docs()

if __name__ == '__main__':
    main()