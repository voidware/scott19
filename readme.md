# Scott19 - Scott Adams Interpreter for the TRS-80!

![](adv05.png)

## Building

Example:
`make adv01`

Will build `adv01.dsk`, `adv01.cmd` and `adv01.cas`

## Running

Example, to run on model I, under DOS

`..\emu\sdltrs -model 1 -romfile ..\emu\model1.rom -scale 2 -disk0 ..\emu\newdos80.dsk -disk1 adv01.dsk -foreground 0x07e214`

Example, to run on model III, under DOS

`..\emu\sdltrs -model 3 -romfile3 ../emu/model3.rom -scale 2 -disk0 "../emu/ld3-531.dsk" -disk1 adv01.dsk -foreground 0x07e214`














