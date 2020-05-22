/**
  ******************************************************************************
  * @file    bl606_partition.c
  * @version V1.2
  * @date
  * @brief   This file is the standard driver c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2018 Bouffalo Lab</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of Bouffalo Lab nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include <string.h>
#include "bl_boot2.h"
#include "bl60x_xip_sflash.h"
#include "softcrc.h"

/** @addtogroup  BL606_Common_Driver
 *  @{
 */

/** @addtogroup  PARTITION
 *  @{
 */

/** @defgroup  PARTITION_Private_Macros
 *  @{
 */
#ifdef CPU_0
#define PtTable_Flash_Read(c,a,d,s) XIP_SFlash_Read_Via_Cache(a+BL606_CPU0_XIP_BASE+8*1024*1024,d,s)
#endif
#ifdef CPU_1
#define PtTable_Flash_Read(c,a,d,s) XIP_SFlash_Read_Via_Cache(a+BL606_CPU1_XIP_BASE+8*1024*1024,d,s)
#endif

/*@} end of group PARTITION_Private_Macros */

/** @defgroup  PARTITION_Private_Types
 *  @{
 */

/*@} end of group PARTITION_Private_Types */

/** @defgroup  PARTITION_Private_Variables
 *  @{
 */
pPtTable_Flash_Erase PtTable_Flash_Erase=NULL;
pPtTable_Flash_Write PtTable_Flash_Write=NULL;

/*@} end of group PARTITION_Private_Variables */

/** @defgroup  PARTITION_Global_Variables
 *  @{
 */

/*@} end of group PARTITION_Global_Variables */

/** @defgroup  PARTITION_Private_Fun_Declaration
 *  @{
 */

/*@} end of group PARTITION_Private_Fun_Declaration */

/** @defgroup  PARTITION_Private_Functions
 *  @{
 */

#if 0
/****************************************************************************//**
 * @brief  Judge partition table valid
 *
 * @param  ptStuff: Partition table stuff pointer
 *
 * @return 0 for invalid and 1 for valid
 *
*******************************************************************************/
static uint8_t PtTable_Valid(PtTable_Stuff_Config *ptStuff)
{
    PtTable_Config *ptTable=&ptStuff->ptTable;
    PtTable_Entry_Config *ptEntries=ptStuff->ptEntries;
    uint32_t *pCrc32;
    uint32_t entriesLen=sizeof(PtTable_Entry_Config)*ptTable->entryCnt;
    
    if(ptTable->magicCode==BFLB_PT_MAGIC_CODE){
        if (ptTable->entryCnt > PT_ENTRY_MAX) {
            //MSG_ERR("PT Entry Count Error\r\n");
            return 0;
        }
        if(ptTable->crc32!=
            BFLB_Soft_CRC32((uint8_t*)ptTable,sizeof(PtTable_Config)-4)){
            //MSG_ERR("PT CRC Error\r\n");
            return 0;
        }
        pCrc32=(uint32_t *)((uint32_t)ptEntries+entriesLen);
        if(*pCrc32!=BFLB_Soft_CRC32((uint8_t*)ptEntries,entriesLen)){
            //MSG_ERR("PT Entry CRC Error\r\n");
            return 0;
        }
        return 1;
    }
    return 0;
}
#endif

/*@} end of group PARTITION_Private_Functions */

/** @defgroup  PARTITION_Public_Functions
 *  @{
 */

/****************************************************************************//**
 * @brief  Register partition flash read write erase fucntion
 *
 * @param  erase: Flash erase function
 * @param  write: Flash read function
 *
 * @return None
 *
*******************************************************************************/
void PtTable_Set_Flash_Operation(pPtTable_Flash_Erase erase,pPtTable_Flash_Write write)
{
    PtTable_Flash_Erase=erase;
    PtTable_Flash_Write=write;
    //PtTable_Flash_Read=read;
}

