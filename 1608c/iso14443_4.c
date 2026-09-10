/**
 * =============================================================================
 * 
 *
 * File   : iso14443_4.c
 *
 * Author : Lihongxu     
 * 
 * Date   : 2014-12-03
 *
 * Description:
 *      implement the half duplex communication protocol with ISO14443-4
 *
 * History:
 *
 * =============================================================================
 */

#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "iso14443_4.h"
#include "mh523.h"
#include "stdio.h"




#define DEBUG_14443_4	0 //

#if (DEBUG_14443_4)
	#define __printf(x) printf(x)
#else
	#define __printf(x) 
#endif


//#define FWI_DEFAULT	4	//
#define FWI_DEFAULT	4	//


pcd_info_s  g_pcd_module_info; /*the global variable in contactless*/



void pcd_default_info(void)// COS_TEST
{
	memset(&g_pcd_module_info, 0, sizeof(g_pcd_module_info));
	g_pcd_module_info.uiFsc = 32;
	g_pcd_module_info.uiFwi = FWI_DEFAULT;
	g_pcd_module_info.uiSfgi = 0;
	g_pcd_module_info.ucNadEn = 0;
	g_pcd_module_info.ucCidEn = 0;
	g_pcd_module_info.ucWTXM = 1; //multi
		
}



/**
 * implement the half duplex communication protocol with ISO14443-4
 * 
 * parameters:
 *             ppcd_info  : PCD information structure pointer
 *             psrc     : the datas information will be transmitted by ppcd_info
 *             i_tx_num     : the number of transmitted datas by ppcd_info
 *             pdes     : the datas information will be transmitted by PICC
 *             pi_rx_n      : the number of transmitted datas by PICC.
 * retval:
 *            0 - successfully
 *            others, error.
 */
