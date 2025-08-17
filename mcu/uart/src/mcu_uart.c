/*==============================================================================
 * Include 
 *============================================================================*/

#include "mcu_uart.h"

#include "stm32wbxx_hal.h"


#include "string.h"


/*==============================================================================
 * Macro
 *============================================================================*/

/* Uart initialization config */
#define D_MCU_UART_INSTANCE                 USART1
#define D_MCU_UART_BAUDRATE                 115200
#define D_MCU_UART_WORDLENGTH               UART_WORDLENGTH_8B
#define D_MCU_UART_STOPBITS                 UART_STOPBITS_1
#define D_MCU_UART_PARITY                   UART_PARITY_NONE
#define D_MCU_UART_MODE	 		            UART_MODE_TX_RX
#define D_MCU_UART_HWFLOWCTL                UART_HWCONTROL_NONE
#define D_MCU_UART_OVERSAMPLING             UART_OVERSAMPLING_16
#define D_MCU_UART_ONEBITSAMPLING           UART_ONE_BIT_SAMPLE_DISABLE
#define D_MCU_UART_CLOCKPRESCALER           UART_PRESCALER_DIV1
#define D_MCU_UART_ADVFEATUREINIT           UART_ADVFEATURE_NO_INIT
#define D_MCU_UART_TXFIFO_THRESHOLD         UART_TXFIFO_THRESHOLD_1_8
#define D_MCU_UART_RXFIFO_THRESHOLD         UART_RXFIFO_THRESHOLD_1_8

/* Uart MSP initialization config */
#define D_MCU_UART_CLK_ENABLE()				__HAL_RCC_USART1_CLK_ENABLE()
#define D_MCU_UART_CLK_DISABLE()		    __HAL_RCC_USART1_CLK_DISABLE()

#define D_MCU_UART_TX_GPIO_PORT				GPIOB
#define D_MCU_UART_TX_GPIO_PIN				GPIO_PIN_6
#define D_MCU_UART_TX_GPIO_CLK_ENABLE()		__HAL_RCC_GPIOB_CLK_ENABLE()
#define D_MCU_UART_TX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOB_CLK_DISABLE()
#define D_MCU_UART_RX_GPIO_PORT				GPIOB
#define D_MCU_UART_RX_GPIO_PIN				GPIO_PIN_7
#define D_MCU_UART_RX_GPIO_CLK_ENABLE()		__HAL_RCC_GPIOB_CLK_ENABLE()
#define D_MCU_UART_RX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOB_CLK_DISABLE()

#define D_MCU_UART_AF						GPIO_AF7_USART1

#define D_MCU_UART_IRQ_NUMBER				USART1_IRQn
#define D_MCU_UART_IRQ_PRIORITY				(5)
#define D_MCU_UART_IRQ_SUBPRIORITY			(0)

#define D_MCU_UART_FORCE_RESET()		    __HAL_RCC_USART1_FORCE_RESET()						
#define D_MCU_UART_RELEASE_RESET()			__HAL_RCC_USART1_RELEASE_RESET()

#define D_MCU_UART_DMAMUX_CLK_ENABLE()		__HAL_RCC_DMAMUX1_CLK_ENABLE()
#define D_MCU_UART_TX_DMA_CLK_ENABLE()		__HAL_RCC_DMA1_CLK_ENABLE()
#define D_MCU_UART_RX_DMA_CLK_ENABLE()		__HAL_RCC_DMA1_CLK_ENABLE()

#define D_MCU_UART_TX_DMA_INSTANCE			DMA1_Channel1
#define D_MCU_UART_TX_DMA_REQUEST			DMA_REQUEST_USART1_TX
#define D_MCU_UART_RX_DMA_INSTANCE			DMA1_Channel2
#define D_MCU_UART_RX_DMA_REQUEST			DMA_REQUEST_USART1_RX

#define D_MCU_UART_TX_DMA_IRQ_NUMBER	    DMA1_Channel1_IRQn
#define D_MCU_UART_TX_DMA_IRQ_PRIORITY		(5)
#define D_MCU_UART_TX_DMA_IRQ_SUBPRIORITY	(0)

