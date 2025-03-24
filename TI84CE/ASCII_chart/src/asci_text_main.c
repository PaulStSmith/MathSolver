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
#include "headers/asci_text_main_public.h"

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
    char* char_str = "\0\0";
    asc2str(char_code, char_str);
    printf("%2x %s", char_code, char_str);
}

/**
 * Returns a 2-character representation for ASCII control character codes
 * 
 * @param code The ASCII code to convert (0x00-0x1F or 0x7F)
 * @param buffer A buffer of at least 3 bytes to store the result
 * @return Pointer to the buffer, now containing the 2-char representation and null terminator
 */
char* asc2str(unsigned char code, char* buffer) {
    if (buffer == NULL) {
        return NULL;
    }
    
    switch (code) {
        case 0x00: strcpy(buffer, "NL"); break; // Null
        case 0x01: strcpy(buffer, "SH"); break; // Start of Heading
        case 0x02: strcpy(buffer, "SX"); break; // Start of Text
        case 0x03: strcpy(buffer, "EX"); break; // End of Text
        case 0x04: strcpy(buffer, "ET"); break; // End of Transmission
        case 0x05: strcpy(buffer, "EQ"); break; // Enquiry
        case 0x06: strcpy(buffer, "AK"); break; // Acknowledge
        case 0x07: strcpy(buffer, "BL"); break; // Bell
        case 0x08: strcpy(buffer, "BS"); break; // Backspace
        case 0x09: strcpy(buffer, "HT"); break; // Horizontal Tab
        case 0x0A: strcpy(buffer, "LF"); break; // Line Feed
        case 0x0B: strcpy(buffer, "VT"); break; // Vertical Tab
        case 0x0C: strcpy(buffer, "FF"); break; // Form Feed
        case 0x0D: strcpy(buffer, "CR"); break; // Carriage Return
        case 0x0E: strcpy(buffer, "SO"); break; // Shift Out
        case 0x0F: strcpy(buffer, "SI"); break; // Shift In
        case 0x10: strcpy(buffer, "DL"); break; // Data Link Escape
        case 0x11: strcpy(buffer, "D1"); break; // Device Control 1 (XON)
        case 0x12: strcpy(buffer, "D2"); break; // Device Control 2
        case 0x13: strcpy(buffer, "D3"); break; // Device Control 3 (XOFF)
        case 0x14: strcpy(buffer, "D4"); break; // Device Control 4
        case 0x15: strcpy(buffer, "NK"); break; // Negative Acknowledge
        case 0x16: strcpy(buffer, "SY"); break; // Synchronous Idle
        case 0x17: strcpy(buffer, "EB"); break; // End of Transmission Block
        case 0x18: strcpy(buffer, "CN"); break; // Cancel
        case 0x19: strcpy(buffer, "EM"); break; // End of Medium
        case 0x1A: strcpy(buffer, "SB"); break; // Substitute
        case 0x1B: strcpy(buffer, "EC"); break; // Escape
        case 0x1C: strcpy(buffer, "FS"); break; // File Separator
        case 0x1D: strcpy(buffer, "GS"); break; // Group Separator
        case 0x1E: strcpy(buffer, "RS"); break; // Record Separator
        case 0x1F: strcpy(buffer, "US"); break; // Unit Separator
        case 0x7F: strcpy(buffer, "DT"); break; // Delete
        default:
            buffer[0] = code;
            buffer[1] = '\0';
            break; // Copy the character of the code
    }
    
    return buffer;
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
            int col = (i % 4) * 7;
            
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
