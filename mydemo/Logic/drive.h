
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#define  GPIO_A	 	0x00
#define  GPIO_B	 	0x01
#define  GPIO_C	 	0x02
#define  GPIO_D	 	0x03
#define  GPIO_E	 	0x04
#define  GPIO_F	 	0x05
#define  GPIO_G	 	0x06
#define  GPIO_H 	0x07


/* config GPx pin out */
#define	 IOCTL_GPIO_FUN_MASK				0x00000F0
#define	 IOCTL_GPX_MASK						0x000000F

#define  IOCTL_GPIO_SET_PIN_OUT				0x0000010			/* pin out  */
#define  IOCTL_GPA_SET_PIN_OUT				0x0000010			/* pin out  */
#define  IOCTL_GPB_SET_PIN_OUT				0x0000011			/* pin out  */
#define  IOCTL_GPC_SET_PIN_OUT				0x0000012			/* pin out  */
#define  IOCTL_GPD_SET_PIN_OUT				0x0000013			/* pin out  */
#define  IOCTL_GPE_SET_PIN_OUT				0x0000014			/* pin out  */
#define  IOCTL_GPF_SET_PIN_OUT				0x0000015			/* pin out  */
#define  IOCTL_GPG_SET_PIN_OUT				0x0000016			/* pin out  */
#define  IOCTL_GPH_SET_PIN_OUT				0x0000017			/* pin out  */


/* set GPx's multi pin's out */
/* one bit is 1, the pin is out */
/* one bit is 0, the pin don't change */
#define  IOCTL_GPIO_SET_MULTI_PIN_OUT		0x0000020
#define  IOCTL_GPA_SET_MULTI_PIN_OUT		0x0000020			
#define  IOCTL_GPB_SET_MULTI_PIN_OUT		0x0000021			
#define  IOCTL_GPC_SET_MULTI_PIN_OUT		0x0000022			
#define  IOCTL_GPD_SET_MULTI_PIN_OUT		0x0000023			
#define  IOCTL_GPE_SET_MULTI_PIN_OUT		0x0000024			
#define  IOCTL_GPF_SET_MULTI_PIN_OUT		0x0000025			
#define  IOCTL_GPG_SET_MULTI_PIN_OUT		0x0000026			
#define  IOCTL_GPH_SET_MULTI_PIN_OUT		0x0000027


/* config pin in   */
#define  IOCTL_GPIO_SET_PIN_IN				0x0000030	
#define  IOCTL_GPA_SET_PIN_IN				0x0000030
#define  IOCTL_GPB_SET_PIN_IN				0x0000031
#define  IOCTL_GPC_SET_PIN_IN				0x0000032
#define  IOCTL_GPD_SET_PIN_IN				0x0000033
#define  IOCTL_GPE_SET_PIN_IN				0x0000034
#define  IOCTL_GPF_SET_PIN_IN				0x0000035
#define  IOCTL_GPG_SET_PIN_IN				0x0000036
#define  IOCTL_GPH_SET_PIN_IN				0x0000037


/* set GPx's multi pin's in */
/* one bit is 1, the pin is in */
/* one bit is 0, the pin don't change */
#define  IOCTL_GPIO_SET_MULTI_PIN_IN		0x0000040
#define  IOCTL_GPA_SET_MULTI_PIN_IN			0x0000040	
#define  IOCTL_GPB_SET_MULTI_PIN_IN			0x0000041
#define  IOCTL_GPC_SET_MULTI_PIN_IN			0x0000042
#define  IOCTL_GPD_SET_MULTI_PIN_IN			0x0000043
#define  IOCTL_GPE_SET_MULTI_PIN_IN			0x0000044
#define  IOCTL_GPF_SET_MULTI_PIN_IN			0x0000045
#define  IOCTL_GPG_SET_MULTI_PIN_IN			0x0000046
#define  IOCTL_GPH_SET_MULTI_PIN_IN			0x0000047

/* set pin high */
#define  IOCTL_GPIO_SET_PIN					0x0000050
#define  IOCTL_GPA_SET_PIN					0x0000050
#define  IOCTL_GPB_SET_PIN					0x0000051
#define  IOCTL_GPC_SET_PIN					0x0000052
#define  IOCTL_GPD_SET_PIN					0x0000053
#define  IOCTL_GPE_SET_PIN					0x0000054
#define  IOCTL_GPF_SET_PIN					0x0000055
#define  IOCTL_GPG_SET_PIN					0x0000056
#define  IOCTL_GPH_SET_PIN					0x0000057
	

/* set GPx's MULTI pins */
/* one bit is 1, the pin is high */
/* one bit is 0, the pin don't change */
#define  IOCTL_GPIO_SET_MULTI_PIN			0x0000060			
#define  IOCTL_GPA_SET_MULTI_PIN			0x0000060	
#define  IOCTL_GPB_SET_MULTI_PIN			0x0000061		
#define  IOCTL_GPC_SET_MULTI_PIN			0x0000062	
#define  IOCTL_GPD_SET_MULTI_PIN			0x0000063	
#define  IOCTL_GPE_SET_MULTI_PIN			0x0000064	
#define  IOCTL_GPF_SET_MULTI_PIN			0x0000065	
#define  IOCTL_GPG_SET_MULTI_PIN			0x0000066	
#define  IOCTL_GPH_SET_MULTI_PIN			0x0000067	