#define D_MCU_UART_RX_DMA_IRQ_NUMBER	    DMA1_Channel2_IRQn
#define D_MCU_UART_RX_DMA_IRQ_PRIORITY		(5)
#define D_MCU_UART_RX_DMA_IRQ_SUBPRIORITY	(0)

/* UART DMA buffer size */
#define D_MCU_UART_TX_DMA_BUFFER_SIZE		(D_MCU_UART_TRANSMIT_SIZE_MAX)	/* Byte */
#define D_MCU_UART_RX_DMA_BUFFER_SIZE		(D_MCU_UART_RECEIVE_SIZE_MAX)	/* Byte */


/*==============================================================================
 * Structure
 *============================================================================*/

typedef struct 
{
	E_MCU_UART_INIT_STATUS_T is_inited;

    UART_HandleTypeDef  uart_hal_handle;
    DMA_HandleTypeDef   tx_dma_hal_handle;
    DMA_HandleTypeDef   rx_dma_hal_handle;

    uint8_t*            p_tx_dma_buf;
    uint8_t*            p_rx_dma_buf;
    uint16_t            tx_dma_buf_size;   
    uint16_t            rx_dma_buf_size;
	uint16_t            rx_dma_buf_last_size;

    volatile E_MCU_UART_TX_STATUS_T tx_status;
	volatile E_MCU_UART_RX_STATUS_T rx_status;

	volatile PF_MCU_UART_TRANSMIT_COMPLETE_CALLBACK_T	pf_transmit_complete_callback;
	volatile PF_MCU_UART_RECEIVE_COMPLETE_CALLBACK_T	pf_receive_complete_callback;
	volatile PF_MCU_UART_RECEIVE_PROCESS_CALLBACK_T 	pf_receive_process_callback;
} S_MCU_UART_T;


/*==============================================================================
 * Global Variable
 *============================================================================*/

static uint8_t gs_mcu_uart_tx_dma_buf[D_MCU_UART_TX_DMA_BUFFER_SIZE];
static uint8_t gs_mcu_uart_rx_dma_buf[D_MCU_UART_RX_DMA_BUFFER_SIZE]; 

static S_MCU_UART_T gs_mcu_uart_handle = {0};


/*==============================================================================
 * Public Function Implementation
 *============================================================================*/

