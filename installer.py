#!/usr/bin/env python3

import os
import sys
import platform
import subprocess
import shutil
import time
import getpass
import re
import json
from pathlib import Path
from enum import Enum
from datetime import datetime

try:
    import curses
    HAS_CURSES = True
except ImportError:
    HAS_CURSES = False


class ConfigType(Enum):
    BOOLEAN = 1
    STRING = 2
    INTEGER = 3
    MENU = 4
    COMMENT = 5


class ConfigOption:
    def __init__(self, name, description, config_type, default_value=None, 
                 dependencies=None, help_text=None, choices=None):
        self.name = name
        self.description = description
        self.type = config_type
        self.default_value = default_value
        self.value = default_value
        self.dependencies = dependencies or []
        self.help_text = help_text or ""
        self.choices = choices or []
        self.enabled = True
        self.visible = True
    
    def toggle(self):
        if self.type == ConfigType.BOOLEAN:
            self.value = not self.value
    
    def set_value(self, value):
        if self.type == ConfigType.BOOLEAN:
            self.value = bool(value)
        elif self.type == ConfigType.INTEGER:
            try:
                self.value = int(value)
            except ValueError:
                self.value = 0
        elif self.type == ConfigType.STRING:
            self.value = str(value)
    
    def get_display_value(self):
        if self.type == ConfigType.BOOLEAN:
            return "y" if self.value else "n"
        elif self.type == ConfigType.INTEGER:
            return str(self.value)
        elif self.type == ConfigType.STRING:
            return f'"{self.value}"' if self.value else '""'
        return str(self.value)
    
    def __str__(self):
        return f"{self.name} ({self.get_display_value()}) - {self.description}"


