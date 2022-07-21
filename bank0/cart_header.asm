# TI99/4A cart header. 
# This needs to be put at 0x6000, so the linker needs to target this file specifically

# GROM header
  byte 0xAA, 0x01, 0x00, 0x00

# Start of power-up chain (unused)
  data 0x0000

# Start of program chain
  data program_record

# Start of DSR chain (unused)
  data 0x0000 

# Start of subprogram list (unused)
# This doubles as the terminator of the program chain
program_record:
  data  0x0000    			# Next program chain record
  data  _start    			# Entry point for program
  nstring "MALLOC"  	# Name of program
  even
