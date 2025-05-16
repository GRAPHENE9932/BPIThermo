#include "shared_defines.h"
#include "../f_cpu.h"

#if F_CPU != 8000000
#error "The leds_large_pwm_cycle function is only adapted for 8 MHz CPU."
#endif

; Takes one argument: uint8_t us.
; <us> MUST be in range [1; 255].
; Pulls the <PORT_ADDR_RCK_CLR_G>'s <BIT_G> pin low for <us> microseconds, sets it high
; and waits for <LED_DUTY_US> - <us> microseconds.
.global leds_large_pwm_cycle
leds_large_pwm_cycle:
    ; The only argument is passed in the R24. Save it also in R25 for later use.
    mov R25, R24
    cli             ; Disable all the interrupts.
    cbi PORT_ADDR_RCK_CLR_G, BIT_G ; Set LOW to the G pin, activating the LEDs.
leds_large_pwm_cycle_loop_1:
    dec R24         ; 1 cycle, sets the Z flag if the result is 0.
    nop             ; 5 cycles
    nop
    nop
    nop
    nop
    brne leds_large_pwm_cycle_loop_1 ; Jumps if the Z flag is not set. 1+1 cycles.
    sbi PORT_ADDR_RCK_CLR_G, BIT_G ; Set HIGH to the G pin, disabling the LEDs.
    sei             ; We won't enable the LEDs anymore, so we can enable interrupts.
    ldi R26, LED_DUTY_US ; Subtract <us> from <LED_DUTY_US> and use the result for countdown.
    sub R26, R25
    breq leds_large_pwm_cycle_end ; Jumps if <LED_DUTY_US> == <us>.
leds_large_pwm_cycle_loop_2:
    dec R26         ; 1 cycle
    nop             ; 5 cycles
    nop
    nop
    nop
    nop
    brne leds_large_pwm_cycle_loop_2 ; Jumps if the Z flag is not set. 1+1 cycles.
leds_large_pwm_cycle_end:
    ret

; Takes one argument: uint8_t cycles.
; <cycles> MUST be in range [0; 7].
; Pulls the <PORT_ADDR_RCK_CLR_G>'s <BIT_G> pin low for total of <cycles> clock cycles. Does so
; with pulses 4, 2 and 1 clock cycles wide.
.global leds_small_pwm_cycle
leds_small_pwm_cycle:
    ; The only argument is passed in the R24.
    ; This function consists of four parts and each part is destined for the corresponding
    ; bit in R24. The part resposible for the 2nd bit, for example, makes an impulse that's
    ; 2^2 = 4 clock cycles long. The same holds for the 1st and 0th bit. That way the
    ; total LOW time will be exactly R24 clock cycles.
    cli             ; Disable interrupts for the time being.
    bst R24, 2
    brtc leds_small_pwm_cycle_1st_bit
    cbi PORT_ADDR_RCK_CLR_G, BIT_G
    nop
    nop
    nop
    sbi PORT_ADDR_RCK_CLR_G, BIT_G
leds_small_pwm_cycle_1st_bit:
    bst R24, 1
    brtc leds_small_pwm_cycle_0th_bit
    cbi PORT_ADDR_RCK_CLR_G, BIT_G
    nop
    sbi PORT_ADDR_RCK_CLR_G, BIT_G
leds_small_pwm_cycle_0th_bit:
    bst R24, 0
    brtc leds_small_pwm_cycle_end
    cbi PORT_ADDR_RCK_CLR_G, BIT_G
    sbi PORT_ADDR_RCK_CLR_G, BIT_G
leds_small_pwm_cycle_end:
    sei             ; Enable interrupts.
    ret
