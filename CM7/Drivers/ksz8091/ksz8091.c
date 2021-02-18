/** 
  ******************************************************************************
  * @file    ksz8091.c
  * @author  André Muller Cascadan
  * @brief   Driver for KSZ8091
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ksz8091.h"



/**
  * KSZ8091 Private Defines
  */
#define KSZ8091_SW_RESET_TO    ((uint32_t)500U)
#define KSZ8091_INIT_TO        ((uint32_t)2000U)



/**
  * Register IO functions to component object
  * 
  *  pObj: device object  of ksz8091_Object_t.
  * ioctx: holds device IO functions.
  * 
  * return: KSZ8091_STATUS_OK  if OK
  *         KSZ8091_STATUS_ERROR if missing mandatory function
  */
int32_t  KSZ8091_RegisterBusIO(ksz8091_Object_t *pObj, ksz8091_IOCtx_t *ioctx)
{
  if(!pObj || !ioctx->ReadReg || !ioctx->WriteReg || !ioctx->GetTick)
  {
    return KSZ8091_STATUS_ERROR;
  }
  
  pObj->IO.Init = ioctx->Init;
  pObj->IO.DeInit = ioctx->DeInit;
  pObj->IO.ReadReg = ioctx->ReadReg;
  pObj->IO.WriteReg = ioctx->WriteReg;
  pObj->IO.GetTick = ioctx->GetTick;
  
  return KSZ8091_STATUS_OK;
}



/**
  * Initializes the KSZ8091 and configure the needed hardware resources
  * 
  * pObj: device object ksz8091_Object_t. 
  * 
  * return: KSZ8091_STATUS_OK  if OK
  *         KSZ8091_STATUS_ADDRESS_ERROR if cannot find device address
  *         KSZ8091_STATUS_READ_ERROR if connot read register
  *         KSZ8091_STATUS_WRITE_ERROR if connot write to register
  *         KSZ8091_STATUS_RESET_TIMEOUT if cannot perform a software reset
  */
int32_t KSZ8091_Init(ksz8091_Object_t *pObj)
{
  uint32_t tickstart = 0, regvalue = 0, addr = 0;
  int32_t status = KSZ8091_STATUS_OK;
  
  if(pObj->Is_Initialized == 0)
  {
    if(pObj->IO.Init != 0)
    {
      /* GPIO and Clocks initialization */
      pObj->IO.Init();
    }
    
    /* Uses the broadcast device address 0 */
    pObj->DevAddr = 0; 
    
    /* Check if the device is responsive */
    if(pObj->IO.ReadReg(addr, KSZ8091_BCR, &regvalue) < 0)
    { 
      return KSZ8091_STATUS_READ_ERROR;
    }
     
    
    /* set a software reset  */
    if(pObj->IO.WriteReg(pObj->DevAddr, KSZ8091_BCR, KSZ8091_BCR_SOFT_RESET) < 0)
    {
      return KSZ8091_STATUS_WRITE_ERROR;
    }
    
    
    /* Wait for reset or timeout */
    tickstart = pObj->IO.GetTick();
    do
    {
      if(pObj->IO.ReadReg(pObj->DevAddr, KSZ8091_BCR, &regvalue) < 0)
      {
        return KSZ8091_STATUS_READ_ERROR;
      }
    }
    while( (regvalue & KSZ8091_BCR_SOFT_RESET) && ((pObj->IO.GetTick() - tickstart) <= KSZ8091_SW_RESET_TO) );
    
    if ( regvalue & KSZ8091_BCR_SOFT_RESET )
    {
      return KSZ8091_STATUS_RESET_TIMEOUT;
    }
    
    
    /* Apply a delay of KSZ8091_INIT_TO */
    tickstart = pObj->IO.GetTick();
    while((pObj->IO.GetTick() - tickstart) <= KSZ8091_INIT_TO)
    {
      asm("nop");
    }
    
    
    pObj->Is_Initialized = 1;
  }
   
  return KSZ8091_STATUS_OK;
}