#if 0
/****************************************************************************//**
 * @brief  Get active partition table whole stuff
 *
 * @param  pFlashCfg: Flash config pointer
 * @param  ptStuff[2]: Partition table stuff pointer
 *
 * @return Active partition table ID
 *
*******************************************************************************/
PtTable_ID_Type PtTable_Get_Active_Partition(const SPI_Flash_Cfg_Type *pFlashCfg,
                                            PtTable_Stuff_Config ptStuff[2])
{
    uint32_t ptValid[2]={0,0};
    PtTable_ID_Type activePtID;

    if(ptStuff==NULL){
        return PT_TABLE_ID_INVALID;
    }
    activePtID=PT_TABLE_ID_INVALID;
    
    PtTable_Flash_Read(pFlashCfg,BFLB_PT_TABLE0_ADDRESS,(uint8_t *)&ptStuff[0],sizeof(PtTable_Stuff_Config));
    ptValid[0]=PtTable_Valid(&ptStuff[0]);
    
    PtTable_Flash_Read(pFlashCfg,BFLB_PT_TABLE1_ADDRESS,(uint8_t *)&ptStuff[1],sizeof(PtTable_Stuff_Config));
    ptValid[1]=PtTable_Valid(&ptStuff[1]);
    
    if(ptValid[0]==1 && ptValid[1]==1){
        if(ptStuff[0].ptTable.age>=ptStuff[1].ptTable.age){
            activePtID=PT_TABLE_ID_0;
        }else{
            activePtID=PT_TABLE_ID_1;
        }
    }else if(ptValid[0]==1){
        activePtID=PT_TABLE_ID_0;
    }else if(ptValid[1]==1){
        activePtID=PT_TABLE_ID_1;
    }
    return activePtID;
}
#endif

/****************************************************************************//**
 * @brief  Get partition entry
 *
 * @param  ptStuff: Partition table stuff pointer
 * @param  type: Type of partition entry
 * @param  ptEntry: Partition entry pointer to store read data
 *
 * @return PT_ERROR_SUCCESS or PT_ERROR_ENTRY_NOT_FOUND or PT_ERROR_PARAMETER
 *
*******************************************************************************/
PtTable_Error_Type PtTable_Get_Active_Entries(PtTable_Stuff_Config *ptStuff,
                                            PtTable_Entry_Type type,
                                            PtTable_Entry_Config *ptEntry)
{
    uint32_t i=0;
    
    if(ptStuff==NULL||ptEntry==NULL){
        return PT_ERROR_PARAMETER;
    }
    for (i=0; i < ptStuff->ptTable.entryCnt; i++) {
        if (ptStuff->ptEntries[i].type == type){
            memcpy(ptEntry,&ptStuff->ptEntries[i],sizeof(PtTable_Entry_Config));
            return PT_ERROR_SUCCESS;
        }
    }
    return PT_ERROR_ENTRY_NOT_FOUND;
}

