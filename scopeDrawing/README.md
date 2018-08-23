Using a Microchip MCP4822 SPI dual channel DAC this program is able to 'draw' text to an oscilloscope, or other suitable analog vector
display. Supported resolution is currently only 8 bits, which allows 256x256, though the DAC supports upto 12 bit, the AVR is not fast
enough to be any use at that point.

The software is currently not feature complete, but is mostly functional. more updates will come later.

Note that the pgmspace.h header is not actually needed, this was used in an older version but was bipassed when trying to track down a
nasty bug. I intend to reimplement this soon as the current application uses quite a lot of the available SRAM.