/**
  * Get the link state of KSZ8091 device.
  * 
  * pObj: Pointer to device object.
  * 
  * return: KSZ8091_STATUS_LINK_DOWN  if link is down
  *         KSZ8091_STATUS_AUTONEGO_NOTDONE if Auto nego not completed 
  *         KSZ8091_STATUS_100MBITS_FULLDUPLEX if 100Mb/s FD
  *         KSZ8091_STATUS_100MBITS_HALFDUPLEX if 100Mb/s HD
  *         KSZ8091_STATUS_10MBITS_FULLDUPLEX  if 10Mb/s FD
  *         KSZ8091_STATUS_10MBITS_HALFDUPLEX  if 10Mb/s HD
  *         KSZ8091_STATUS_READ_ERROR if connot read register
  *         KSZ8091_STATUS_WRITE_ERROR if connot write to register
  */
int32_t KSZ8091_GetLinkState(ksz8091_Object_t *pObj)
{
  uint32_t readval = 0;
  
  /* Read Status register  */
  if(pObj->IO.ReadReg(pObj->DevAddr, KSZ8091_BSR, &readval) < 0)
  {
    return KSZ8091_STATUS_READ_ERROR;
  }
  
  /* Read Status register again */
  if(pObj->IO.ReadReg(pObj->DevAddr, KSZ8091_BSR, &readval) < 0)
  {
    return KSZ8091_STATUS_READ_ERROR;
  }
  
  if((readval & KSZ8091_BSR_LINK_STATUS) == 0)
  {
    /* Return Link Down status */
    return KSZ8091_STATUS_LINK_DOWN;
  }
  
  /* Check Auto negotiaition */
  if(pObj->IO.ReadReg(pObj->DevAddr, KSZ8091_BCR, &readval) < 0)
  {
    return KSZ8091_STATUS_READ_ERROR;
  }
  
  if((readval & KSZ8091_BCR_AUTONEGO_EN) != KSZ8091_BCR_AUTONEGO_EN)
  {
    if(((readval & KSZ8091_BCR_SPEED_SELECT) == KSZ8091_BCR_SPEED_SELECT) && ((readval & KSZ8091_BCR_DUPLEX_MODE) == KSZ8091_BCR_DUPLEX_MODE)) 
    {
      return KSZ8091_STATUS_100MBITS_FULLDUPLEX;
    }
    else if ((readval & KSZ8091_BCR_SPEED_SELECT) == KSZ8091_BCR_SPEED_SELECT)
    {
      return KSZ8091_STATUS_100MBITS_HALFDUPLEX;
    }        
    else if ((readval & KSZ8091_BCR_DUPLEX_MODE) == KSZ8091_BCR_DUPLEX_MODE)
    {
      return KSZ8091_STATUS_10MBITS_FULLDUPLEX;
    }
    else
    {
      return KSZ8091_STATUS_10MBITS_HALFDUPLEX;
    }  		
  }
  else /* Auto Nego enabled */
  {
    if(pObj->IO.ReadReg(pObj->DevAddr, KSZ8091_PHYCTRL1, &readval) < 0)
    {
      return KSZ8091_STATUS_READ_ERROR;
    }
    
    /* Check if auto nego not done */
    if((readval & KSZ8091_PHYCTRL1_OPERM_MASK) == 0)
    {
      return KSZ8091_STATUS_AUTONEGO_NOTDONE;
    }
    
    if((readval & KSZ8091_PHYCTRL1_OPERM_MASK) == KSZ8091_PHYCTRL1_OPERM_100BTX_FD)
    {
      return KSZ8091_STATUS_100MBITS_FULLDUPLEX;
    }
    else if ((readval & KSZ8091_PHYCTRL1_OPERM_MASK) == KSZ8091_PHYCTRL1_OPERM_100BTX_HD)
    {
      return KSZ8091_STATUS_100MBITS_HALFDUPLEX;
    }
    else if ((readval & KSZ8091_PHYCTRL1_OPERM_MASK) == KSZ8091_PHYCTRL1_OPERM_10BT_FD)
    {
      return KSZ8091_STATUS_10MBITS_FULLDUPLEX;
    }
    else
    {
      return KSZ8091_STATUS_10MBITS_HALFDUPLEX;
    }
  }
}