/****************************************************************************//**
 * @brief  Update partition entry
 *
 * @param  pFlashCfg: Flash config pointer
 * @param  targetTableID: Target partition table to update
 * @param  ptStuff: Partition table stuff pointer
 * @param  ptEntry: Partition entry pointer to update
 *
 * @return Partition update result
 *
*******************************************************************************/
PtTable_Error_Type PtTable_Update_Entry(const SPI_Flash_Cfg_Type *pFlashCfg,
                                                PtTable_ID_Type targetTableID,
                                                PtTable_Stuff_Config *ptStuff,
                                                PtTable_Entry_Config *ptEntry)
{
    uint32_t i=0;
    BL_Err_Type ret;
    uint32_t writeAddr;
    uint32_t entriesLen;
    PtTable_Config *ptTable;
    PtTable_Entry_Config *ptEntries;
    uint32_t *pCrc32;

    if(ptEntry==NULL||ptStuff==NULL){
        return PT_ERROR_PARAMETER;
    }
    
    ptTable=&ptStuff->ptTable;
    ptEntries=ptStuff->ptEntries;
    
    if(targetTableID==PT_TABLE_ID_INVALID){
        return PT_ERROR_TABLE_NOT_VALID;
    }
    
    if(targetTableID==PT_TABLE_ID_0){
        writeAddr=BFLB_PT_TABLE0_ADDRESS;
    }else{
        writeAddr=BFLB_PT_TABLE1_ADDRESS;
    }
    for (i=0; i < ptTable->entryCnt; i++) {
        if (ptEntries[i].type == ptEntry->type){
            memcpy(&ptEntries[i],ptEntry,sizeof(PtTable_Entry_Config));
            break;
        }
    }
    if(i==ptTable->entryCnt){
        /* Not found this entry ,add new one */
        if(ptTable->entryCnt<PT_ENTRY_MAX){
            memcpy(&ptEntries[ptTable->entryCnt],ptEntry,sizeof(PtTable_Entry_Config));
            ptTable->entryCnt++;
        }else{
            return PT_ERROR_ENTRY_UPDATE_FAIL;
        }
    }
    
    /* Prepare write back to flash */
    /* Update age */
    ptTable->age++;
    ptTable->crc32=BFLB_Soft_CRC32((uint8_t*)ptTable,sizeof(PtTable_Config)-4);
    
    /* Update entries CRC */
    entriesLen=ptTable->entryCnt*sizeof(PtTable_Entry_Config);
    pCrc32=(uint32_t *)((uint32_t)ptEntries+entriesLen);
    *pCrc32=BFLB_Soft_CRC32((uint8_t *)&ptEntries[0],entriesLen);
    
    /* Write back to flash */
    /* Erase flash first */
    ret=PtTable_Flash_Erase(writeAddr,sizeof(PtTable_Config)+entriesLen+4);
    if(ret!=SUCCESS){
        //MSG_ERR("Flash Erase error\r\n");
        return PT_ERROR_FALSH_WRITE;
    }
    /* Write flash */
    ret=PtTable_Flash_Write(writeAddr,(uint8_t *)ptStuff,sizeof(PtTable_Stuff_Config));
    if(ret!=SUCCESS){
        //MSG_ERR("Flash Write error\r\n");
        return PT_ERROR_FALSH_WRITE;
    }

    return PT_ERROR_SUCCESS;
}

/****************************************************************************//**
 * @brief  Create partition entry
 *
 * @param  pFlashCfg: Flash config pointer
 * @param  ptID: Partition table ID
 *
 * @return Partition create result
 *
*******************************************************************************/
PtTable_Error_Type PtTable_Create(const SPI_Flash_Cfg_Type *pFlashCfg,PtTable_ID_Type ptID)
{
    uint32_t writeAddr;
    BL_Err_Type ret;
    PtTable_Config ptTable;
    
    if(ptID==PT_TABLE_ID_INVALID){
        return PT_ERROR_TABLE_NOT_VALID;
    }
    
    if(ptID==PT_TABLE_ID_0){
        writeAddr=BFLB_PT_TABLE0_ADDRESS;
    }else{
        writeAddr=BFLB_PT_TABLE1_ADDRESS;
    }
    
    /* Prepare write back to flash */
    ptTable.magicCode=BFLB_PT_MAGIC_CODE;
    ptTable.version=0;
    ptTable.entryCnt=0;
    ptTable.age=0;
    ptTable.crc32=BFLB_Soft_CRC32((uint8_t*)&ptTable,sizeof(PtTable_Config)-4);
    /* Write back to flash */
    ret=PtTable_Flash_Erase(writeAddr,sizeof(PtTable_Config)); 
    if(ret!=SUCCESS){
        //MSG_ERR("Flash Erase error\r\n");
        return PT_ERROR_FALSH_ERASE;
    }
    ret=PtTable_Flash_Write(writeAddr,(uint8_t *)&ptTable,sizeof(PtTable_Config));
    if(ret!=SUCCESS){
        //MSG_ERR("Flash Write error\r\n");
        return PT_ERROR_FALSH_WRITE;
    }
    return PT_ERROR_SUCCESS;
}

/*@} end of group PARTITION_Public_Functions */

/*@} end of group PARTITION */

/*@} end of group BL606_Common_Driver */