class MenuConfig:
    def __init__(self, stdscr):
        self.stdscr = stdscr
        self.options = []
        self.menu_stack = []
        self.selected_index = 0
        self.scroll_offset = 0
        self.height = 0
        self.width = 0
        
        if curses.has_colors():
            curses.start_color()
            curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_CYAN)
            curses.init_pair(2, curses.COLOR_GREEN, curses.COLOR_BLACK)
            curses.init_pair(3, curses.COLOR_BLUE, curses.COLOR_BLACK)
            curses.init_pair(4, curses.COLOR_WHITE, curses.COLOR_BLACK)
            curses.init_pair(5, curses.COLOR_BLACK, curses.COLOR_WHITE)
            curses.init_pair(6, curses.COLOR_CYAN, curses.COLOR_BLACK)
            curses.init_pair(7, curses.COLOR_BLACK, curses.COLOR_BLUE)
            
    
    def add_option(self, option):
        self.options.append(option)
    
    def draw(self):
        self.stdscr.clear()
        self.height, self.width = self.stdscr.getmaxyx()
        
        if curses.has_colors():
            self.stdscr.attron(curses.color_pair(5))
            for x in range(self.width):
                self.safe_add(0, x, " ")
            title = "Gooey GUI Configuration"
            x_pos = (self.width - len(title)) // 2
            if x_pos >= 0:
                self.safe_add(0, x_pos, title)
            self.stdscr.attroff(curses.color_pair(5))
        else:
            title = "Gooey GUI Configuration"
            x_pos = (self.width - len(title)) // 2
            if x_pos >= 0:
                self.safe_add(0, x_pos, title, curses.A_BOLD)
        
        if self.menu_stack:
            path = " > ".join(self.menu_stack)
            self.safe_add(1, 2, path, curses.A_BOLD if not curses.has_colors() else curses.color_pair(3))
        
        help_text = "Arrow keys: Navigate  Space: Toggle  Enter: Select  S: Save  Q: Quit"
        self.safe_add(2, 2, "-" * min(self.width - 4, 78), curses.A_DIM if not curses.has_colors() else curses.color_pair(4))
        self.safe_add(3, 2, help_text[:self.width-5], curses.A_DIM if not curses.has_colors() else curses.color_pair(6))
        
        visible_height = self.height - 8
        if visible_height < 1:
            visible_height = 1
        
        start_idx = self.scroll_offset
        end_idx = min(start_idx + visible_height, len(self.options))
        
        for i in range(start_idx, end_idx):
            option = self.options[i]
            line_y = 5 + (i - start_idx)
            
            if not option.visible or line_y >= self.height - 3:
                continue
            
            if i == self.selected_index:
                if curses.has_colors():
                    attr = curses.color_pair(1)
                else:
                    attr = curses.A_REVERSE
                marker = ">"
            else:
                attr = curses.color_pair(4) if curses.has_colors() else curses.A_NORMAL
                marker = " "
            
            if option.type == ConfigType.BOOLEAN:
                if option.value:
                    type_indicator = "[*]"
                    value_color = curses.color_pair(2) if curses.has_colors() else curses.A_BOLD
                else:
                    type_indicator = "[ ]"
                    value_color = curses.color_pair(4) if curses.has_colors() else curses.A_NORMAL
            elif option.type == ConfigType.MENU:
                type_indicator = "--->"
                value_color = curses.color_pair(3) if curses.has_colors() else curses.A_BOLD
            elif option.type == ConfigType.COMMENT:
                type_indicator = "   "
                value_color = curses.color_pair(4) if curses.has_colors() else curses.A_DIM
            else:
                type_indicator = "( )"
                value_color = curses.color_pair(4) if curses.has_colors() else curses.A_NORMAL
            
            if option.type == ConfigType.COMMENT:
                line = f"  {option.description}"
                line_attr = curses.color_pair(4) | curses.A_DIM if curses.has_colors() else curses.A_DIM
                self.safe_add(line_y, 2, line[:self.width-4], line_attr)
            else:
                line = f"{marker} {type_indicator} {option.name}"
                if option.type != ConfigType.MENU:
                    display_value = option.get_display_value()
                    line += f" = {display_value}"
                
                max_line_len = self.width - 4
                if len(line) > max_line_len:
                    line = line[:max_line_len]
                
                if i == self.selected_index:
                    line_attr = attr
                else:
                    line_attr = value_color
                
                if not option.enabled:
                    line_attr |= curses.A_DIM
                
                self.safe_add(line_y, 2, line, line_attr)
        
        if 0 <= self.selected_index < len(self.options):
            selected = self.options[self.selected_index]
            if selected.help_text and self.height > visible_height + 8:
                help_y = self.height - 3
                if help_y > 0:
                    self.safe_add(help_y, 2, "-" * min(self.width - 4, 78), 
                                 curses.A_DIM if not curses.has_colors() else curses.color_pair(6))
                    help_text = f"Help: {selected.help_text}"
                    self.safe_add(help_y + 1, 2, help_text[:self.width-5], 
                                 curses.A_DIM if not curses.has_colors() else curses.color_pair(6))
        
        if curses.has_colors():
            self.stdscr.attron(curses.color_pair(7))
            for x in range(self.width):
                self.safe_add(self.height - 1, x, " ")
            status = f"Items: {len(self.options)}  Selected: {self.selected_index + 1}"
            if self.width >= len(status) + 2:
                self.safe_add(self.height - 1, 2, status)
            self.stdscr.attroff(curses.color_pair(7))
        else:
            status = f"Items: {len(self.options)}  Selected: {self.selected_index + 1}"
            self.safe_add(self.height - 1, 2, status[:self.width-3], curses.A_REVERSE)
        
        self.stdscr.refresh()
    
    def safe_add(self, y, x, text, attr=0):
        """Safely add text to screen, ensuring it stays within bounds"""
        if not self.stdscr or y < 0 or x < 0:
            return
            
        max_y, max_x = self.stdscr.getmaxyx()
        if y >= max_y or x >= max_x:
            return
            
        available = max_x - x
        if available <= 0:
            return
            
        if isinstance(text, str):
            text = text[:available]
            try:
                self.stdscr.addstr(y, x, text, attr)
            except curses.error:
                for i, char in enumerate(text):
                    if x + i >= max_x:
                        break
                    try:
                        self.stdscr.addch(y, x + i, char, attr)
                    except:
                        pass
    
    def handle_input(self):
        key = self.stdscr.getch()
        
        if key == ord('q') or key == ord('Q'):
            return 'quit'
        elif key == ord('s') or key == ord('S'):
            return 'save'
        elif key == ord('r') or key == ord('R'):
            return 'reset'
        elif key == ord('h') or key == ord('H'):
            return 'help'
        elif key == curses.KEY_UP:
            self.selected_index = max(0, self.selected_index - 1)
            self.adjust_scroll()
        elif key == curses.KEY_DOWN:
            self.selected_index = min(len(self.options) - 1, self.selected_index + 1)
            self.adjust_scroll()
        elif key == ord(' ') and 0 <= self.selected_index < len(self.options):
            option = self.options[self.selected_index]
            if option.type == ConfigType.BOOLEAN and option.enabled:
                option.toggle()
            elif option.type == ConfigType.MENU:
                return 'enter_menu'
        elif key == 10:
            if 0 <= self.selected_index < len(self.options):
                option = self.options[self.selected_index]
                if option.type == ConfigType.INTEGER or option.type == ConfigType.STRING:
                    return 'edit_value'
                elif option.type == ConfigType.MENU:
                    return 'enter_menu'
        return None
    
    def adjust_scroll(self):
        if self.height < 8:
            return
            
        visible_height = self.height - 8
        
        if self.selected_index < self.scroll_offset:
            self.scroll_offset = self.selected_index
        elif self.selected_index >= self.scroll_offset + visible_height:
            self.scroll_offset = self.selected_index - visible_height + 1
    
    def edit_value(self, option):
        height, width = self.stdscr.getmaxyx()

        popup_h = 7
        popup_w = min(60, width - 4)
        popup_y = max(0, (height - popup_h) // 2)
        popup_x = max(0, (width - popup_w) // 2)
        
        if popup_y + popup_h > height:
            popup_y = height - popup_h
        if popup_x + popup_w > width:
            popup_x = width - popup_w
        
        popup = curses.newwin(popup_h, popup_w, popup_y, popup_x)
        popup.box()
        
        title = f"Edit: {option.name}"
        title_x = max(1, (popup_w - len(title)) // 2)
        popup.addstr(0, title_x, title, curses.A_BOLD if not curses.has_colors() else curses.color_pair(5))
        
        desc = option.description[:popup_w - 15]
        popup.addstr(2, 2, f"Description: {desc}")
        
        current_val = option.get_display_value()
        current_val_display = current_val[:popup_w - 20]
        popup.addstr(3, 2, f"Current value: {current_val_display}")
        popup.addstr(4, 2, "New value: ")
        
        popup.refresh()
        
        curses.echo()
        popup.move(4, 13)
        try:
            new_value = popup.getstr().decode('utf-8')
        except:
            new_value = ""
        curses.noecho()
        
        if new_value:
            option.set_value(new_value)
        
        return True
    
    def show_help(self):
        height, width = self.stdscr.getmaxyx()
        
        help_h = min(18, height - 4)
        help_w = min(60, width - 4)
        help_y = max(0, (height - help_h) // 2)
        help_x = max(0, (width - help_w) // 2)
        
        if help_y + help_h > height:
            help_y = height - help_h
        if help_x + help_w > width:
            help_x = width - help_w
        
        help_win = curses.newwin(help_h, help_w, help_y, help_x)
        help_win.box()
        
        title = "Configuration Help"
        title_x = max(1, (help_w - len(title)) // 2)
        help_win.addstr(0, title_x, title, curses.A_BOLD if not curses.has_colors() else curses.color_pair(5))
        
        help_lines = [
            "",
            "Navigation:",
            "  Up/Down arrows - Move selection",
            "  Space          - Toggle boolean options",
            "  Enter          - Edit values or enter submenus",
            "  S              - Save configuration",
            "  R              - Reset to defaults",
            "  Q              - Quit without saving",
            "",
            "Symbols:",
            "  [*]            - Feature enabled",
            "  [ ]            - Feature disabled",
            "  --->           - Submenu",
            "  ( )            - Editable value",
            "",
            "Press any key to continue..."
        ]
        
        for i, line in enumerate(help_lines):
            if i + 1 < help_h - 1:
                help_win.addstr(i + 1, 2, line[:help_w - 4])
        
        help_win.refresh()
        help_win.getch()
    
    def run(self):
        self.stdscr.clear()
        self.stdscr.keypad(True)
        curses.curs_set(0)
        
        while True:
            self.draw()
            action = self.handle_input()
            
            if action == 'quit':
                return False
            elif action == 'save':
                return True
            elif action == 'reset':
                for option in self.options:
                    if option.name:
                        option.value = option.default_value
            elif action == 'help':
                self.show_help()
            elif action == 'edit_value':
                option = self.options[self.selected_index]
                self.edit_value(option)
            elif action == 'enter_menu':
                pass


class InstallationManager:
    def __init__(self, stdscr=None):
        self.stdscr = stdscr
        self.system = platform.system().lower()
        self.sudo_password = None
        self.password_cached = False
        self.install_record_path = Path.home() / ".gooey_install_record.json"
        
        self.source_dir = Path("gooey-source")
        self.install_dir = Path("/usr/local/Gooey")
        
        self.steps = [
            "System Check",
            "Installation Detection",
            "Install dependencies",
            "Clone repository",
            "Init submodules",
            "Update submodules",
            "Configure build",
            "Find user_config.h",
            "Configure user_config.h",
            "Compile library",
            "Install library",
            "Record Installation",
        ]
        
        self.step_funcs = [
            self.check_system,
            self.check_installation,
            self.install_dependencies,
            self.clone_repository,
            self.init_submodules,
            self.update_submodules,
            self.configure_build,
            self.find_user_config,
            self.modify_user_config,
            self.compile_library,
            self.install_library,
            self.record_installation,
        ]
        
        self.step_status = [None] * len(self.steps)
        self.current_step = 0
        self.installation_found = False
        self.install_info = None
        self.user_action = None
        
        self.logs = []
        self.spinner = ["[    ]", "[=   ]", "[==  ]", "[=== ]", "[====]", "[ ===]", "[  ==]", "[   =]"]
        self.spin_index = 0
        self.height = 0
        self.width = 0
        self.user_config_path = None
        
        if self.stdscr and curses.has_colors():
            curses.start_color()
            curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_CYAN)
            curses.init_pair(2, curses.COLOR_GREEN, curses.COLOR_BLACK)
            curses.init_pair(3, curses.COLOR_BLUE, curses.COLOR_BLACK)
            curses.init_pair(4, curses.COLOR_WHITE, curses.COLOR_BLACK)
            curses.init_pair(5, curses.COLOR_BLACK, curses.COLOR_WHITE)
            curses.init_pair(6, curses.COLOR_CYAN, curses.COLOR_BLACK)
            curses.init_pair(7, curses.COLOR_BLACK, curses.COLOR_BLUE)

    def load_install_record(self):
        if self.install_record_path.exists():
            try:
                with open(self.install_record_path, 'r') as f:
                    return json.load(f)
            except:
                return None
        return None

    def save_install_record(self, info):
        try:
            with open(self.install_record_path, 'w') as f:
                json.dump(info, f, indent=2)
            return True
        except:
            return False

    def check_system_installation(self):
        """Check for Gooey installations in the system"""
        installations = []
        
        common_paths = [
            Path("/usr/local/Gooey"),
            Path("/usr/local/lib"),
            Path("/usr/lib"),
            Path("/usr/lib64"),
            Path("/usr/lib/x86_64-linux-gnu"),
            Path("/usr/local/include"),
            Path("/usr/include"),
        ]
        
        library_patterns = [
            "libGooeyGUI-1.so",
            "libGooeyGUI-1.so.*",
            "libGooeyGUI-1.a",
        ]
        
        header_patterns = [
            "gooey.h",
            "GooeyGUI.h",
        ]
        
        if Path("/usr/local/Gooey").exists():
            self.log("Found Gooey installation in /usr/local/Gooey")
            install_info = {
                'type': 'system',
                'path': '/usr/local/Gooey',
                'detected_by': 'directory_exists'
            }
            installations.append(install_info)
        
        for lib_pattern in library_patterns:
            find_cmd = ["find", "/usr", "/usr/local", "-name", lib_pattern, "-type", "f", "2>/dev/null"]
            try:
                result = subprocess.run(find_cmd, capture_output=True, text=True)
                if result.returncode == 0 and result.stdout.strip():
                    for lib_path in result.stdout.strip().split('\n'):
                        if lib_path:
                            lib_dir = Path(lib_path).parent
                            self.log(f"Found Gooey library: {lib_path}")
                            install_info = {
                                'type': 'library',
                                'path': lib_path,
                                'library_dir': str(lib_dir),
                                'detected_by': 'library_file'
                            }
                            installations.append(install_info)
            except:
                pass
        
        for header_pattern in header_patterns:
            find_cmd = ["find", "/usr", "/usr/local", "-name", header_pattern, "-type", "f", "2>/dev/null"]
            try:
                result = subprocess.run(find_cmd, capture_output=True, text=True)
                if result.returncode == 0 and result.stdout.strip():
                    for header_path in result.stdout.strip().split('\n'):
                        if header_path:
                            header_dir = Path(header_path).parent
                            self.log(f"Found Gooey header: {header_path}")
                            install_info = {
                                'type': 'header',
                                'path': header_path,
                                'header_dir': str(header_dir),
                                'detected_by': 'header_file'
                            }
                            installations.append(install_info)
            except:
                pass
        
        try:
            result = subprocess.run(["pkg-config", "--exists", "GooeyGUI-1"], 
                                  capture_output=True, text=True)
            if result.returncode == 0:
                self.log("Found GooeyGUI-1 via pkg-config")
                install_info = {
                    'type': 'pkg-config',
                    'detected_by': 'pkg-config'
                }
                installations.append(install_info)
        except:
            pass
        
        try:
            result = subprocess.run(["ldconfig", "-p"], capture_output=True, text=True)
            if "GooeyGUI-1" in result.stdout:
                self.log("Found GooeyGUI-1 in ldconfig cache")
                install_info = {
                    'type': 'ldconfig',
                    'detected_by': 'ldconfig'
                }
                installations.append(install_info)
        except:
            pass
        
        return installations

    def check_installation(self):
        self.install_info = self.load_install_record()
        
        system_installations = self.check_system_installation()
        
        if system_installations:
            self.installation_found = True
            self.log(f"Found {len(system_installations)} system installation(s)")
            
            if not self.install_info:
                self.install_info = {
                    'version': 'unknown',
                    'install_path': '/usr/local/Gooey',
                    'install_date': 'unknown',
                    'system_installations': system_installations,
                    'detection_method': 'system_scan'
                }
            else:
                self.install_info['system_installations'] = system_installations
                self.install_info['detection_method'] = 'combined'
            
            return True
        
        if self.install_info:
            install_path = Path(self.install_info.get('install_path', ''))
            if install_path.exists():
                self.installation_found = True
                self.log(f"Found existing installation from record")
                self.log(f"  Version: {self.install_info.get('version', 'unknown')}")
                self.log(f"  Path: {install_path}")
                self.log(f"  Date: {self.install_info.get('install_date', 'unknown')}")
                return True
        
        self.installation_found = False
        self.log("No existing installation found")
        return True

    def show_action_menu(self):
        if not self.stdscr:
            return self.show_text_action_menu()
        
        self.height, self.width = self.stdscr.getmaxyx()
        
        self.stdscr.clear()
        
        if curses.has_colors():
            self.stdscr.attron(curses.color_pair(5))
            for x in range(self.width):
                self.safe_add(0, x, " ")
            title = "Gooey GUI Library Installer"
            x_pos = max(0, (self.width - len(title)) // 2)
            self.safe_add(0, x_pos, title)
            self.stdscr.attroff(curses.color_pair(5))
        else:
            title = "Gooey GUI Library Installer"
            x_pos = max(0, (self.width - len(title)) // 2)
            self.safe_add(0, x_pos, title, curses.A_BOLD)
        
        platform_text = f"Platform: {self.system.upper()}"
        self.safe_add(2, 2, platform_text)
        
        if self.installation_found:
            self.safe_add(4, 2, "INSTALLATION DETECTED", curses.A_BOLD if not curses.has_colors() else curses.color_pair(3))
            
            if self.install_info and 'system_installations' in self.install_info:
                sys_installs = self.install_info.get('system_installations', [])
                self.safe_add(5, 4, f"Found {len(sys_installs)} installation(s) in system")
                
                for i, inst in enumerate(sys_installs[:3]):
                    if i < 3:
                        inst_type = inst.get('type', 'unknown')
                        inst_path = inst.get('path', 'unknown')
                        if len(inst_path) > 40:
                            inst_path = "..." + inst_path[-37:]
                        self.safe_add(6 + i, 6, f"{inst_type}: {inst_path}")
                
                if len(sys_installs) > 3:
                    self.safe_add(9, 6, f"... and {len(sys_installs) - 3} more")
                start_y = 11 if len(sys_installs) > 3 else 9
            elif self.install_info:
                self.safe_add(5, 4, f"Version: {self.install_info.get('version', 'unknown')}")
                self.safe_add(6, 4, f"Path: {self.install_info.get('install_path', 'unknown')}")
                self.safe_add(7, 4, f"Date: {self.install_info.get('install_date', 'unknown')}")
                start_y = 9
            else:
                start_y = 6
        else:
            self.safe_add(4, 2, "FRESH INSTALLATION", curses.A_BOLD if not curses.has_colors() else curses.color_pair(3))
            start_y = 6
        
        if start_y >= self.height - 6:
            start_y = max(4, self.height - 10)
        
        self.safe_add(start_y, 2, "SELECT ACTION:", curses.A_BOLD if not curses.has_colors() else curses.color_pair(3))
        
        actions = [
            ("I", "Install Gooey GUI Library"),
            ("R", "Reinstall (Clean & Install)"),
            ("D", "Delete Existing Installation"),
            ("C", "Cancel Installation"),
        ]
        
        if not self.installation_found:
            actions = [actions[0], actions[3]]
        
        selected = 0
        
        while True:
            for i, (key, desc) in enumerate(actions):
                line_y = start_y + 2 + i
                if line_y >= self.height - 1:
                    break
                    
                if i == selected:
                    attr = curses.color_pair(1) if curses.has_colors() else curses.A_REVERSE
                else:
                    attr = curses.color_pair(4) if curses.has_colors() else curses.A_NORMAL
                
                line = f"  [{key}] {desc}"
                self.safe_add(line_y, 4, line[:self.width-6], attr)
            
            footer_y = start_y + len(actions) + 3
            if footer_y < self.height - 2:
                self.safe_add(footer_y, 2, "-" * min(self.width - 4, 78), curses.A_DIM if not curses.has_colors() else curses.color_pair(4))
                help_text = "Arrow keys: Select  Enter: Confirm  Q: Quit"
                help_x = max(2, (self.width - len(help_text)) // 2)
                self.safe_add(footer_y + 1, help_x, help_text[:self.width-help_x-1], curses.A_DIM if not curses.has_colors() else curses.color_pair(6))
            
            self.stdscr.refresh()
            
            key = self.stdscr.getch()
            
            if key == curses.KEY_UP:
                selected = max(0, selected - 1)
            elif key == curses.KEY_DOWN:
                selected = min(len(actions) - 1, selected + 1)
            elif key == 10:
                action_map = {'I': 'install', 'R': 'reinstall', 'D': 'delete', 'C': 'cancel'}
                self.user_action = action_map.get(actions[selected][0], 'cancel')
                break
            elif key == ord('q') or key == ord('Q'):
                self.user_action = 'cancel'
                break
            elif key in [ord('i'), ord('I')]:
                self.user_action = 'install'
                break
            elif key in [ord('r'), ord('R')] and self.installation_found:
                self.user_action = 'reinstall'
                break
            elif key in [ord('d'), ord('D')] and self.installation_found:
                self.user_action = 'delete'
                break
            elif key in [ord('c'), ord('C')]:
                self.user_action = 'cancel'
                break
        
        return self.user_action != 'cancel'

    def show_text_action_menu(self):
        print("\n" + "=" * 60)
        print("   Gooey GUI Library Installer")
        print("=" * 60)
        
        if self.installation_found:
            print("\nINSTALLATION DETECTED")
            if self.install_info and 'system_installations' in self.install_info:
                sys_installs = self.install_info.get('system_installations', [])
                print(f"   Found {len(sys_installs)} installation(s) in system:")
                for i, inst in enumerate(sys_installs[:5]):
                    inst_type = inst.get('type', 'unknown')
                    inst_path = inst.get('path', 'unknown')
                    print(f"   {i+1}. {inst_type}: {inst_path}")
                if len(sys_installs) > 5:
                    print(f"   ... and {len(sys_installs) - 5} more")
            elif self.install_info:
                print(f"   Version: {self.install_info.get('version', 'unknown')}")
                print(f"   Path: {self.install_info.get('install_path', 'unknown')}")
                print(f"   Date: {self.install_info.get('install_date', 'unknown')}")
            print("\n" + "-" * 60)
        
        print("\nSELECT ACTION:")
        print("  [I] Install Gooey GUI Library")
        if self.installation_found:
            print("  [R] Reinstall (Clean & Install)")
            print("  [D] Delete Existing Installation")
        print("  [C] Cancel Installation")
        print("\n" + "-" * 60)
        
        while True:
            choice = input("\nSelect action: ").lower().strip()
            
            if choice == 'i':
                self.user_action = 'install'
                return True
            elif choice == 'r' and self.installation_found:
                self.user_action = 'reinstall'
                return True
            elif choice == 'd' and self.installation_found:
                self.user_action = 'delete'
                return True
            elif choice == 'c' or choice == 'q':
                self.user_action = 'cancel'
                return False
            else:
                print("Invalid choice. Please enter I, R, D, or C.")

    def delete_system_installation(self):
        """Delete system-wide Gooey installation"""
        self.log("Starting deletion of system installation...")
        
        targets = []
        
        if self.install_info and 'system_installations' in self.install_info:
            sys_installs = self.install_info.get('system_installations', [])
            for inst in sys_installs:
                if 'path' in inst:
                    targets.append(inst['path'])
        
        common_targets = [
            "/usr/local/Gooey",
            "/usr/local/lib/libGooeyGUI-1*",
            "/usr/local/include/Gooey",
            "/usr/lib/libGooeyGUI-1*",
            "/usr/include/Gooey",
        ]
        
        for target in common_targets:
            targets.append(target)
        
        targets = list(set(targets))
        
        success = True
        removed_count = 0
        
        for target in targets:
            find_cmd = ["sh", "-c", f"ls -d {target} 2>/dev/null || true"]
            try:
                result = subprocess.run(find_cmd, capture_output=True, text=True)
                if result.stdout.strip():
                    self.log(f"Removing: {target}")
                    if self.run_cmd(["sudo", "rm", "-rf"] + result.stdout.strip().split()):
                        removed_count += 1
                    else:
                        success = False
                else:
                    self.log(f"Not found (skipping): {target}")
            except Exception as e:
                self.log(f"Error checking {target}: {e}")
                success = False
        
        if removed_count > 0:
            self.log(f"Removed {removed_count} item(s)")
            
            self.log("Updating library cache...")
            self.run_cmd(["sudo", "ldconfig"])
            
            if self.install_record_path.exists():
                self.install_record_path.unlink()
                self.log("Removed installation record")
            
            self.installation_found = False
            self.install_info = None
        
        return success

    def delete_installation(self):
        self.log("Starting deletion of existing installation...")
        
        if self.install_info and 'install_path' in self.install_info:
            install_path = Path(self.install_info.get('install_path', ''))
            
            if install_path.exists():
                self.log(f"Removing {install_path}...")
                if self.run_cmd(["sudo", "rm", "-rf", str(install_path)]):
                    self.log(f"Removed {install_path}")
                else:
                    self.log("Failed to remove installation path")
        
        system_success = self.delete_system_installation()
        
        if self.install_record_path.exists():
            self.install_record_path.unlink()
            self.log("Removed installation record")
        
        self.installation_found = False
        self.install_info = None
        
        return system_success

    def log(self, msg):
        timestamp = datetime.now().strftime("%H:%M:%S")
        log_msg = f"[{timestamp}] {msg}"
        self.logs.append(log_msg)
        if not self.stdscr:
            print(log_msg)

    def safe_add(self, y, x, txt, attr=0):
        """Safely add text to screen, ensuring it stays within bounds"""
        if not self.stdscr or y < 0 or x < 0:
            return
            
        if y >= self.height or x >= self.width:
            return
            
        available = self.width - x
        if available <= 0:
            return
            
        if isinstance(txt, str):
            txt = txt[:available]
            try:
                self.stdscr.addstr(y, x, txt, attr)
            except curses.error:
                for i, char in enumerate(txt):
                    if x + i >= self.width:
                        break
                    try:
                        self.stdscr.addch(y, x + i, char, attr)
                    except:
                        pass

    def init_screen(self):
        if self.stdscr:
            curses.curs_set(0)
            self.stdscr.keypad(True)
            self.height, self.width = self.stdscr.getmaxyx()

    def draw(self):
        if not self.stdscr:
            return
            
        self.stdscr.clear()
        self.height, self.width = self.stdscr.getmaxyx()

        if curses.has_colors():
            self.stdscr.attron(curses.color_pair(5))
            for x in range(self.width):
                self.safe_add(0, x, " ")
            title = "Gooey GUI Installation"
            x_pos = max(0, (self.width - len(title)) // 2)
            self.safe_add(0, x_pos, title)
            self.stdscr.attroff(curses.color_pair(5))
        else:
            title = "Gooey GUI Installation"
            x_pos = max(0, (self.width - len(title)) // 2)
            self.safe_add(0, x_pos, title, curses.A_BOLD)
        
        status = f"Status: {self.user_action.upper() if self.user_action else 'SELECTING'}"
        self.safe_add(1, 2, status[:self.width-3], curses.color_pair(3) if curses.has_colors() else curses.A_BOLD)
        
        if self.width > 30 and self.height > 5:
            bar_w = min(self.width - 20, 50)
            progress = int((self.current_step / len(self.steps)) * bar_w) if len(self.steps) > 0 else 0
            bar = "[" + "=" * progress + " " * (bar_w - progress) + "]"
            percent = int((self.current_step / len(self.steps)) * 100) if len(self.steps) > 0 else 0
            progress_text = f"{self.spinner[self.spin_index]} {bar} {percent:3d}%"
            self.safe_add(3, 2, progress_text[:self.width-3], curses.color_pair(6) if curses.has_colors() else curses.A_BOLD)
            y = 5
        else:
            y = 3
        
        visible_steps = min(len(self.steps), self.height - y - 6)
        if visible_steps < 1:
            visible_steps = 1
        
        for i in range(visible_steps):
            step_idx = i
            if step_idx >= len(self.steps):
                break
                
            step = self.steps[step_idx]
            line_y = y + i
            
            if line_y >= self.height - 2:
                break
                
            if step_idx == self.current_step:
                step_text = f"> {step}"
                attr = curses.color_pair(1) if curses.has_colors() else curses.A_REVERSE
            elif self.step_status[step_idx] is True:
                step_text = f"[*] {step}"
                attr = curses.color_pair(2) if curses.has_colors() else curses.A_BOLD
            elif self.step_status[step_idx] is False:
                step_text = f"[ ] {step}"
                attr = curses.color_pair(4) if curses.has_colors() else curses.A_NORMAL
            else:
                step_text = f"  {step}"
                attr = curses.color_pair(4) if curses.has_colors() else curses.A_NORMAL
            
            self.safe_add(line_y, 2, step_text[:self.width-4], attr)
        
        log_start = y + visible_steps + 1
        if log_start < self.height - 5:
            self.safe_add(log_start, 2, "-" * min(self.width - 4, 78), curses.A_DIM if not curses.has_colors() else curses.color_pair(4))
            log_start += 1
            
            log_title = "Log Output:"
            self.safe_add(log_start, 2, log_title[:self.width-4], curses.A_BOLD if not curses.has_colors() else curses.color_pair(3))
            log_start += 1
            
            log_lines = self.height - log_start - 2
            if log_lines > 0:
                log_start_idx = max(0, len(self.logs) - log_lines)
                for i in range(log_lines):
                    if log_start_idx + i < len(self.logs):
                        log_line = self.logs[log_start_idx + i]
                        self.safe_add(log_start + i, 2, log_line[:self.width-4], curses.A_DIM if not curses.has_colors() else curses.color_pair(6))
        
        if curses.has_colors():
            self.stdscr.attron(curses.color_pair(7))
            for x in range(self.width):
                self.safe_add(self.height - 1, x, " ")
            help_text = "Press Q to quit"
            help_x = max(0, (self.width - len(help_text)) // 2)
            self.safe_add(self.height - 1, help_x, help_text)
            self.stdscr.attroff(curses.color_pair(7))
        else:
            help_text = "Press Q to quit"
            help_x = max(2, (self.width - len(help_text)) // 2)
            self.safe_add(self.height - 1, help_x, help_text[:self.width-help_x-1], curses.A_REVERSE)
        
        self.stdscr.refresh()
        self.spin_index = (self.spin_index + 1) % len(self.spinner)

    def handle_input(self):
        if not self.stdscr:
            return True

        self.stdscr.nodelay(True)
        try:
            key = self.stdscr.getch()
            if key == ord('q') or key == ord('Q'):
                return self.confirm_quit()
        except:
            pass
        finally:
            self.stdscr.nodelay(False)
        return True

    def confirm_quit(self):
        height, width = self.stdscr.getmaxyx()
        
        popup_h = 7
        popup_w = min(40, width - 4)
        popup_y = max(0, (height - popup_h) // 2)
        popup_x = max(0, (width - popup_w) // 2)
        
        if popup_y + popup_h > height:
            popup_y = height - popup_h
        if popup_x + popup_w > width:
            popup_x = width - popup_w
        
        popup = curses.newwin(popup_h, popup_w, popup_y, popup_x)
        popup.box()
        
        title = "Confirm Quit"
        title_x = max(1, (popup_w - len(title)) // 2)
        popup.addstr(0, title_x, title, curses.A_BOLD if not curses.has_colors() else curses.color_pair(5))
        
        popup.addstr(2, 2, "Are you sure you want to quit?", curses.A_BOLD)
        popup.addstr(3, 2, "Installation will be incomplete.")
        
        popup.addstr(5, 4, "[Y] Yes, quit", curses.A_BOLD)
        if popup_w >= 35:
            popup.addstr(5, 20, "[N] No, continue", curses.A_BOLD)
        else:
            popup.addstr(5, 18, "[N] No", curses.A_BOLD)
        
        popup.refresh()
        
        while True:
            key = popup.getch()
            if key == ord('y') or key == ord('Y'):
                return False
            elif key == ord('n') or key == ord('N'):
                return True

    def run_cmd(self, cmd, cwd=None):
        self.log(f"$ {' '.join(cmd)}")
        
        needs_sudo = False
        sudo_commands = ['apt-get', 'apt', 'dnf', 'yum', 'pacman', 'zypper', 'apk']
        
        for sudo_cmd in sudo_commands:
            if sudo_cmd in cmd:
                needs_sudo = True
                break
        
        if 'make' in cmd and 'install' in cmd:
            needs_sudo = True
        
        if needs_sudo and not self.password_cached:
            if self.stdscr:
                height, width = self.stdscr.getmaxyx()
                
                popup_h = 7
                popup_w = min(60, width - 4)
                popup_y = max(0, (height - popup_h) // 2)
                popup_x = max(0, (width - popup_w) // 2)
                
                if popup_y + popup_h > height:
                    popup_y = height - popup_h
                if popup_x + popup_w > width:
                    popup_x = width - popup_w
                
                popup = curses.newwin(popup_h, popup_w, popup_y, popup_x)
                popup.box()
                
                title = "Authentication Required"
                title_x = max(1, (popup_w - len(title)) // 2)
                popup.addstr(0, title_x, title, curses.A_BOLD if not curses.has_colors() else curses.color_pair(5))
                
                cmd_str = ' '.join(cmd)
                popup.addstr(2, 2, "Command requires sudo privileges:", curses.A_BOLD)
                popup.addstr(3, 2, cmd_str[:popup_w-4], curses.color_pair(3) if curses.has_colors() else curses.A_NORMAL)
                popup.addstr(4, 2, "Enter sudo password: ", curses.A_BOLD)
                
                popup.refresh()
                
                curses.echo(False)
                password = ""
                while True:
                    try:
                        ch = popup.getch()
                    except:
                        break
                        
                    if ch == 10:
                        break
                    elif ch == 127 or ch == 8:
                        password = password[:-1]
                    elif 32 <= ch <= 126:
                        password += chr(ch)
                    
                    popup.move(4, 22)
                    popup.clrtoeol()
                    popup.addstr(4, 22, "*" * min(len(password), popup_w-23))
                
                curses.echo(True)
                self.sudo_password = password
            else:
                self.sudo_password = getpass.getpass("Enter sudo password: ")
            
            test_cmd = ['sudo', '-S', 'echo', 'test']
            proc = subprocess.Popen(
                test_cmd,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            stdout, stderr = proc.communicate(input=f"{self.sudo_password}\n")
            
            if proc.returncode != 0:
                self.log("ERROR: Invalid sudo password")
                return False
            
            self.password_cached = True
        
        try:
            if needs_sudo and self.sudo_password:
                sudo_cmd = ['sudo', '-S'] + cmd
                proc = subprocess.Popen(
                    sudo_cmd,
                    cwd=cwd,
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    bufsize=1
                )
                
                proc.stdin.write(f"{self.sudo_password}\n")
                proc.stdin.flush()
                
                for line in iter(proc.stdout.readline, ''):
                    line = line.rstrip()
                    if line:
                        self.log(f"  {line}")
                    if self.stdscr:
                        self.draw()
                        self.handle_input()
                
                proc.wait()
                return proc.returncode == 0
            else:
                proc = subprocess.Popen(
                    cmd,
                    cwd=cwd,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    bufsize=1
                )
                
                for line in iter(proc.stdout.readline, ''):
                    line = line.rstrip()
                    if line:
                        self.log(f"  {line}")
                    if self.stdscr:
                        self.draw()
                        self.handle_input()
                
                proc.wait()
                return proc.returncode == 0
                
        except Exception as e:
            self.log(f"ERROR: {e}")
            return False

    def check_system(self):
        self.log(f"System: {platform.system()} {platform.release()}")
        self.log(f"Architecture: {platform.machine()}")
        self.log(f"Python: {platform.python_version()}")
        return True

    def install_dependencies(self):
        if self.system == "linux":
            if shutil.which("apt"):
                return self.run_cmd(["sudo", "apt-get", "install", "-y",
                                     "git", "cmake", "build-essential",
                                     "libx11-dev", "libxrandr-dev",
                                     "libxinerama-dev", "libxcursor-dev", "libxi-dev"])
            elif shutil.which("dnf"):
                return self.run_cmd(["sudo", "dnf", "install", "-y",
                                     "git", "cmake", "gcc", "gcc-c++", "make",
                                     "libX11-devel", "libXrandr-devel",
                                     "libXinerama-devel", "libXcursor-devel", "libXi-devel"])
            elif shutil.which("pacman"):
                return self.run_cmd(["sudo", "pacman", "-S", "--noconfirm",
                                     "git", "cmake", "base-devel",
                                     "libx11", "libxrandr", "libxinerama", "libxcursor", "libxi"])
            elif shutil.which("zypper"):
                return self.run_cmd(["sudo", "zypper", "install", "-y",
                                     "git", "cmake", "gcc", "gcc-c++", "make",
                                     "libX11-devel", "libXrandr-devel",
                                     "libXinerama-devel", "libXcursor-devel", "libXi-devel"])
            elif shutil.which("apk"):
                return self.run_cmd(["sudo", "apk", "add",
                                     "git", "cmake", "build-base",
                                     "libx11-dev", "libxrandr-dev",
                                     "libxinerama-dev", "libxcursor-dev", "libxi-dev"])
        
        self.log("Note: Manual dependency installation may be required")
        return True

    def clone_repository(self):
        if self.source_dir.exists():
            if self.user_action == 'reinstall':
                self.log("Cleaning existing source directory...")
                shutil.rmtree(self.source_dir)
            else:
                self.log("Source directory already exists, skipping clone")
                return True
                
        return self.run_cmd(["git", "clone",
                             "https://github.com/GooeyUI/GooeyGUI.git",
                             str(self.source_dir)])

    def init_submodules(self):
        return self.run_cmd(["git", "submodule", "init"], self.source_dir)

    def update_submodules(self):
        return self.run_cmd(["git", "submodule", "update", "--remote", "--force", "--merge"], self.source_dir)

    def configure_build(self):
        build = self.source_dir / "build"
        if build.exists():
            shutil.rmtree(build)
        build.mkdir(exist_ok=True)
        return self.run_cmd(["cmake", ".."], build)

    def find_user_config(self):
        possible_paths = [
            self.source_dir / "include" / "user_config.h",
            self.source_dir / "src" / "user_config.h",
            self.source_dir / "user_config.h",
            self.source_dir / "config" / "user_config.h"
        ]
        
        for path in possible_paths:
            if path.exists():
                self.user_config_path = path
                self.log(f"Found user_config.h at: {path}")
                return True
        
        for root, dirs, files in os.walk(self.source_dir):
            for file in files:
                if file == "user_config.h":
                    self.user_config_path = Path(root) / file
                    self.log(f"Found user_config.h at: {self.user_config_path}")
                    return True
        
        self.log("Warning: user_config.h not found, using defaults")
        return True

    def parse_user_config(self):
        if not self.user_config_path or not self.user_config_path.exists():
            return []
        
        try:
            with open(self.user_config_path, 'r') as f:
                content = f.read()
        except Exception as e:
            self.log(f"Error reading user_config.h: {e}")
            return []
        
        configs = []
        lines = content.split('\n')
        i = 0
        
        while i < len(lines):
            line = lines[i].strip()
            
            if not line or line.startswith('//') or line.startswith('/*') or line.startswith('*'):
                if line.startswith('//') and len(line) > 2:
                    comment = line[2:].strip()
                    if comment and not comment.startswith('===') and not comment.startswith('---'):
                        configs.append(ConfigOption(
                            name="",
                            description=comment,
                            config_type=ConfigType.COMMENT,
                            default_value=None
                        ))
                i += 1
                continue

            if line.startswith('#define '):
                parts = line.split(maxsplit=2)
                if len(parts) >= 2:
                    name = parts[1]
                    
                    if len(parts) == 2 or (len(parts) == 3 and not parts[2]):
                        config_type = ConfigType.BOOLEAN
                        default_value = True
                        value_str = ""
                    else:
                        value = parts[2].strip()
                        
                        if value.startswith('"') and value.endswith('"'):
                            config_type = ConfigType.STRING
                            default_value = value[1:-1]
                        elif value.replace('-', '').replace('.', '').isdigit():
                            config_type = ConfigType.INTEGER
                            try:
                                if '.' in value:
                                    default_value = float(value)
                                else:
                                    default_value = int(value)
                            except ValueError:
                                default_value = 0
                        elif any(op in value for op in ['+', '-', '*', '/', '(', ')', '<<', '>>']):
                            config_type = ConfigType.INTEGER
                            default_value = 0
                        else:
                            config_type = ConfigType.STRING
                            default_value = value
                    
                    description = ""
                    if i > 0:
                        prev_line = lines[i-1].strip()
                        if prev_line.startswith('//'):
                            description = prev_line[2:].strip()
                    
                    if not name.endswith("_H"):
                        config = ConfigOption(
                            name=name,
                            description=description,
                            config_type=config_type,
                            default_value=default_value
                        )
                        
                        if len(parts) > 2 and config_type != ConfigType.BOOLEAN:
                            config.value = parts[2].strip()
                        
                        configs.append(config)
            
            elif line.startswith('#undef '):
                parts = line.split()
                if len(parts) >= 2:
                    name = parts[1]
                    
                    description = ""
                    if i > 0:
                        prev_line = lines[i-1].strip()
                        if prev_line.startswith('//'):
                            description = prev_line[2:].strip()
                    
                    config = ConfigOption(
                        name=name,
                        description=description,
                        config_type=ConfigType.BOOLEAN,
                        default_value=False
                    )
                    configs.append(config)
            
            i += 1
        
        return configs

    def write_user_config(self, configs):
        if not self.user_config_path:
            return False
        
        try:
            with open(self.user_config_path, 'r') as f:
                original_lines = f.readlines()
        except Exception as e:
            self.log(f"Error reading original user_config.h: {e}")
            return False
        
        config_map = {}
        for config in configs:
            if config.name:
                if config.type == ConfigType.BOOLEAN:
                    config_map[config.name] = config.value
                else:
                    config_map[config.name] = config.value
        
        new_lines = []
        i = 0
        
        while i < len(original_lines):
            line = original_lines[i].rstrip()
            
            if line.startswith('#define '):
                parts = line.split(maxsplit=2)
                if len(parts) >= 2:
                    name = parts[1]
                    if name in config_map:
                        if config_map[name] is True:
                            new_lines.append(f"#define {name}")
                        elif config_map[name] is False:
                            new_lines.append(f"//#define {name}  // Disabled")
                        else:
                            new_lines.append(f"#define {name} {config_map[name]}")
                    else:
                        new_lines.append(line)
                else:
                    new_lines.append(line)
            
            elif line.startswith('#undef '):
                parts = line.split()
                if len(parts) >= 2:
                    name = parts[1]
                    if name in config_map:
                        if config_map[name] is True:
                            new_lines.append(f"#define {name}")
                        else:
                            new_lines.append(line)
                    else:
                        new_lines.append(line)
                else:
                    new_lines.append(line)
            
            else:
                new_lines.append(line)
            
            i += 1
        
        try:
            with open(self.user_config_path, 'w') as f:
                f.write('\n'.join(new_lines))
            self.log(f"Updated {self.user_config_path}")
            return True
        except Exception as e:
            self.log(f"Error writing user_config.h: {e}")
            return False

    def modify_user_config(self):
        if not self.user_config_path or not self.user_config_path.exists():
            self.log("Note: Using default configuration")
            return True
        
        if not self.stdscr:
            self.log("Note: Running in non-interactive mode, using defaults")
            return True
        
        self.stdscr.clear()
        
        self.safe_add(2, 2, "Parsing user_config.h...", curses.A_BOLD if not curses.has_colors() else curses.color_pair(3))
        self.stdscr.refresh()
        time.sleep(0.5)
        
        configs = self.parse_user_config()
        
        if not configs:
            self.stdscr.clear()
            self.safe_add(2, 2, "No configuration options found", curses.color_pair(4) if curses.has_colors() else curses.A_NORMAL)
            self.safe_add(4, 2, "Using default configuration...", curses.A_DIM if not curses.has_colors() else curses.color_pair(6))
            self.safe_add(6, 2, "Press any key to continue...")
            self.stdscr.refresh()
            self.stdscr.getch()
            return True

        menu = MenuConfig(self.stdscr)
        
        for config in configs:
            menu.add_option(config)
        
        self.stdscr.clear()
        result = menu.run()
        
        if result:
            if self.write_user_config(configs):
                self.stdscr.clear()
                self.safe_add(2, 2, "Configuration saved successfully!", curses.color_pair(2) if curses.has_colors() else curses.A_BOLD)
                self.safe_add(4, 2, "Press any key to continue...")
                self.stdscr.refresh()
                self.stdscr.getch()
                return True
            else:
                self.stdscr.clear()
                self.safe_add(2, 2, "Error saving configuration!", curses.color_pair(4) if curses.has_colors() else curses.A_BOLD)
                self.safe_add(4, 2, "Press any key to continue anyway...")
                self.stdscr.refresh()
                self.stdscr.getch()
                return True
        else:
            self.stdscr.clear()
            self.safe_add(2, 2, "Configuration not saved.", curses.A_BOLD if not curses.has_colors() else curses.color_pair(3))
            self.safe_add(4, 2, "Using default configuration.")
            self.safe_add(6, 2, "Press any key to continue...")
            self.stdscr.refresh()
            self.stdscr.getch()
            return True

    def compile_library(self):
        cpu_count = os.cpu_count() or 4
        return self.run_cmd(["cmake", "--build", ".", f"--parallel={cpu_count}"], 
                           self.source_dir / "build")

    def install_library(self):
        return self.run_cmd(["sudo", "make", "install"], self.source_dir / "build")

    def record_installation(self):
        install_info = {
            'version': '1.0.2',
            'install_path': str(self.install_dir),
            'install_date': datetime.now().isoformat(),
            'system': self.system,
            'architecture': platform.machine(),
            'action': self.user_action,
            'source_dir': str(self.source_dir),
        }
        
        if self.save_install_record(install_info):
            self.log(f"Installation recorded: {self.install_record_path}")
            return True
        else:
            self.log("Failed to record installation")
            return True

    def run_installation(self):
        if self.user_action == 'delete':
            self.steps = ["Delete existing installation"]
            self.step_funcs = [self.delete_installation]
            self.step_status = [None]
        
        for i in range(len(self.steps)):
            self.current_step = i
            
            if self.stdscr:
                self.draw()
            
            if not self.handle_input():
                self.log("Installation cancelled")
                return False

            self.log(f"Starting: {self.steps[i]}")
            
            ok = self.step_funcs[i]()
            self.step_status[i] = ok
            
            if not ok:
                self.log(f"FAILED: {self.steps[i]}")
                if self.stdscr:
                    self.draw()
                    self.stdscr.getch()
                return False
            
            time.sleep(0.3)
        
        self.show_completion_screen()
        return True

    def show_completion_screen(self):
        if not self.stdscr:
            self.show_text_completion()
            return
        
        self.height, self.width = self.stdscr.getmaxyx()
        self.stdscr.clear()
        
        if curses.has_colors():
            self.stdscr.attron(curses.color_pair(5))
            for x in range(self.width):
                self.safe_add(0, x, " ")
            title = "Installation Complete!"
            x_pos = max(0, (self.width - len(title)) // 2)
            self.safe_add(0, x_pos, title)
            self.stdscr.attroff(curses.color_pair(5))
        else:
            title = "Installation Complete!"
            x_pos = max(0, (self.width - len(title)) // 2)
            self.safe_add(0, x_pos, title, curses.A_BOLD)
        
        y = 2
        
        if self.user_action == 'delete':
            message = "Gooey GUI Library has been successfully removed."
            self.safe_add(y, 2, message[:self.width-4], curses.color_pair(3) if curses.has_colors() else curses.A_BOLD)
            y += 2
            
            details = [
                f"Action: {self.user_action.upper()}",
                f"Removed: {self.install_dir}",
                "",
                "The library has been completely uninstalled.",
            ]
        else:
            message = "Gooey GUI Library has been successfully installed!"
            self.safe_add(y, 2, message[:self.width-4], curses.color_pair(2) if curses.has_colors() else curses.A_BOLD)
            y += 2
            
            details = [
                f"Version: 1.0.2",
                f"Path: {self.install_dir}",
                f"Action: {self.user_action.upper()}",
                f"Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
                "",
                "To use the library in your projects:",
                "  #include <Gooey/gooey.h>",
                "  Link with: -lGooeyGUI-1",
            ]
        
        for line in details:
            if y < self.height - 2:
                self.safe_add(y, 4, line[:self.width-6], curses.color_pair(4) if curses.has_colors() else curses.A_NORMAL)
                y += 1
        
        if curses.has_colors():
            self.stdscr.attron(curses.color_pair(7))
            for x in range(self.width):
                self.safe_add(self.height - 1, x, " ")
            exit_text = "Press any key to exit"
            exit_x = max(0, (self.width - len(exit_text)) // 2)
            self.safe_add(self.height - 1, exit_x, exit_text)
            self.stdscr.attroff(curses.color_pair(7))
        else:
            exit_text = "Press any key to exit"
            exit_x = max(2, (self.width - len(exit_text)) // 2)
            self.safe_add(self.height - 1, exit_x, exit_text[:self.width-exit_x-1], curses.A_REVERSE)
        
        self.stdscr.refresh()
        self.stdscr.getch()

    def show_text_completion(self):
        print("\n" + "=" * 60)
        if self.user_action == 'delete':
            print("   INSTALLATION REMOVED")
            print("=" * 60)
            print("\nGooey GUI Library has been successfully removed.")
            print(f"\n  Removed: {self.install_dir}")
            print("\n  The library has been completely uninstalled.")
        else:
            print("   INSTALLATION COMPLETE!")
            print("=" * 60)
            print("\nGooey GUI Library has been successfully installed!")
            print("\n  Installation Details:")
            print(f"    Version: 1.0.2")
            print(f"    Path: {self.install_dir}")
            print(f"    Action: {self.user_action}")
            print(f"    Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
            print("\n  To use the library in your projects:")
            print("    #include <Gooey/gooey.h>")
            print("    Link with: -lGooeyGUI-1")
        print("\n" + "=" * 60)

    def run(self):
        if not self.show_action_menu():
            return True
        
        if self.user_action == 'reinstall':
            self.log("Starting reinstallation process...")
            if self.installation_found:
                self.log("Removing old installation first...")
                if not self.delete_installation():
                    self.log("Failed to remove old installation")
                    return False
        
        if self.stdscr:
            self.init_screen()
        
        return self.run_installation()


def main():
    if HAS_CURSES:
        try:
            result = curses.wrapper(lambda stdscr: InstallationManager(stdscr).run())
            if not result:
                return 1
        except KeyboardInterrupt:
            print("\nInstallation cancelled by user")
            return 1
        except Exception as e:
            print(f"\nUnexpected error: {e}")
            return 1
    else:
        installer = InstallationManager()
        if not installer.run():
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())