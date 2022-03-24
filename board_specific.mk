INC_BOARD = -I ../../board_specific/ -I ../../apollo/
SRC_BOARD=$(wildcard ../../board_specific/*.c) $(wildcard ../../apollo/*.c)
