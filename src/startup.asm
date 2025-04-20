.org 0x0000
    rjmp RESET
    rjmp INT0_ISR
    rjmp INT1_ISR
    rjmp PCINT0_ISR
    rjmp PCINT1_ISR
    rjmp PCINT2_ISR
    rjmp PCINT3_ISR
    rjmp WDT_ISR
    rjmp TIM1_CAPT_ISR
    rjmp TIM1_COMPA_ISR
    rjmp TIM1_COMPB_ISR
    rjmp TIM1_OVF_ISR
    rjmp TIM0_COMPA_ISR
    rjmp TIM0_COMPB_ISR
    rjmp TIM0_OVF_ISR
    rjmp SPI_STC_ISR
    rjmp ADC_ISR
    rjmp EE_RDY_ISR
    rjmp ANA_COMP_ISR
    rjmp TWI_ISR

.extern main
    
.weak INT0_ISR
.set INT0_ISR, RESET

.weak INT1_ISR
.set INT1_ISR, RESET

.weak PCINT0_ISR
.set PCINT0_ISR, RESET

.weak PCINT1_ISR
.set PCINT1_ISR, RESET

.weak PCINT2_ISR
.set PCINT2_ISR, RESET

.weak PCINT3_ISR
.set PCINT3_ISR, RESET

.weak WDT_ISR
.set WDT_ISR, RESET

.weak TIM1_CAPT_ISR
.set TIM1_CAPT_ISR, RESET

.weak TIM1_COMPA_ISR
.set TIM1_COMPA_ISR, RESET

.weak TIM1_COMPB_ISR
.set TIM1_COMPB_ISR, RESET

.weak TIM1_OVF_ISR
.set TIM1_OVF_ISR, RESET

.weak TIM0_COMPA_ISR
.set TIM0_COMPA_ISR, RESET

.weak TIM0_COMPB_ISR
.set TIM0_COMPB_ISR, RESET

.weak TIM0_OVF_ISR
.set TIM0_OVF_ISR, RESET

.weak SPI_STC_ISR
.set SPI_STC_ISR, RESET

.weak ADC_ISR
.set ADC_ISR, RESET

.weak EE_RDY_ISR
.set EE_RDY_ISR, RESET

.weak ANA_COMP_ISR
.set ANA_COMP_ISR, RESET

.weak TWI_ISR
.set TWI_ISR, RESET

RESET:
    rjmp main
