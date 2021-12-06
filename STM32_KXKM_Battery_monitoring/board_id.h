#ifndef BOARD_ID_H
#define BOARD_ID_H

/* Board ID definition. Set it here if you compile using Arduino.
 * If you compile and flash with the Makefile it will be updated later when uploading.
 */
uint32_t __attribute__((section (".boardId"))) BOARD_ID = 201;

#endif
