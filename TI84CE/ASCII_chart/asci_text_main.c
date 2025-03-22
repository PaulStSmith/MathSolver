/**
 * TI-84 CE Text Screen ASCII Chart Viewer
 * 
 * This program displays ASCII/extended ASCII characters (0-255)
 * on the TI-84 CE text screen, allowing the user to identify
 * special characters like π and their corresponding codes.
 */

 #include <tice.h>
 #include <keypadc.h>
 #include <string.h>
 #include <stdio.h>
 
 #define SCREEN_ROWS 8  // Number of text rows on the TI-84 CE
 #define SCREEN_COLS 26 // Number of text columns on the TI-84 CE
 
 void clear_screen(void) {
     os_ClrHome();
 }
 
 void print_header(int page, int total_pages) {
     os_SetCursorPos(0, 0);
     printf("ASCII Chart - Page %d/%d", page + 1, total_pages);
     printf("\nUP/DOWN to navigate");
     printf("\nCLEAR to exit");
 }
 
 void print_char_info(int char_code, int row, int col) {
     os_SetCursorPos(row, col);
     
     // Print a concise format showing code and character
     printf("%3d:%c", char_code, char_code);
 }
 
 int main(void) {
     int current_page = 0;
     int chars_per_page = 16; // Display 16 characters per page (4 rows of 4)
     int total_pages = 256 / chars_per_page;
     bool running = true;
     
     while (running) {
         // Clear screen and draw header
         clear_screen();
         print_header(current_page, total_pages);
         
         // Draw current page of characters
         int start_char = current_page * chars_per_page;
         
         for (int i = 0; i < chars_per_page; i++) {
             int char_code = start_char + i;
             
             // Layout: 4 rows of 4 characters starting at row 3
             int row = 3 + (i / 4);
             int col = (i % 4) * 6;
             
             print_char_info(char_code, row, col);
         }
         
         while (true)          {
            // Process key input
            kb_Scan();
            
            // Handle navigation
            if (kb_Data[7] & kb_Down && current_page < total_pages - 1) {
                current_page++;
                delay(200); // Debounce
                break;
            }
            if (kb_Data[7] & kb_Up && current_page > 0) {
                current_page--;
                delay(200); // Debounce
                break;
            }
            
            // Handle exit
            if (kb_Data[6] & kb_Clear) {
                running = false;
                break;
            }
            
            // Small delay for stability
            delay(100);
        }
     }

     return 0;
 }