extern E_MCU_UART_RET_STATUS_T mcu_uart_init(void)
{
	/* Check UART initialization status */
	if (E_MCU_UART_INIT_STATUS_NO != gs_mcu_uart_handle.is_inited)
	{
		return E_MCU_UART_RET_STATUS_INIT_STATUS_ERR;
	}

    /* UART HAL initialization */
    UART_HandleTypeDef* p_uart_hal_handle 			= 	&(gs_mcu_uart_handle.uart_hal_handle);

	p_uart_hal_handle->Instance					    =	D_MCU_UART_INSTANCE;	
    p_uart_hal_handle->Init.BaudRate 			    = 	D_MCU_UART_BAUDRATE;
	p_uart_hal_handle->Init.WordLength 			    = 	D_MCU_UART_WORDLENGTH;
	p_uart_hal_handle->Init.StopBits 			    = 	D_MCU_UART_STOPBITS;
	p_uart_hal_handle->Init.Parity 				    = 	D_MCU_UART_PARITY;
	p_uart_hal_handle->Init.Mode 				    = 	D_MCU_UART_MODE;
	p_uart_hal_handle->Init.HwFlowCtl 			    = 	D_MCU_UART_HWFLOWCTL;
	p_uart_hal_handle->Init.OverSampling 		    = 	D_MCU_UART_OVERSAMPLING;
	p_uart_hal_handle->Init.OneBitSampling 		    = 	D_MCU_UART_ONEBITSAMPLING;
	p_uart_hal_handle->Init.ClockPrescaler 		    = 	D_MCU_UART_CLOCKPRESCALER;
	p_uart_hal_handle->AdvancedInit.AdvFeatureInit	= 	D_MCU_UART_ADVFEATUREINIT;

    HAL_StatusTypeDef ret_status_hal = HAL_OK;

    ret_status_hal = HAL_UART_Init(p_uart_hal_handle);      
    if (HAL_OK != ret_status_hal)
    {
        (void)ret_status_hal;

        return E_MCU_UART_RET_STATUS_RESOURCE_ERR;
    }

    ret_status_hal = HAL_UARTEx_SetTxFifoThreshold(p_uart_hal_handle, D_MCU_UART_TXFIFO_THRESHOLD);
    if (HAL_OK != ret_status_hal)
    {
        (void)ret_status_hal;

        return E_MCU_UART_RET_STATUS_RESOURCE_ERR;
    }

    ret_status_hal = HAL_UARTEx_SetRxFifoThreshold(p_uart_hal_handle, D_MCU_UART_RXFIFO_THRESHOLD);
    if (HAL_OK != ret_status_hal)
    {
        (void)ret_status_hal;

        return E_MCU_UART_RET_STATUS_RESOURCE_ERR;
    }

    ret_status_hal = HAL_UARTEx_DisableFifoMode(p_uart_hal_handle);
    if (HAL_OK != ret_status_hal)
    {
        (void)ret_status_hal;

        return E_MCU_UART_RET_STATUS_RESOURCE_ERR;
    }

	/* Update UART DMA buffer */
	gs_mcu_uart_handle.p_tx_dma_buf = gs_mcu_uart_tx_dma_buf;
	gs_mcu_uart_handle.p_rx_dma_buf = gs_mcu_uart_rx_dma_buf;
	gs_mcu_uart_handle.tx_dma_buf_size = D_MCU_UART_TX_DMA_BUFFER_SIZE;
	gs_mcu_uart_handle.rx_dma_buf_size = D_MCU_UART_RX_DMA_BUFFER_SIZE;

    /* Update UART TX status */
    gs_mcu_uart_handle.tx_status = E_MCU_UART_TX_STATUS_READY;

	/* Update UART RX status */
	gs_mcu_uart_handle.rx_status = E_MCU_UART_RX_STATUS_READY;

	/* Update UART initialization status */
	gs_mcu_uart_handle.is_inited = E_MCU_UART_INIT_STATUS_OK;

    return E_MCU_UART_RET_STATUS_OK;
}

extern E_MCU_UART_RET_STATUS_T mcu_uart_deinit(void)
{   
	/* Check UART initialization status */
	if (E_MCU_UART_INIT_STATUS_NO == gs_mcu_uart_handle.is_inited)
	{
		return E_MCU_UART_RET_STATUS_OK;
	}

    /* UART HAL deinitialization */
    HAL_StatusTypeDef ret_status_hal = HAL_OK;
    
    ret_status_hal = HAL_UART_DeInit(&(gs_mcu_uart_handle.uart_hal_handle));
    if (HAL_OK != ret_status_hal)
    {
		/* Ignore return value on purpose for best-effort cleanup */
        (void)ret_status_hal;
    }

	/* Reset UART DMA buffer */
	gs_mcu_uart_handle.p_tx_dma_buf = NULL;
	gs_mcu_uart_handle.p_rx_dma_buf = NULL;
	gs_mcu_uart_handle.tx_dma_buf_size = 0;
	gs_mcu_uart_handle.rx_dma_buf_size = 0;
	gs_mcu_uart_handle.rx_dma_buf_last_size = 0;

	/* Reset UART TX status */
	gs_mcu_uart_handle.tx_status = E_MCU_UART_TX_STATUS_NONE;

	/* Reset UART RX status */	
	gs_mcu_uart_handle.rx_status = E_MCU_UART_RX_STATUS_NONE;

	/* Reset UART initialization status */
	gs_mcu_uart_handle.is_inited = E_MCU_UART_INIT_STATUS_NO;

    return E_MCU_UART_RET_STATUS_OK;
}

extern E_MCU_UART_RET_STATUS_T mcu_uart_init_status_get(E_MCU_UART_INIT_STATUS_T* const p_init_status)
{
	/* Check input parameters */
	if (NULL == p_init_status)
	{
		return E_MCU_UART_RET_STATUS_INPUT_PARAM_ERR;
	}

	/* Get UART initialization status */
	*p_init_status = gs_mcu_uart_handle.is_inited;

	return E_MCU_UART_RET_STATUS_OK;
}