/* set pin low   */
#define  IOCTL_GPIO_CLR_PIN					0x0000070
#define  IOCTL_GPA_CLR_PIN					0x0000070	
#define  IOCTL_GPB_CLR_PIN					0x0000071
#define  IOCTL_GPC_CLR_PIN					0x0000072
#define  IOCTL_GPD_CLR_PIN					0x0000073
#define  IOCTL_GPE_CLR_PIN					0x0000074
#define  IOCTL_GPF_CLR_PIN					0x0000075
#define  IOCTL_GPG_CLR_PIN					0x0000076
#define  IOCTL_GPH_CLR_PIN					0x0000077


/* clr GPx's all pins */
/* one bit is 1, the pin is low */
/* one bit is 0, the pin don't change */
#define  IOCTL_GPIO_CLR_MULTI_PIN				0x0000080			
#define  IOCTL_GPA_CLR_MULTI_PIN				0x0000080	
#define  IOCTL_GPB_CLR_MULTI_PIN				0x0000081	
#define  IOCTL_GPC_CLR_MULTI_PIN				0x0000082	
#define  IOCTL_GPD_CLR_MULTI_PIN				0x0000083
#define  IOCTL_GPE_CLR_MULTI_PIN				0x0000084
#define  IOCTL_GPF_CLR_MULTI_PIN				0x0000085	
#define  IOCTL_GPG_CLR_MULTI_PIN				0x0000086	
#define  IOCTL_GPH_CLR_MULTI_PIN				0x0000087	                                                                
                                                                
/* read pin's voltage level status */
#define  IOCTL_GPIO_READ_PIN				0x0000090	
#define  IOCTL_GPA_READ_PIN					0x0000090
#define  IOCTL_GPB_READ_PIN					0x0000091
#define  IOCTL_GPC_READ_PIN					0x0000092
#define  IOCTL_GPD_READ_PIN					0x0000093
#define  IOCTL_GPE_READ_PIN					0x0000094
#define  IOCTL_GPF_READ_PIN					0x0000095
#define  IOCTL_GPG_READ_PIN					0x0000096
#define  IOCTL_GPH_READ_PIN					0x0000097


/* read GPx's all pin's set */				
#define  IOCTL_GPIO_READ_ALL_PIN			0x00000A0			
#define  IOCTL_GPA_READ_ALL_PIN				0x00000A0
#define  IOCTL_GPB_READ_ALL_PIN				0x00000A1
#define  IOCTL_GPC_READ_ALL_PIN				0x00000A2
#define  IOCTL_GPD_READ_ALL_PIN				0x00000A3
#define  IOCTL_GPE_READ_ALL_PIN				0x00000A4
#define  IOCTL_GPF_READ_ALL_PIN				0x00000A5
#define  IOCTL_GPG_READ_ALL_PIN				0x00000A6
#define  IOCTL_GPH_READ_ALL_PIN				0x00000A7


/* enable GPx pin pull-up */
#define  IOCTL_GPIO_EN_PULLUP				0x00000B0	
#define  IOCTL_GPA_EN_PULLUP				0x00000B0
#define  IOCTL_GPB_EN_PULLUP				0x00000B1
#define  IOCTL_GPC_EN_PULLUP				0x00000B2
#define  IOCTL_GPD_EN_PULLUP				0x00000B3
#define  IOCTL_GPE_EN_PULLUP				0x00000B4
#define  IOCTL_GPF_EN_PULLUP				0x00000B5
#define  IOCTL_GPG_EN_PULLUP				0x00000B6
#define  IOCTL_GPH_EN_PULLUP				0x00000B7


/* enable GPx pin pull-up */
#define  IOCTL_GPIO_EN_MULTI_PIN_PULLUP		0x00000C0	
#define  IOCTL_GPA_EN_MULTI_PIN_PULLUP		0x00000C0
#define  IOCTL_GPB_EN_MULTI_PIN_PULLUP		0x00000C1
#define  IOCTL_GPC_EN_MULTI_PIN_PULLUP		0x00000C2
#define  IOCTL_GPD_EN_MULTI_PIN_PULLUP		0x00000C3
#define  IOCTL_GPE_EN_MULTI_PIN_PULLUP		0x00000C4
#define  IOCTL_GPF_EN_MULTI_PIN_PULLUP		0x00000C5
#define  IOCTL_GPG_EN_MULTI_PIN_PULLUP		0x00000C6
#define  IOCTL_GPH_EN_MULTI_PIN_PULLUP		0x00000C7


