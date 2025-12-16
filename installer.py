#!/usr/bin/env python3

import os
import sys
import platform
import subprocess
import shutil
import time
import webbrowser
import getpass
import re
from pathlib import Path
from enum import Enum

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
        self.width = 80
        self.height = 24
        self.current_menu = None
        
    def add_option(self, option):
        self.options.append(option)
    
    def draw(self):
        self.stdscr.clear()
        height, width = self.stdscr.getmaxyx()
        
        if self.menu_stack:
            menu_title = " > ".join(self.menu_stack)
            self.safe_add(0, 2, menu_title, curses.A_BOLD)
        else:
            self.safe_add(0, 2, "Gooey GUI Configuration", curses.A_BOLD)
        
        self.safe_add(1, 2, "Arrow keys: Navigate  Space: Toggle/Select  Enter: Edit  S: Save  Q: Quit")
        self.safe_add(2, 2, "-" * (width - 4))
        
        visible_height = height - 6
        start_idx = self.scroll_offset
        end_idx = min(start_idx + visible_height, len(self.options))
        
        for i in range(start_idx, end_idx):
            option = self.options[i]
            line_y = 4 + (i - start_idx)
            
            if not option.visible:
                continue
            
            marker = ">" if i == self.selected_index else " "
            
            if option.type == ConfigType.BOOLEAN:
                type_indicator = "[*]" if option.value else "[ ]"
            elif option.type == ConfigType.MENU:
                type_indicator = "--->"
            else:
                type_indicator = "( )"
            
            if option.type == ConfigType.COMMENT:
                line = f"  {option.description}"
                attr = curses.A_DIM
            else:
                line = f"{marker} {type_indicator} {option.name}"
                if option.type != ConfigType.MENU:
                    line += f" = {option.get_display_value()}"
                if option.description:
                    line += f"  -  {option.description}"
                
                attr = 0
                if i == self.selected_index:
                    attr |= curses.A_REVERSE
                if not option.enabled:
                    attr |= curses.A_DIM
            
            self.safe_add(line_y, 2, line[:width-4], attr)
        
        if 0 <= self.selected_index < len(self.options):
            selected = self.options[self.selected_index]
            if selected.help_text and height > visible_height + 8:
                help_y = height - 3
                self.safe_add(help_y, 2, "─" * (width - 4))
                self.safe_add(help_y + 1, 2, selected.help_text[:width-4])
        
        self.stdscr.refresh()
    
    def safe_add(self, y, x, text, attr=0):
        if not self.stdscr:
            return
        max_y, max_x = self.stdscr.getmaxyx()
        if 0 <= y < max_y and 0 <= x < max_x:
            try:
                text = text[:max_x - x - 1]
                self.stdscr.addstr(y, x, text, attr)
            except curses.error:
                pass
    
    def handle_input(self):
        key = self.stdscr.getch()
        
        if key == ord('q') or key == ord('Q'):
            return 'quit'
        elif key == ord('s') or key == ord('S'):
            return 'save'
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
        return None
    
    def adjust_scroll(self):
        height, _ = self.stdscr.getmaxyx()
        visible_height = height - 6
        
        if self.selected_index < self.scroll_offset:
            self.scroll_offset = self.selected_index
        elif self.selected_index >= self.scroll_offset + visible_height:
            self.scroll_offset = self.selected_index - visible_height + 1
    
    def edit_value(self, option):
        height, width = self.stdscr.getmaxyx()

        popup_h = 5
        popup_w = min(60, width - 4)
        popup_y = (height - popup_h) // 2
        popup_x = (width - popup_w) // 2
        
        popup = curses.newwin(popup_h, popup_w, popup_y, popup_x)
        popup.box()
        
        title = f"Edit {option.name}"
        popup.addstr(0, (popup_w - len(title)) // 2, title, curses.A_BOLD)
        popup.addstr(1, 2, f"Current value: {option.get_display_value()}")
        popup.addstr(2, 2, "New value: ")
        
        popup.refresh()
        
        curses.echo()
        popup.move(2, 14)
        new_value = popup.getstr().decode('utf-8')
        curses.noecho()
        
        if new_value:
            option.set_value(new_value)
        
        return True
    
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
            elif action == 'edit_value':
                option = self.options[self.selected_index]
                self.edit_value(option)
            elif action == 'enter_menu':
                pass


class GooeyTUIInstaller:
    def __init__(self, stdscr=None):
        self.stdscr = stdscr
        self.system = platform.system().lower()
        self.sudo_password = None
        self.password_cached = False

        self.source_dir = Path("gooey-source")
        self.install_dir = Path("/usr/local/Gooey")

        self.steps = [
            "Check Git",
            "Install dependencies",
            "Clone repository",
            "Init submodules",
            "Update submodules",
            "Configure build",
            "Find user_config.h",
            "Configure user_config.h",
            "Compile library",
            "Install library",
        ]

        self.step_funcs = [
            self.check_git,
            self.install_dependencies,
            self.clone_repository,
            self.init_submodules,
            self.update_submodules,
            self.configure_build,
            self.find_user_config,
            self.modify_user_config,
            self.compile_library,
            self.install_library,
        ]

        self.step_status = [None] * len(self.steps)
        self.current_step = 0

        self.logs = []
        self.spinner = ["⠋","⠙","⠹","⠸","⠼","⠴","⠦","⠧","⠇","⠏"]
        self.spin_index = 0
        self.height = 0
        self.width = 0
        self.user_config_path = None

    def log(self, msg):
        self.logs.append(msg)
        print(msg)

    def safe_add(self, y, x, txt, attr=0):
        if not self.stdscr:
            return
        if 0 <= y < self.height and 0 <= x < self.width:
            try:
                txt = txt[:self.width - x]
                self.stdscr.addstr(y, x, txt, attr)
            except curses.error:
                pass

    def init_screen(self):
        curses.curs_set(0)
        self.stdscr.keypad(True)
        self.height, self.width = self.stdscr.getmaxyx()

        if curses.has_colors():
            curses.start_color()
            curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)
            curses.init_pair(2, curses.COLOR_RED, curses.COLOR_BLACK)
            curses.init_pair(3, curses.COLOR_CYAN, curses.COLOR_BLACK)
            curses.init_pair(4, curses.COLOR_YELLOW, curses.COLOR_BLACK)
            curses.init_pair(5, curses.COLOR_MAGENTA, curses.COLOR_BLACK)

    def draw(self):
        if not self.stdscr:
            return
            
        self.stdscr.clear()

        title = "Gooey GUI Library Installer"
        self.safe_add(0, max(0, (self.width - len(title)) // 2), title, curses.A_BOLD)
        
        platform_text = f"Platform: {self.system.upper()}"
        self.safe_add(1, 2, platform_text)

        if self.width > 20:
            bar_w = min(self.width - 10, 50)
            progress = int((self.current_step / len(self.steps)) * bar_w)
            bar = "[" + "█" * progress + " " * (bar_w - progress) + "]"
            self.safe_add(3, 2, bar)

        y = 5
        for i, step in enumerate(self.steps):
            if y >= self.height - 8:
                break
                
            if i == self.current_step:
                spin = self.spinner[self.spin_index]
                step_text = f"{spin} {step}"
                color = curses.color_pair(3) if curses.has_colors() else curses.A_BOLD
                self.safe_add(y, 2, step_text, color | curses.A_BOLD)
            elif self.step_status[i] is True:
                step_text = f"✓ {step}"
                color = curses.color_pair(1) if curses.has_colors() else curses.A_NORMAL
                self.safe_add(y, 2, step_text, color)
            elif self.step_status[i] is False:
                step_text = f"✗ {step}"
                color = curses.color_pair(2) if curses.has_colors() else curses.A_NORMAL
                self.safe_add(y, 2, step_text, color)
            else:
                step_text = f"· {step}"
                self.safe_add(y, 2, step_text)
            y += 1

        if y < self.height - 5:
            self.safe_add(y, 2, "-" * (self.width - 4))
            y += 1

            log_start = max(0, len(self.logs) - (self.height - y - 2))
            visible_logs = self.logs[log_start:]
            
            for i, line in enumerate(visible_logs):
                if y + i < self.height - 2:
                    self.safe_add(y + i, 4, line[:self.width - 5])

        if self.height > 1:
            footer = "Press Q to quit"
            self.safe_add(self.height - 1, max(0, (self.width - len(footer)) // 2), footer)

        self.stdscr.refresh()
        self.spin_index = (self.spin_index + 1) % len(self.spinner)

    def handle_input(self):
        if not self.stdscr:
            return True

        self.stdscr.nodelay(True)
        try:
            key = self.stdscr.getch()
            if key == ord('q') or key == ord('Q'):
                return False
        except:
            pass
        finally:
            self.stdscr.nodelay(False)
        return True

    def run_cmd(self, cmd, cwd=None):
        self.log(f"> {' '.join(cmd)}")
        
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
                self.stdscr.clear()
                self.safe_add(2, 2, "SUDO PASSWORD REQUIRED", curses.A_BOLD | curses.color_pair(4))
                self.safe_add(4, 2, "This command requires sudo privileges:")
                self.safe_add(5, 2, f"{' '.join(cmd)}")
                self.safe_add(7, 2, "Enter sudo password: ")
                
                curses.echo(False)
                self.stdscr.refresh()
                
                password = ""
                while True:
                    ch = self.stdscr.getch()
                    if ch == 10:
                        break
                    elif ch == 127 or ch == 8:
                        password = password[:-1]
                    elif 32 <= ch <= 126:
                        password += chr(ch)
                    
                    self.stdscr.move(7, 21)
                    self.stdscr.clrtoeol()
                    self.stdscr.addstr(7, 21, "*" * len(password))
                
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

    def check_git(self):
        return self.run_cmd(["git", "--version"])

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
        
        return True

    def clone_repository(self):
        if self.source_dir.exists():
            shutil.rmtree(self.source_dir)
        return self.run_cmd(["git", "clone",
                             "https://github.com/GooeyUI/GooeyGUI.git",
                             str(self.source_dir)])

    def init_submodules(self):
        return self.run_cmd(["git", "submodule", "init"], self.source_dir)

    def update_submodules(self):
        return self.run_cmd(["git", "submodule", "update", "--remote", "--force", "--merge"], self.source_dir)

    def configure_build(self):
        build = self.source_dir / "build"
        build.mkdir(exist_ok=True)
        return self.run_cmd(["cmake", "..",
                             "-DCMAKE_BUILD_TYPE=Release",
                             "-DBUILD_SHARED_LIBS=ON",
                             "-DBUILD_EXAMPLES=ON"],
                             build)

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
        
        self.log("Error: user_config.h not found anywhere!")
        return False

    def parse_user_config(self):
        """Parse user_config.h and extract configuration options"""
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
                    if(not name.endswith("_H")):
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
        """Write modified configuration back to user_config.h"""
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
        if not self.user_config_path:
            self.log("Error: user_config.h path not found")
            return False
        
        if not self.stdscr:
            return True
        
        self.stdscr.clear()
        
        self.safe_add(0, 2, "Parsing user_config.h...", curses.A_BOLD)
        self.stdscr.refresh()
        
        configs = self.parse_user_config()
        
        if not configs:
            self.safe_add(2, 2, "No configuration options found or error parsing file.")
            self.safe_add(4, 2, "Press any key to continue with defaults...")
            self.stdscr.refresh()
            self.stdscr.getch()
            return True

        menu = MenuConfig(self.stdscr)
        
        for config in configs:
            menu.add_option(config)
        
        self.stdscr.clear()
        result = menu.run()
        
        if result:  # Save was pressed
            if self.write_user_config(configs):
                self.stdscr.clear()
                self.safe_add(2, 2, "Configuration saved successfully!", curses.color_pair(1) | curses.A_BOLD)
                self.safe_add(4, 2, "Press any key to continue...")
                self.stdscr.refresh()
                self.stdscr.getch()
                return True
            else:
                self.stdscr.clear()
                self.safe_add(2, 2, "Error saving configuration!", curses.color_pair(2) | curses.A_BOLD)
                self.safe_add(4, 2, "Press any key to continue anyway...")
                self.stdscr.refresh()
                self.stdscr.getch()
                return True
        else:  # Quit was pressed
            self.stdscr.clear()
            self.safe_add(2, 2, "Configuration not saved.", curses.A_BOLD)
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

    def run_installation(self):
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
        
        if self.stdscr:
            self.stdscr.clear()
            self.safe_add(2, 2, "Installation Complete!", curses.A_BOLD | curses.color_pair(1))
            self.safe_add(4, 2, f"Library installed to: {self.install_dir}")
        
            self.safe_add(10, 2, "Press any key to exit")
            self.stdscr.refresh()
            self.stdscr.getch()
        
        return True

    def run(self):
        if self.stdscr:
            self.init_screen()
        return self.run_installation()


def main():
    if HAS_CURSES:
        try:
            curses.wrapper(lambda stdscr: GooeyTUIInstaller(stdscr).run())
        except KeyboardInterrupt:
            print("\nInstallation cancelled by user")
            return 1
    else:
        installer = GooeyTUIInstaller()
        if not installer.run():
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())