extern E_MCU_UART_RET_STATUS_T mcu_uart_transmit_dma_start(const uint8_t* const data, const uint16_t data_size)
{
    /* Check input parameters */
	/* Note: data size must not be 0 to ensure DMA is started and TX complete interrupt can be triggered */
    if (NULL == data || 0 == data_size || D_MCU_UART_TRANSMIT_SIZE_MAX < data_size)
    {
        return E_MCU_UART_RET_STATUS_INPUT_PARAM_ERR;
    }

    /* Check UART TX status */
    if(E_MCU_UART_TX_STATUS_READY != gs_mcu_uart_handle.tx_status)
    {
        return E_MCU_UART_RET_STATUS_TX_BUSY;
    }

    /* Copy data to DMA buffer */
    memcpy(gs_mcu_uart_handle.p_tx_dma_buf, data, data_size);

    /* Transmit data */
    HAL_StatusTypeDef ret_status_hal = HAL_UART_Transmit_DMA(&(gs_mcu_uart_handle.uart_hal_handle), gs_mcu_uart_handle.p_tx_dma_buf, data_size);
    if (HAL_OK != ret_status_hal)
    {
        /* Get UART HAL error code */
        uint32_t err_code = HAL_UART_GetError(&(gs_mcu_uart_handle.uart_hal_handle) );
        (void)err_code;

        return E_MCU_UART_RET_STATUS_RESOURCE_ERR;
    }

    /* Update UART DMA status */
    gs_mcu_uart_handle.tx_status = E_MCU_UART_TX_STATUS_BUSY;

    return E_MCU_UART_RET_STATUS_OK;
}

extern E_MCU_UART_RET_STATUS_T mcu_uart_transmit_status_get(E_MCU_UART_TX_STATUS_T* const p_tx_status)
{
	/* Check input parameters */
	if (NULL == p_tx_status)
	{
		return E_MCU_UART_RET_STATUS_INPUT_PARAM_ERR;
	}

	/* Check UART initialization status */
	if (E_MCU_UART_INIT_STATUS_OK != gs_mcu_uart_handle.is_inited)
	{
		return E_MCU_UART_RET_STATUS_INIT_STATUS_ERR;
	}

	/* Get UART TX status */
	*p_tx_status = gs_mcu_uart_handle.tx_status;

	return E_MCU_UART_RET_STATUS_OK;
}

extern E_MCU_UART_RET_STATUS_T mcu_uart_transmit_complete_callback_register(PF_MCU_UART_TRANSMIT_COMPLETE_CALLBACK_T pf_callback)
{
	/* Check input parameters */
	if (NULL == pf_callback)
	{
		return E_MCU_UART_RET_STATUS_INPUT_PARAM_ERR;
	}

	/* Check UART initialization status */
	if (E_MCU_UART_INIT_STATUS_OK != gs_mcu_uart_handle.is_inited)
	{
		return E_MCU_UART_RET_STATUS_INIT_STATUS_ERR;
	}

	/* Register transmit complete callback */
	gs_mcu_uart_handle.pf_transmit_complete_callback = pf_callback;

	return E_MCU_UART_RET_STATUS_OK;
}

extern E_MCU_UART_RET_STATUS_T mcu_uart_receive_dma_idle_enable(void)
{
	/* Check UART initialization status */
	if (E_MCU_UART_INIT_STATUS_OK != gs_mcu_uart_handle.is_inited)
	{
		return E_MCU_UART_RET_STATUS_INIT_STATUS_ERR;
	}

	/* Reset DMA buffer last size */
	gs_mcu_uart_handle.rx_dma_buf_last_size = 0;

	/* Enable UART reception */
	HAL_StatusTypeDef ret_status_hal = HAL_UARTEx_ReceiveToIdle_DMA(&(gs_mcu_uart_handle.uart_hal_handle), gs_mcu_uart_handle.p_rx_dma_buf, D_MCU_UART_RX_DMA_BUFFER_SIZE);
	if (HAL_OK != ret_status_hal)
	{
		(void)ret_status_hal;

		return E_MCU_UART_RET_STATUS_RESOURCE_ERR;
	}

	/* Update UART RX status */
	gs_mcu_uart_handle.rx_status = E_MCU_UART_RX_STATUS_BUSY;

	return E_MCU_UART_RET_STATUS_OK;
}

