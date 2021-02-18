/**
  ******************************************************************************
  * @file    ksz8091.h
  * @author  André Muller Cascadan
  * @brief   Driver for KSZ8091
  *
  ******************************************************************************
  */ 


#ifndef KSZ8091_H
#define KSZ8091_H


#include <stdint.h>



/**
  * KSZ8091 Registers Mapping
  */
#define KSZ8091_BCR      ((uint16_t)0x0000U)
#define KSZ8091_BSR      ((uint16_t)0x0001U)
#define KSZ8091_PHYI1R   ((uint16_t)0x0002U)
#define KSZ8091_PHYI2R   ((uint16_t)0x0003U)
#define KSZ8091_ANAR     ((uint16_t)0x0004U)
#define KSZ8091_ANLPAR   ((uint16_t)0x0005U)
#define KSZ8091_ANER     ((uint16_t)0x0006U)
#define KSZ8091_ANNPTR   ((uint16_t)0x0007U)
#define KSZ8091_ANNPRR   ((uint16_t)0x0008U)
#define KSZ8091_PHYCTRL1 ((uint16_t)0x001EU)



/**
  * KSZ8091 BCR Bit Definition
  */
#define KSZ8091_BCR_SOFT_RESET         ((uint16_t)0x8000U)
#define KSZ8091_BCR_LOOPBACK           ((uint16_t)0x4000U)
#define KSZ8091_BCR_SPEED_SELECT       ((uint16_t)0x2000U)
#define KSZ8091_BCR_AUTONEGO_EN        ((uint16_t)0x1000U)
#define KSZ8091_BCR_POWER_DOWN         ((uint16_t)0x0800U)
#define KSZ8091_BCR_ISOLATE            ((uint16_t)0x0400U)
#define KSZ8091_BCR_RESTART_AUTONEGO   ((uint16_t)0x0200U)
#define KSZ8091_BCR_DUPLEX_MODE        ((uint16_t)0x0100U) 



/**
  * KSZ8091 BSR Bit Definition
  */   
#define KSZ8091_BSR_100BASE_T4       ((uint16_t)0x8000U)
#define KSZ8091_BSR_100BASE_TX_FD    ((uint16_t)0x4000U)
#define KSZ8091_BSR_100BASE_TX_HD    ((uint16_t)0x2000U)
#define KSZ8091_BSR_10BASE_T_FD      ((uint16_t)0x1000U)
#define KSZ8091_BSR_10BASE_T_HD      ((uint16_t)0x0800U)
#define KSZ8091_BSR_100BASE_T2_FD    ((uint16_t)0x0400U)
#define KSZ8091_BSR_100BASE_T2_HD    ((uint16_t)0x0200U)
#define KSZ8091_BSR_EXTENDED_STATUS  ((uint16_t)0x0100U)
#define KSZ8091_BSR_AUTONEGO_CPLT    ((uint16_t)0x0020U)
#define KSZ8091_BSR_REMOTE_FAULT     ((uint16_t)0x0010U)
#define KSZ8091_BSR_AUTONEGO_ABILITY ((uint16_t)0x0008U)
#define KSZ8091_BSR_LINK_STATUS      ((uint16_t)0x0004U)
#define KSZ8091_BSR_JABBER_DETECT    ((uint16_t)0x0002U)
#define KSZ8091_BSR_EXTENDED_CAP     ((uint16_t)0x0001U)



/**
  * KSZ8091 PHYI1R Bit Definition
  */
#define KSZ8091_PHYI1R_OUI_3_18           ((uint16_t)0xFFFFU)



/** 
  * KSZ8091 PHYI2R Bit Definition
  */
#define KSZ8091_PHYI2R_OUI_19_24          ((uint16_t)0xFC00U)
#define KSZ8091_PHYI2R_MODEL_NBR          ((uint16_t)0x03F0U)
#define KSZ8091_PHYI2R_REVISION_NBR       ((uint16_t)0x000FU)



/**
  * KSZ8091 ANAR Bit Definition
  */
