; Takes only one parameter of type uint8_t: amount of cycles to waste divided by 8.
; Wastes exactly this many cycles. The argument can not be less than 2.
; void variable_delay_for_us(uint8_t us)
.global variable_delay_for_us
variable_delay_for_us: ; 2 cycles to do the rcall.
    ; The only argument is passed in the R24.
    dec R24         ; 1 cycle, extra decrement to account for the rcall and ret overhead.
variable_delay_for_us_loop_start:
    dec R24         ; 1 cycle, sets the Z flag if the result is 0.
    nop             ; 5 cycles
    nop
    nop
    nop
    nop
    brne variable_delay_for_us_loop_start ; Jumps if the Z flag is not set. 1+1 cycles.

    nop             ; 2 cycles
    nop
    ret             ; 4 cycles