extern E_MCU_UART_RET_STATUS_T mcu_uart_receive_status_get(E_MCU_UART_RX_STATUS_T* const p_rx_status)
{
	/* Check input parameters */
	if (NULL == p_rx_status)
	{
		return E_MCU_UART_RET_STATUS_INPUT_PARAM_ERR;
	}

	/* Get UART RX status */
	*p_rx_status = gs_mcu_uart_handle.rx_status;

	return E_MCU_UART_RET_STATUS_OK;
}

extern E_MCU_UART_RET_STATUS_T mcu_uart_receive_complete_callback_register(PF_MCU_UART_RECEIVE_COMPLETE_CALLBACK_T pf_callback)
{
	/* Check input parameters */
	if (NULL == pf_callback)
	{
		return E_MCU_UART_RET_STATUS_INPUT_PARAM_ERR;
	}

	/* Check UART initialization status */
	if (E_MCU_UART_INIT_STATUS_OK != gs_mcu_uart_handle.is_inited)
	{
		return E_MCU_UART_RET_STATUS_INIT_STATUS_ERR;
	}

	/* Register receive complete callback */
	gs_mcu_uart_handle.pf_receive_complete_callback = pf_callback;

	return E_MCU_UART_RET_STATUS_OK;
}

extern E_MCU_UART_RET_STATUS_T mcu_uart_receive_process_callback_register(PF_MCU_UART_RECEIVE_PROCESS_CALLBACK_T pf_callback)
{
	/* Check input parameters */
	if (NULL == pf_callback)
	{
		return E_MCU_UART_RET_STATUS_INPUT_PARAM_ERR;
	}

	/* Check UART initialization status */
	if (E_MCU_UART_INIT_STATUS_OK != gs_mcu_uart_handle.is_inited)
	{
		return E_MCU_UART_RET_STATUS_INIT_STATUS_ERR;
	}

	/* Register receive process callback */
	gs_mcu_uart_handle.pf_receive_process_callback = pf_callback;

	return E_MCU_UART_RET_STATUS_OK;
}

