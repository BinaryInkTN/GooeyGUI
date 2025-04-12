/*
 Copyright (c) 2025 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

 #include <GLPS/glps_thread.h>
 #include <stdio.h>

void* start(void *arg)
{
  printf("test \n");
}

 int main()
 {
  gthread_t thread;
  
  glps_thread_create(&thread, NULL, start, 0);

  glps_thread_join(thread, NULL);

  return 0;

 }