#define KSZ8091_ANAR_NEXT_PAGE               ((uint16_t)0x8000U)
#define KSZ8091_ANAR_REMOTE_FAULT            ((uint16_t)0x2000U)
#define KSZ8091_ANAR_PAUSE_OPERATION         ((uint16_t)0x0C00U)
#define KSZ8091_ANAR_PO_NOPAUSE              ((uint16_t)0x0000U)
#define KSZ8091_ANAR_PO_SYMMETRIC_PAUSE      ((uint16_t)0x0400U)
#define KSZ8091_ANAR_PO_ASYMMETRIC_PAUSE     ((uint16_t)0x0800U)
#define KSZ8091_ANAR_PO_ADVERTISE_SUPPORT    ((uint16_t)0x0C00U)
#define KSZ8091_ANAR_100BASE_TX_FD           ((uint16_t)0x0100U)
#define KSZ8091_ANAR_100BASE_TX              ((uint16_t)0x0080U)
#define KSZ8091_ANAR_10BASE_T_FD             ((uint16_t)0x0040U)
#define KSZ8091_ANAR_10BASE_T                ((uint16_t)0x0020U)
#define KSZ8091_ANAR_SELECTOR_FIELD          ((uint16_t)0x000FU)



/**
  * KSZ8091 ANLPAR Bit Definition
  */
#define KSZ8091_ANLPAR_NEXT_PAGE            ((uint16_t)0x8000U)
#define KSZ8091_ANLPAR_REMOTE_FAULT         ((uint16_t)0x2000U)
#define KSZ8091_ANLPAR_PAUSE_OPERATION      ((uint16_t)0x0C00U)
#define KSZ8091_ANLPAR_PO_NOPAUSE           ((uint16_t)0x0000U)
#define KSZ8091_ANLPAR_PO_SYMMETRIC_PAUSE   ((uint16_t)0x0400U)
#define KSZ8091_ANLPAR_PO_ASYMMETRIC_PAUSE  ((uint16_t)0x0800U)
#define KSZ8091_ANLPAR_PO_ADVERTISE_SUPPORT ((uint16_t)0x0C00U)
#define KSZ8091_ANLPAR_100BASE_TX_FD        ((uint16_t)0x0100U)
#define KSZ8091_ANLPAR_100BASE_TX           ((uint16_t)0x0080U)
#define KSZ8091_ANLPAR_10BASE_T_FD          ((uint16_t)0x0040U)
#define KSZ8091_ANLPAR_10BASE_T             ((uint16_t)0x0020U)
#define KSZ8091_ANLPAR_SELECTOR_FIELD       ((uint16_t)0x000FU)



/**
  * KSZ8091 ANER Bit Definition
  */
#define KSZ8091_ANER_RX_NP_LOCATION_ABLE    ((uint16_t)0x0040U)
#define KSZ8091_ANER_RX_NP_STORAGE_LOCATION ((uint16_t)0x0020U)
#define KSZ8091_ANER_PARALLEL_DETECT_FAULT  ((uint16_t)0x0010U)
#define KSZ8091_ANER_LP_NP_ABLE             ((uint16_t)0x0008U)
#define KSZ8091_ANER_NP_ABLE                ((uint16_t)0x0004U)
#define KSZ8091_ANER_PAGE_RECEIVED          ((uint16_t)0x0002U)
#define KSZ8091_ANER_LP_AUTONEG_ABLE        ((uint16_t)0x0001U)



/**
  * KSZ8091 ANNPTR Bit Definition
  */
#define KSZ8091_ANNPTR_NEXT_PAGE         ((uint16_t)0x8000U)
#define KSZ8091_ANNPTR_MESSAGE_PAGE      ((uint16_t)0x2000U)
#define KSZ8091_ANNPTR_ACK2              ((uint16_t)0x1000U)
#define KSZ8091_ANNPTR_TOGGLE            ((uint16_t)0x0800U)
#define KSZ8091_ANNPTR_MESSAGGE_CODE     ((uint16_t)0x07FFU)



/**
  * KSZ8091 ANNPRR Bit Definition
  */
#define KSZ8091_ANNPTR_NEXT_PAGE         ((uint16_t)0x8000U)
#define KSZ8091_ANNPRR_ACK               ((uint16_t)0x4000U)
#define KSZ8091_ANNPRR_MESSAGE_PAGE      ((uint16_t)0x2000U)
#define KSZ8091_ANNPRR_ACK2              ((uint16_t)0x1000U)
#define KSZ8091_ANNPRR_TOGGLE            ((uint16_t)0x0800U)
#define KSZ8091_ANNPRR_MESSAGGE_CODE     ((uint16_t)0x07FFU)



/**
  * KSZ8091 PHYCTRL1 Bit Definition
  */