extern void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    HAL_StatusTypeDef ret_status_hal = HAL_OK;
    GPIO_InitTypeDef GPIO_InitStruct = {0};

	if (gs_mcu_uart_handle.uart_hal_handle.Instance == huart->Instance)
	{
		/* UART clock enable */
		D_MCU_UART_CLK_ENABLE();
		D_MCU_UART_TX_GPIO_CLK_ENABLE();
        D_MCU_UART_RX_GPIO_CLK_ENABLE();

	    /**
	     * UART GPIO configuration
	     * PB6     ------> USART1_TX
	     * PB7     ------> USART1_RX
	     */
	    GPIO_InitStruct.Pin 		= D_MCU_UART_TX_GPIO_PIN;
	    GPIO_InitStruct.Mode 		= GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull 		= GPIO_PULLUP;
	    GPIO_InitStruct.Speed 		= GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate 	= D_MCU_UART_AF;
	    HAL_GPIO_Init(D_MCU_UART_TX_GPIO_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin 		= D_MCU_UART_RX_GPIO_PIN;
	    GPIO_InitStruct.Mode 		= GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull 		= GPIO_PULLUP;
	    GPIO_InitStruct.Speed 		= GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate 	= D_MCU_UART_AF;
	    HAL_GPIO_Init(D_MCU_UART_RX_GPIO_PORT, &GPIO_InitStruct);

		/* UART interrupt initialization */
	    HAL_NVIC_SetPriority(D_MCU_UART_IRQ_NUMBER, D_MCU_UART_IRQ_PRIORITY, D_MCU_UART_IRQ_SUBPRIORITY);
	    HAL_NVIC_EnableIRQ(D_MCU_UART_IRQ_NUMBER);

		/* DMA controller clock enable */
		D_MCU_UART_DMAMUX_CLK_ENABLE();
		D_MCU_UART_TX_DMA_CLK_ENABLE();
		D_MCU_UART_RX_DMA_CLK_ENABLE();

		/* UART DMA TX initialization */
		DMA_HandleTypeDef* dma_uart_tx_handle 			= 	&(gs_mcu_uart_handle.tx_dma_hal_handle);

		dma_uart_tx_handle->Instance 					=	D_MCU_UART_TX_DMA_INSTANCE;
		dma_uart_tx_handle->Init.Request 				=	D_MCU_UART_TX_DMA_REQUEST;
		dma_uart_tx_handle->Init.Direction 				=	DMA_MEMORY_TO_PERIPH;
		dma_uart_tx_handle->Init.PeriphInc 				=	DMA_PINC_DISABLE;
		dma_uart_tx_handle->Init.MemInc 				=	DMA_MINC_ENABLE;
		dma_uart_tx_handle->Init.PeriphDataAlignment 	=	DMA_PDATAALIGN_BYTE;
		dma_uart_tx_handle->Init.MemDataAlignment 		=	DMA_MDATAALIGN_BYTE;
		dma_uart_tx_handle->Init.Mode 					=	DMA_NORMAL;
		dma_uart_tx_handle->Init.Priority 				=	DMA_PRIORITY_LOW;
		
		ret_status_hal = HAL_DMA_Init(dma_uart_tx_handle);
		if (HAL_OK != ret_status_hal)
		{
			(void)ret_status_hal;
            
			// TBD: Error_Handler() 
            return;
		}

		__HAL_LINKDMA(huart, hdmatx, (*dma_uart_tx_handle) );

		/* UART DMA RX initialization */
		DMA_HandleTypeDef* dma_uart_rx_handle 			= 	&(gs_mcu_uart_handle.rx_dma_hal_handle);

		dma_uart_rx_handle->Instance 					=	D_MCU_UART_RX_DMA_INSTANCE;
		dma_uart_rx_handle->Init.Request 				=	D_MCU_UART_RX_DMA_REQUEST;
		dma_uart_rx_handle->Init.Direction 				=	DMA_PERIPH_TO_MEMORY;
		dma_uart_rx_handle->Init.PeriphInc 				=	DMA_PINC_DISABLE;
		dma_uart_rx_handle->Init.MemInc 				= 	DMA_MINC_ENABLE;
		dma_uart_rx_handle->Init.PeriphDataAlignment	= 	DMA_PDATAALIGN_BYTE;
		dma_uart_rx_handle->Init.MemDataAlignment 		= 	DMA_MDATAALIGN_BYTE;
		dma_uart_rx_handle->Init.Mode 					= 	DMA_CIRCULAR;
		dma_uart_rx_handle->Init.Priority 				= 	DMA_PRIORITY_HIGH;
		
		ret_status_hal = HAL_DMA_Init(dma_uart_rx_handle);
		if (HAL_OK != ret_status_hal)
		{
			(void)ret_status_hal;

			// TBD: Error_Handler() 
			return;
		}

		__HAL_LINKDMA(huart, hdmarx, (*dma_uart_rx_handle) );

		/* DMA TX interrupt initialization  */
  		HAL_NVIC_SetPriority(D_MCU_UART_TX_DMA_IRQ_NUMBER, D_MCU_UART_TX_DMA_IRQ_PRIORITY, D_MCU_UART_TX_DMA_IRQ_SUBPRIORITY);
  		HAL_NVIC_EnableIRQ(D_MCU_UART_TX_DMA_IRQ_NUMBER);

  		/* DMA RX interrupt initialization  */
 	 	HAL_NVIC_SetPriority(D_MCU_UART_RX_DMA_IRQ_NUMBER, D_MCU_UART_RX_DMA_IRQ_PRIORITY, D_MCU_UART_RX_DMA_IRQ_SUBPRIORITY);
  		HAL_NVIC_EnableIRQ(D_MCU_UART_RX_DMA_IRQ_NUMBER);
	}
}