/* disable GPx pin pull-up */
#define  IOCTL_GPIO_DIS_PULLUP				0x00000D0	
#define  IOCTL_GPA_DIS_PULLUP				0x00000D0
#define  IOCTL_GPB_DIS_PULLUP				0x00000D1
#define  IOCTL_GPC_DIS_PULLUP				0x00000D2
#define  IOCTL_GPD_DIS_PULLUP				0x00000D3
#define  IOCTL_GPE_DIS_PULLUP				0x00000D4
#define  IOCTL_GPF_DIS_PULLUP				0x00000D5
#define  IOCTL_GPG_DIS_PULLUP				0x00000D6
#define  IOCTL_GPH_DIS_PULLUP				0x00000D7

/* disable GPx pin pull-up */
#define  IOCTL_GPIO_DIS_MULTI_PIN_PULLUP	0x00000E0	
#define  IOCTL_GPA_DIS_MULTI_PIN_PULLUP		0x00000E0
#define  IOCTL_GPB_DIS_MULTI_PIN_PULLUP		0x00000E1
#define  IOCTL_GPC_DIS_MULTI_PIN_PULLUP		0x00000E2
#define  IOCTL_GPD_DIS_MULTI_PIN_PULLUP		0x00000E3
#define  IOCTL_GPE_DIS_MULTI_PIN_PULLUP		0x00000E4
#define  IOCTL_GPF_DIS_MULTI_PIN_PULLUP		0x00000E5
#define  IOCTL_GPG_DIS_MULTI_PIN_PULLUP		0x00000E6
#define  IOCTL_GPH_DIS_MULTI_PIN_PULLUP		0x00000E7



/* read port's setting */
#define  IOCTL_GPIO_READ_PORT				0x00000F0			
#define  IOCTL_GPA_READ_PORT				0x00000F0			
#define  IOCTL_GPB_READ_PORT				0x00000F1			
#define  IOCTL_GPC_READ_PORT				0x00000F2			
#define  IOCTL_GPD_READ_PORT				0x00000F3			
#define  IOCTL_GPE_READ_PORT				0x00000F4			
#define  IOCTL_GPF_READ_PORT				0x00000F5			
#define  IOCTL_GPG_READ_PORT				0x00000F6			
#define  IOCTL_GPH_READ_PORT				0x00000F7			

				

#ifdef __cplusplus
}
#endif

#endif // __GPIO_H__

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif
	
#define IOCTL_SPI_LEFT			        0
#define IOCTL_SPI_MIDDLE				1 
#define IOCTL_SPI_RIGHT                 2

	
#ifdef __cplusplus
}
#endif

#endif // __SPI_H__

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


#ifndef __PWM_H__
#define __PWM_H__

#ifdef __cplusplus
extern "C" {
#endif
	
#define IOCTL_PWM_SET_PRESCALER         1
#define IOCTL_PWM_SET_DIVIDER           2 
#define IOCTL_PWM_START					3
#define IOCTL_PWM_GET_FREQUENCY         4
	
	
#ifdef __cplusplus
}
#endif

#endif // __PWM_H__


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif
	
	
#define  IOCTL_SET_ADC_CHANNEL		0x01
	
#define  IOCTL_SET_ADC_FREQUENCY    0x02
	
#define  IOCTL_GET_ADC_CURFREQ  	0x03
	
#define  IOCTL_SET_ADC_MAXFREQ		0x04
	
#define  IOCTL_SET_ADC_MINFREQ		0x05
	
	// ADCCON ¼Ä´æÆ÷
#define  ADCCON_EN_START 	     (1 << 0)
#define  ADCCON_DIS_START   	 (0 << 0)
	
#define  ADCCON_DIS_READSTART    (0 << 1)
#define  ADCCON_EN_READSTART     (1 << 1)
	
#define  ADCCON_NORMAL_MODE      (0 << 2)
#define  ADCCON_STANDBY_MODE     (1 << 2)
	
#define  ADCCON_CHANNEL_MSK	 	 (0x07 << 3)
#define  ADCCON_CHANNEL_0		 0
#define  ADCCON_CHANNEL_1		 1
#define  ADCCON_CHANNEL_2		 2
#define  ADCCON_CHANNEL_3		 3
#define  ADCCON_CHANNEL_4		 4
#define  ADCCON_CHANNEL_5		 5
#define  ADCCON_CHANNEL_6		 6
#define  ADCCON_CHANNEL_7		 7
	
#define  ADCCON_PRSCVL_MSK       (0xFF << 6) 
	
#define  ADCCON_PRSCEN_EN	  	 (1 << 14)
#define  ADCCON_PRSCEN_DIS		 (0 << 14)
	
#define  ADCCIN_CONVERTING		 (0 << 15)
#define  ADCCIN_CONVERT_END		 (1 << 15)
	
	// ADCTSC ¼Ä´æÆ÷
#define  ADCTSC_NORMAL_MODE	     (0x3 << 0)
	
#ifdef __cplusplus
}
#endif

#endif // __ADC_H__