#define KSZ8091_PHYCTRL1_OPERM_MASK          ((uint16_t)0x0007U)
#define KSZ8091_PHYCTRL1_OPERM_10BT_HD       ((uint16_t)0x0001U)
#define KSZ8091_PHYCTRL1_OPERM_100BTX_HD     ((uint16_t)0x0002U)
#define KSZ8091_PHYCTRL1_OPERM_10BT_FD       ((uint16_t)0x0005U)
#define KSZ8091_PHYCTRL1_OPERM_100BTX_FD     ((uint16_t)0x0006U)



/**
  * KSZ8091 Status
  */
#define  KSZ8091_STATUS_READ_ERROR            ((int32_t)-5)
#define  KSZ8091_STATUS_WRITE_ERROR           ((int32_t)-4)
#define  KSZ8091_STATUS_ADDRESS_ERROR         ((int32_t)-3)
#define  KSZ8091_STATUS_RESET_TIMEOUT         ((int32_t)-2)
#define  KSZ8091_STATUS_ERROR                 ((int32_t)-1)
#define  KSZ8091_STATUS_OK                    ((int32_t) 0)
#define  KSZ8091_STATUS_LINK_DOWN             ((int32_t) 1)
#define  KSZ8091_STATUS_100MBITS_FULLDUPLEX   ((int32_t) 2)
#define  KSZ8091_STATUS_100MBITS_HALFDUPLEX   ((int32_t) 3)
#define  KSZ8091_STATUS_10MBITS_FULLDUPLEX    ((int32_t) 4)
#define  KSZ8091_STATUS_10MBITS_HALFDUPLEX    ((int32_t) 5)
#define  KSZ8091_STATUS_AUTONEGO_NOTDONE      ((int32_t) 6)




/* Exported types ------------------------------------------------------------*/ 


/**
  * KSZ8091 Exported Types
  */
typedef int32_t  (*ksz8091_Init_Func) (void); 
typedef int32_t  (*ksz8091_DeInit_Func) (void);
typedef int32_t  (*ksz8091_ReadReg_Func) (uint32_t, uint32_t, uint32_t *);
typedef int32_t  (*ksz8091_WriteReg_Func) (uint32_t, uint32_t, uint32_t);
typedef int32_t  (*ksz8091_GetTick_Func) (void);

typedef struct 
{
  ksz8091_Init_Func      Init;
  ksz8091_DeInit_Func    DeInit;
  ksz8091_WriteReg_Func  WriteReg;
  ksz8091_ReadReg_Func   ReadReg;
  ksz8091_GetTick_Func   GetTick;
} ksz8091_IOCtx_t;

  
typedef struct 
{
  uint32_t            DevAddr;
  uint32_t            Is_Initialized;
  ksz8091_IOCtx_t     IO;
  void               *pData;
}ksz8091_Object_t;



/* Exported functions --------------------------------------------------------*/


/**
  * KSZ8091 Exported Functions
  * 
  * Commented headers refer to not implemented functions.
  * 
  */
int32_t KSZ8091_RegisterBusIO(ksz8091_Object_t *pObj, ksz8091_IOCtx_t *ioctx);
int32_t KSZ8091_Init(ksz8091_Object_t *pObj);
//int32_t KSZ8091_DeInit(ksz8091_Object_t *pObj);
//int32_t KSZ8091_DisablePowerDownMode(ksz8091_Object_t *pObj);
//int32_t KSZ8091_EnablePowerDownMode(ksz8091_Object_t *pObj);
//int32_t KSZ8091_StartAutoNego(ksz8091_Object_t *pObj);
int32_t KSZ8091_GetLinkState(ksz8091_Object_t *pObj);
//int32_t KSZ8091_SetLinkState(ksz8091_Object_t *pObj, uint32_t LinkState);
//int32_t KSZ8091_EnableLoopbackMode(ksz8091_Object_t *pObj);
//int32_t KSZ8091_DisableLoopbackMode(ksz8091_Object_t *pObj);
//int32_t KSZ8091_EnableIT(ksz8091_Object_t *pObj, uint32_t Interrupt);
//int32_t KSZ8091_DisableIT(ksz8091_Object_t *pObj, uint32_t Interrupt);
//int32_t KSZ8091_ClearIT(ksz8091_Object_t *pObj, uint32_t Interrupt);
//int32_t KSZ8091_GetITStatus(ksz8091_Object_t *pObj, uint32_t Interrupt);


#endif /* KSZ8091_H */