extern void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    if (gs_mcu_uart_handle.uart_hal_handle.Instance == huart->Instance)
	{
		/* DMA TX interrupt disable */
		HAL_NVIC_DisableIRQ(D_MCU_UART_TX_DMA_IRQ_NUMBER);

		/* DMA RX interrupt disable */
		HAL_NVIC_DisableIRQ(D_MCU_UART_RX_DMA_IRQ_NUMBER);

		/* UART DMA deInit */
		HAL_DMA_DeInit(huart->hdmatx);
		HAL_DMA_DeInit(huart->hdmarx);

		/* Disable UART interrupt */
		HAL_NVIC_DisableIRQ(D_MCU_UART_IRQ_NUMBER);

	    /**
	     * UART1 GPIO configuration
	     * PB6     ------> UART1_TX
	     * PB7     ------> UART1_RX
	     */
	    HAL_GPIO_DeInit(D_MCU_UART_TX_GPIO_PORT, D_MCU_UART_TX_GPIO_PIN);
	    HAL_GPIO_DeInit(D_MCU_UART_RX_GPIO_PORT, D_MCU_UART_RX_GPIO_PIN);

		/* Reset UART */
		D_MCU_UART_FORCE_RESET();
		D_MCU_UART_RELEASE_RESET();

		/* UART clock disable */
		D_MCU_UART_CLK_DISABLE();
	}
}

extern void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (gs_mcu_uart_handle.uart_hal_handle.Instance == huart->Instance)
	{
		/* Update UART TX status */
		/* This operation should be placed before calling transmit complete callback to avoid transmit failure in callback function */
		if (E_MCU_UART_TX_STATUS_READY != gs_mcu_uart_handle.tx_status)
		{
			gs_mcu_uart_handle.tx_status = E_MCU_UART_TX_STATUS_READY;
		}

		/* Call transmit complete callback */
		if (NULL != gs_mcu_uart_handle.pf_transmit_complete_callback)
		{
			gs_mcu_uart_handle.pf_transmit_complete_callback();
		}
	}
}

extern void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{	
	UNUSED(huart);
}

extern void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size)
{
	if (gs_mcu_uart_handle.uart_hal_handle.Instance == huart->Instance)
	{
		/* Last received data length in DMA buffer */
        uint16_t 	dma_buf_last_size	=	gs_mcu_uart_handle.rx_dma_buf_last_size;
		/* Current received data length in DMA buffer */
		uint16_t 	dma_buf_curr_size	=	Size;
		/* Data length to be processed */
		uint16_t	proc_char_size		=	0;

		/* Check if number of received data in reception buffer has changed */
		if (dma_buf_last_size != dma_buf_curr_size)
		{
			if (dma_buf_last_size < dma_buf_curr_size)
			{
				/* Continue getting data from the DMA buffer */
				proc_char_size = dma_buf_curr_size - dma_buf_last_size;
				if (NULL != gs_mcu_uart_handle.pf_receive_process_callback)
				{
					gs_mcu_uart_handle.pf_receive_process_callback(&(gs_mcu_uart_handle.p_rx_dma_buf[dma_buf_last_size]), proc_char_size);
				}
			}
			else
			{
				/* Restart getting data from the DMA buffer */
				proc_char_size = dma_buf_curr_size;
				if (NULL != gs_mcu_uart_handle.pf_receive_process_callback)
				{
					gs_mcu_uart_handle.pf_receive_process_callback(&(gs_mcu_uart_handle.p_rx_dma_buf[0]), proc_char_size);
				}
			}

			/* Update last received data length in DMA buffer */
			gs_mcu_uart_handle.rx_dma_buf_last_size = dma_buf_curr_size;

			/* Call receive complete callback */
			if (NULL != gs_mcu_uart_handle.pf_receive_complete_callback)
			{
				gs_mcu_uart_handle.pf_receive_complete_callback();
			}
		}
	}
}

extern void DMA1_Channel1_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&(gs_mcu_uart_handle.tx_dma_hal_handle) );
}

extern void DMA1_Channel2_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&(gs_mcu_uart_handle.rx_dma_hal_handle) );
}

extern void USART1_IRQHandler()
{
	HAL_UART_IRQHandler(&(gs_mcu_uart_handle.uart_hal_handle) );
}