int ISO14443_4_HalfDuplexExchange( struct pcd_info_s *ppcd_info, 
                                   unsigned char *psrc, 
                                   int i_tx_n, 
                                   unsigned char *pdes, 
                                  unsigned int *pi_rx_n )
{    
	
    int status;
    int            i_tx_num= 0; 
    int            i_tx_lmt= 0;
    int            i_tx_cn = 0;
    //unsigned char XDATA acu_tx_buf[256];
    unsigned char* acu_tx_buf = mf_com_data.mf_data;
    //unsigned char XDATA  acu_rx_buf[256];
    unsigned char* acu_rx_buf = mf_com_data.mf_data;
    unsigned char* puc_tx_buf  = acu_tx_buf;
    unsigned char* puc_rx_buf  = acu_rx_buf;
    
    int            i_s_retry   = 0;
    int            i_i_retry   = 0; 
    int            i_err_retry = 0;
    
    unsigned int   ui_wtx     = 1;
    
    int s_swt_limit_count;
	                          
    
    enum exe_step_t  e_ex_step; /*excuting stage*/

	transceive_buffer XDATA *pi;
	pi = &mf_com_data;


    e_ex_step   = ORG_IBLOCK;
    *pi_rx_n    = 0;
    ui_wtx     = 1;
    i_s_retry   = 0;
    i_i_retry   = 0;
    i_err_retry = 0;
    
    if( ppcd_info->uiFsc < 16 )ppcd_info->uiFsc = 32;
    i_tx_lmt = ppcd_info->uiFsc - 3;
	
    if( ppcd_info->ucNadEn )i_tx_lmt--;
    if( ppcd_info->ucCidEn )i_tx_lmt--;
    
    i_tx_num = i_tx_n;
    s_swt_limit_count=0;
    do
    {
        switch( e_ex_step )
        {
        case ORG_IBLOCK:
            __printf( "ORG_IBLOCK\r\n" );
            puc_tx_buf  = acu_tx_buf;
            
            if( i_tx_num > i_tx_lmt )
            {
                acu_tx_buf[0] = 0x12 | ( ppcd_info->ucPcdPcb & ISO14443_CL_PROTOCOL_ISN );
                if( ppcd_info->ucCidEn )acu_tx_buf[0] |= ISO14443_CL_PROTOCOL_CID;
                if( ppcd_info->ucNadEn )acu_tx_buf[0] |= ISO14443_CL_PROTOCOL_NAD;
                puc_tx_buf++;
                
                if( ppcd_info->ucCidEn )*puc_tx_buf++ = ppcd_info->ucCid & 0x0F;  
                if( ppcd_info->ucNadEn )*puc_tx_buf++ = ppcd_info->ucNad & 0x0F;
                
                i_tx_cn    = i_tx_lmt;
                i_tx_num  -= i_tx_lmt;
            }
            else 
            {
                acu_tx_buf[0] = 0x02 | ( ppcd_info->ucPcdPcb & ISO14443_CL_PROTOCOL_ISN );
                if( ppcd_info->ucCidEn )acu_tx_buf[0] |= ISO14443_CL_PROTOCOL_CID;
                if( ppcd_info->ucNadEn )acu_tx_buf[0] |= ISO14443_CL_PROTOCOL_NAD;
                puc_tx_buf++;
                
                if( ppcd_info->ucCidEn )*puc_tx_buf++ = ( ppcd_info->ucCid & 0x0F );  
                if( ppcd_info->ucNadEn )*puc_tx_buf++ = ( ppcd_info->ucNad & 0x0F );
                
                i_tx_cn   = i_tx_num;
                i_tx_num  = 0;
            }
            memcpy( puc_tx_buf, psrc, i_tx_cn );
            puc_tx_buf += i_tx_cn;
            psrc   += i_tx_cn;
         
            ppcd_info->ucPcdPcb = acu_tx_buf[0];
            
            e_ex_step = ORG_TRARCV;
        break;
        case ORG_ACKBLOCK:
            __printf( "ORG_ACKBLOCK\r\n" );
            puc_tx_buf  = acu_tx_buf;
            acu_tx_buf[0] = 0xA2 | ( ppcd_info->ucPcdPcb & 1 );
            if( ppcd_info->ucCidEn )acu_tx_buf[0] |= ISO14443_CL_PROTOCOL_CID;
            puc_tx_buf++;
            if( ppcd_info->ucCidEn )*puc_tx_buf++ = ( ppcd_info->ucCid & 0x0F );  
            e_ex_step = ORG_TRARCV;
        break;
        case ORG_NACKBLOCK:
            __printf( "ORG_NACKBLOCK\r\n" );
            puc_tx_buf  = acu_tx_buf;
            acu_tx_buf[0] = 0xB2 | ( ppcd_info->ucPcdPcb & 1 );
            if( ppcd_info->ucCidEn )acu_tx_buf[0] |= ISO14443_CL_PROTOCOL_CID;
            puc_tx_buf++;
            if( ppcd_info->ucCidEn )*puc_tx_buf++ = ( ppcd_info->ucCid & 0x0F );  
            e_ex_step = ORG_TRARCV;
        break;
        case ORG_SBLOCK:
            __printf( "ORG_SBLOCK\r\n" );
            puc_tx_buf  = acu_tx_buf;
            *puc_tx_buf++ = 0xF2;
            *puc_tx_buf++ = ui_wtx & 0x3F;  
            e_ex_step = ORG_TRARCV;    
        break;
        case ORG_TRARCV:
            __printf( "ORG_TRARCV\r\n" );
           
            i_err_retry++;
            

			if( ui_wtx > 59 )
			{
				ui_wtx = 1;
				e_ex_step = NON_EVENT;
				status = ISO14443_4_ERR_PROTOCOL;
				break;
			}
			
            //ppcd_info->uiFwi = ui_wtx * ppcd_info->uiFwi;
			g_pcd_module_info.ucWTXM = ui_wtx;


			pcd_delay_sfgi(ppcd_info->uiSfgi);
			pcd_set_tmo(ppcd_info->uiFwi);
			pi->mf_length = puc_tx_buf - acu_tx_buf;
			memcpy(pi->mf_data, acu_tx_buf, pi->mf_length);
			pi->mf_command = PCD_TRANSCEIVE;
			status = pcd_com_transceive(pi);
			ppcd_info->uiPcdTxRNum = pi->mf_length / 8 + !!(pi->mf_length % 8);
			memcpy(acu_rx_buf, pi->mf_data, ppcd_info->uiPcdTxRNum);
			ppcd_info->uiPcdTxRLastBits = pi->mf_length % 8;
			
           
            if (status != MI_OK)
            {

                if( i_err_retry > ISO14443_PROTOCOL_RETRANSMISSION_LIMITED ||
                    i_s_retry > ISO14443_PROTOCOL_RETRANSMISSION_LIMITED )
                {
                    e_ex_step = RCV_INVBLOCK;
                }
                else
                {
                    if( ppcd_info->ucPiccPcb & ISO14443_CL_PROTOCOL_CHAINED )
                    {
                        e_ex_step = ORG_ACKBLOCK;
                    }
                    else
                    {
                        e_ex_step = ORG_NACKBLOCK;  
                    }
                }
            }
            else
            {
                i_err_retry = 0;
                
                if (status == MI_OK)
                {   
                    puc_rx_buf = acu_rx_buf;
                    
                    if( 0x02 == ( acu_rx_buf[0] & 0xE2 ) )
                    {
                        if( ( 0 == ( acu_rx_buf[0] & 0x2 ) ) || ( ppcd_info->uiPcdTxRNum > 254 ) ||
                            ( ( ISO14443_CL_PROTOCOL_CID | ISO14443_CL_PROTOCOL_NAD ) & acu_rx_buf[0] )
                          )
                        {
                           // __printf( "invblock1\r\n" );
                            e_ex_step = RCV_INVBLOCK;
                        }
                        else
                        {
                            i_s_retry = 0;
                            e_ex_step  = RCV_IBLOCK;
                        }
                    }
                    else if( 0xA0 == ( acu_rx_buf[0] & 0xE0 ) )
                    {
                        
                        if( ( ( ISO14443_CL_PROTOCOL_CID | ISO14443_CL_PROTOCOL_NAD ) & acu_rx_buf[0] ) ||
                            ( ppcd_info->uiPcdTxRNum > 2 )
                          )
                        {    
                          //  __printf( "invblock2\r\n" );
                            e_ex_step = RCV_INVBLOCK;
                        }
                        else
                        {
                            i_s_retry = 0;
                            e_ex_step  = RCV_RBLOCK;
                        }
                    }
                    else if( 0xC0 == ( acu_rx_buf[0] & 0xC0 ) )
                    {   
                        if( ( ( ISO14443_CL_PROTOCOL_CID | ISO14443_CL_PROTOCOL_NAD ) & acu_rx_buf[0] )||
                            ( ppcd_info->uiPcdTxRNum > 2 )
                          )
                        {    
                         //   __printf( "invblock3\r\n" );
                            e_ex_step = RCV_INVBLOCK;
                        }
                        else
                        {
                            i_s_retry++;
                            e_ex_step = RCV_SBLOCK;
                        }
                    }
                    else 
                    {
                    	//__printf( "invblock4\r\n" );
                        e_ex_step = RCV_INVBLOCK;
                    }
                }
                else
                {
                    
                }
            }
            
 	
            ui_wtx       = 1;
			g_pcd_module_info.ucWTXM = ui_wtx;
        break;
        case RCV_IBLOCK:
            __printf( "RCV_IBLOCK\r\n" );
            if( i_tx_num )
            {
            	//__printf( "invblock5\r\n" );
                e_ex_step = RCV_INVBLOCK;
            }
            else
            {
                puc_rx_buf++;
                
                
                if( ( ppcd_info->ucPcdPcb & ISO14443_CL_PROTOCOL_ISN ) == 
                    ( acu_rx_buf[0] & ISO14443_CL_PROTOCOL_ISN ) )
                {   
                    ppcd_info->ucPiccPcb = acu_rx_buf[0];
                    
                    if( acu_rx_buf[0] & ISO14443_CL_PROTOCOL_CHAINED )
                    {
                        if( acu_rx_buf[0] & ISO14443_CL_PROTOCOL_CID )puc_rx_buf++;
                        if( acu_rx_buf[0] & ISO14443_CL_PROTOCOL_NAD )puc_rx_buf++;
                         
                        
                        e_ex_step = ORG_ACKBLOCK;
                    }
                    else
                    {
                        if( acu_rx_buf[0] & ISO14443_CL_PROTOCOL_CID )puc_rx_buf++;
                        if( acu_rx_buf[0] & ISO14443_CL_PROTOCOL_NAD )puc_rx_buf++;
                        
                        e_ex_step = NON_EVENT;   
                    }
                    
                    
                    if( ppcd_info->uiPcdTxRNum >= ( puc_rx_buf - acu_rx_buf ) )
                    {
                        memcpy( pdes, puc_rx_buf, ( ppcd_info->uiPcdTxRNum - ( puc_rx_buf - acu_rx_buf ) ) );
                        pdes  += ppcd_info->uiPcdTxRNum - ( puc_rx_buf - acu_rx_buf );
                        *pi_rx_n  += ppcd_info->uiPcdTxRNum - ( puc_rx_buf - acu_rx_buf );
                    }
                    
                    ppcd_info->ucPcdPcb ^= ISO14443_CL_PROTOCOL_ISN;
                }
                else 
                {
                    //__printf( "invblock6\r\n" );
                    e_ex_step = RCV_INVBLOCK;   
                }
            }
        break;
        case RCV_RBLOCK:
            __printf( "RCV_RBLOCK\r\n" );
            if( acu_rx_buf[0] & 0x10 )
            {
            	//__printf( "invblock7\r\n" );
                e_ex_step = RCV_INVBLOCK;
            }
            else
            {
                
                if( ( ppcd_info->ucPcdPcb & ISO14443_CL_PROTOCOL_ISN ) == 
                    ( acu_rx_buf[0] & ISO14443_CL_PROTOCOL_ISN ) )
                {
                    
                    if( ppcd_info->ucPcdPcb & ISO14443_CL_PROTOCOL_CHAINED )
                    {   
                        ppcd_info->ucPcdPcb ^= ISO14443_CL_PROTOCOL_ISN;
                        
                        i_i_retry = 0;
                        e_ex_step = ORG_IBLOCK;
                    }
                    else
                    {
                    	//__printf( "invblock8\r\n" );
                        e_ex_step = RCV_INVBLOCK; 
                    }
                }
                else 
                {
                    i_i_retry++;
                    
                    if( i_i_retry > ISO14443_PROTOCOL_RETRANSMISSION_LIMITED )
                    {
                    	//__printf( "invblock9\r\n" );
                        e_ex_step = RCV_INVBLOCK;     
                    }
                    else
                    {
                        i_tx_num += i_tx_cn;
                        psrc -= i_tx_cn;
                        e_ex_step = ORG_IBLOCK;
                    }
                }
            }
        break;
        case RCV_SBLOCK:
            __printf( "RCV_SBLOCK\r\n" );
            if( 0xF2 != ( acu_rx_buf[0] & 0xF7 ) )
            {
            	//__printf( "invblock10\r\n" );
                e_ex_step = RCV_INVBLOCK;
            }
            else
            {
                puc_rx_buf = acu_rx_buf + 1;
                if( acu_rx_buf[0] & ISO14443_CL_PROTOCOL_CID )puc_rx_buf++;
                if( 0 == ( *puc_rx_buf & 0x3F ) )
                {
                    //__printf( "invblock11\r\n" );
                    e_ex_step = RCV_INVBLOCK;
                }
                else
                {
                    s_swt_limit_count++;
                    ui_wtx = ( *puc_rx_buf & 0x3F );
                    e_ex_step = ORG_SBLOCK;
                }
            }
        break;
        case RCV_INVBLOCK:
            __printf( "RCV_INVBLOCK\r\n" );

			if (MI_NOTAGERR != status)
			{
				status = ISO14443_4_ERR_PROTOCOL;
			}
			e_ex_step = NON_EVENT;
            
        break;
        default:
        break;
        }
    }while( NON_EVENT != e_ex_step );

    return status;
}

//////////////////////////////////////////////////////////////////////
//ISO14443 DESELECT
//////////////////////////////////////////////////////////////////////
int iso14443_4_deselect(u8 CID)
{
    int status;
    
	transceive_buffer XDATA *pi;
    pi = &mf_com_data;

#if (NFC_DEBUG)
	printf("DESELECT:\n");
#endif
	pcd_set_tmo(4);
    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = 0xca;
    mf_com_data.mf_data[1] = CID & 0x0f;
    status = pcd_com_transceive(pi);
    return status;
}



